#include "AlpideDecoder.h"
#include "AlpideDictionary.h"
#include "BoardDecoder.h"
#include "TAlpide.h"
#include "TChipConfig.h"
#include "TDevice.h"
#include "TDeviceDigitalScan.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TScanConfig.h"
#include <stdexcept>
#include <iostream>
#include <bitset>

using namespace std;

//___________________________________________________________________
TDeviceDigitalScan::TDeviceDigitalScan() :
TDeviceChipVisitor(),
fScanConfig( nullptr )
{
    ClearHitData();
}

//___________________________________________________________________
TDeviceDigitalScan::TDeviceDigitalScan( shared_ptr<TDevice> aDevice,
                                       shared_ptr<TScanConfig> aScanConfig ) :
TDeviceChipVisitor( aDevice ),
fScanConfig( nullptr )
{
    ClearHitData();
    try {
        SetScanConfig( aScanConfig );
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
}

//___________________________________________________________________
TDeviceDigitalScan::~TDeviceDigitalScan()
{
    if ( fScanConfig ) fScanConfig.reset();
}

//___________________________________________________________________
void TDeviceDigitalScan::SetScanConfig( shared_ptr<TScanConfig> aScanConfig )
{
    if ( !aScanConfig ) {
        throw runtime_error( "TDeviceDigitalScan::SetScanConfig() - can not use a null pointer !" );
    }
    fScanConfig = aScanConfig;
}

//___________________________________________________________________
void TDeviceDigitalScan::WriteDataToFile( const char *fName, bool Recreate )
{
    char  fNameChip[100];
    FILE *fp;
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    
    for ( int ichip = 0; ichip < fDevice->GetNChips(); ichip ++ ) {
        cout << "TDeviceDigitalScan::WriteDataToFile() - ichip = "<< ichip << endl;
        int chipId = fDevice->GetChipId(ichip) & 0xf;
        int ctrInt = fDevice->GetChipConfig(ichip)->GetControlInterface();
        
        if ( !HasData(chipId) ) continue;  // write files only for chips with data
        if ( fDevice->GetNChips() > 1 ) {
            sprintf( fNameChip, "%s_Chip%d_%d.dat", fNameTemp, chipId, ctrInt );
        } else {
            sprintf( fNameChip, "%s.dat", fNameTemp );
        }
        cout << "TDeviceDigitalScan::WriteDataToFile() - Writing data to file "<< fNameChip << endl;
        
        if ( Recreate ) fp = fopen(fNameChip, "w");
        else            fp = fopen(fNameChip, "a");
        for ( int icol = 0; icol < TDeviceDigitalScan::NPRIORITY_ENCODERS; icol ++) {
            for (int iaddr = 0; iaddr < TDeviceDigitalScan::NADDRESSES; iaddr ++) {
                if ( fHitData[chipId][icol][iaddr] > 0 ) {
                    fprintf( fp, "%d %d %d\n",
                            icol,
                            iaddr,
                            fHitData[chipId][icol][iaddr] );
                }
            }
        }
        if (fp) fclose (fp);
    }
}

//___________________________________________________________________
void TDeviceDigitalScan::Go()
{
    const int myNTriggers = fScanConfig->GetNInj();
    const int myMaskStages = fScanConfig->GetNMaskStages();
    const int myPixPerRegion = fScanConfig->GetPixPerRegion();

    unsigned char buffer[1024*4000];
    int n_bytes_data, n_bytes_header, n_bytes_trailer, errors8b10b = 0, nClosedEvents = 0;
    int nBad       = 0;
    int nSkipped   = 0;
    TBoardHeader boardInfo;
    
    shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(fDevice->GetBoard( 0 ));

    if ( myMOSAIC ) {
        myMOSAIC->StartRun();
    }
    
    for ( int istage = 0; istage < myMaskStages; istage ++ ) {
        
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::Go() - Mask stage " << istage << endl;
        }
        DoConfigureMaskStage( myPixPerRegion, istage );
        
        //uint16_t Value;
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_CMUDMU_CONFIG, Value );
        //cout << "CMU DMU Config: 0x" << std::hex << Value << std::dec << endl;
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_FROMU_STATUS1, Value );
        //cout << "Trigger counter before: " << Value << endl;
        (fDevice->GetBoard( 0 ))->Trigger(myNTriggers);
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_FROMU_STATUS1, Value );
        //cout << "Trigger counter after: " << Value << endl;
        
        //(fDevice->GetBoard( 0 ))->SendOpCode( (uint16_t)Alpide::OPCODE_DEBUG );
        //(fDevice->GetChip(0))->PrintDebugStream();
        
        int itrg = 0;
        int nTrials = 0;
        
        while( itrg < myNTriggers * fDevice->GetNWorkingChips() ) {

            if ( (fDevice->GetBoard( 0 ))->ReadEventData(n_bytes_data, buffer) == -1) {

                // no event available in buffer yet, wait a bit
                usleep(100);
                nTrials ++;
                if ( nTrials == TDeviceDigitalScan::MAXTRIALS ) {
                    if ( GetVerboseLevel() > kSILENT ) {
                        cout << "TDeviceDigitalScan::Go() - Reached " << nTrials << " timeouts, giving up on this point." << endl;
                    }
                    itrg = myNTriggers * fDevice->GetNWorkingChips();
                    nSkipped ++;
                    nTrials = 0;
                }
                continue;
                
            } else {
                //cout << "TDeviceDigitalScan::Go() - received Event" << itrg << " with length " << n_bytes_data << endl;
                //for (int iByte=0; iByte<n_bytes_data; ++iByte) {
                //  printf ("%02x ", (int) buffer[iByte]);
                //}
                //cout << endl;

                // decode readout board event
                shared_ptr<TBoardConfig> boardConfig = (fDevice->GetBoard( 0 ))->GetConfig().lock();
                TBoardType boardType = boardConfig->GetBoardType();
                BoardDecoder::DecodeEvent( boardType, buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo );
                //cout << "Closed data counter: " <<  boardInfo.eoeCount << endl;
                if ( boardInfo.eoeCount) {
                    nClosedEvents = boardInfo.eoeCount;
                } else {
                    nClosedEvents = 1;
                }
                if ( boardInfo.decoder10b8bError ) errors8b10b++;

                // decode Chip event
                int n_bytes_chipevent = n_bytes_data-n_bytes_header - n_bytes_trailer;
                if ( !AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, fHits) ) {
                    if ( GetVerboseLevel() > kSILENT ) {
                        cout << "TDeviceDigitalScan::Go() - Found bad event " << endl;
                    }
                    nBad ++;
                    if ( nBad > TDeviceDigitalScan::MAXNBAD ) continue;
                    FILE *fDebug = fopen ("DebugData.dat", "a");
                    for ( int iByte=0; iByte<n_bytes_data; ++iByte ) {
                        fprintf (fDebug, "%02x ", (int) buffer[iByte]);
                    }
                    fclose( fDebug );
                }
                //cout << "TDeviceDigitalScan::Go() - total number of hits found: " << fHits.size() << std::endl;
                itrg+= nClosedEvents;
            }
        }
        
        //cout << "TDeviceDigitalScan::Go() - Hit pixels: " << endl;
        //for ( int i=0; i < fHits.size(); i++ ) {
        //  cout << i << ":\t region: " << fHits.at(i)->GetRegion() << "\tdcol: " << fHits.at(i)->GetDoubleColumn() << "\taddres: " << fHits.at(i)->GetAddress() << endl;
        //}
        CopyHitData();
    }
    
    cout << endl;
    if ( myMOSAIC ) {
        myMOSAIC->StopRun();
        cout << "TDeviceDigitalScan::Go() - Total number of 8b10b decoder errors: " << errors8b10b << endl;
    }
    cout << "TDeviceDigitalScan::Go() - Number of corrupt events:             " << nBad       << endl;
    cout << "TDeviceDigitalScan::Go() - Number of skipped points:             " << nSkipped   << endl;
}

//___________________________________________________________________
void TDeviceDigitalScan::ClearHitData()
{
    for ( int ichip = 0; ichip < TDeviceDigitalScan::MAX_NCHIPS; ichip ++ ) {
        for ( int icol = 0; icol < TDeviceDigitalScan::NPRIORITY_ENCODERS; icol ++ ) {
            for ( int iaddr = 0; iaddr < TDeviceDigitalScan::NADDRESSES; iaddr ++ ) {
                fHitData[ichip][icol][iaddr] = 0;
            }
        }
    }
}

//___________________________________________________________________
void TDeviceDigitalScan::CopyHitData()
{
    for ( unsigned int ihit = 0; ihit < fHits.size(); ihit ++ ) {
        int chipId  = (fHits.at(ihit))->GetChipId();
        int dcol    = (fHits.at(ihit))->GetDoubleColumn();
        int region  = (fHits.at(ihit))->GetRegion();
        int address = (fHits.at(ihit))->GetAddress();
        if ((chipId < 0) || (dcol < 0) || (region < 0) || (address < 0)) {
            cout << "TDeviceDigitalScan::CopyHitData() - Bad pixel coordinates ( <0), skipping hit" << endl;
        } else {
            fHitData[chipId][dcol + region * 16][address] ++;
        }
    }
    fHits.clear();
}

//___________________________________________________________________
bool TDeviceDigitalScan::HasData( const int chipId )
{
    for ( int icol = 0; icol < TDeviceDigitalScan::NPRIORITY_ENCODERS; icol ++ ) {
        for (int iaddr = 0; iaddr < TDeviceDigitalScan::NADDRESSES; iaddr ++) {
            if (fHitData[chipId][icol][iaddr] > 0) return true;
        }
    }
    return false;
}


