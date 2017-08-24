#include "AlpideDecoder.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TAlpide.h"
#include "TChipConfig.h"
#include "TDevice.h"
#include "TDeviceDigitalScan.h"
#include "TPixHit.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TScanConfig.h"
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <string.h>


using namespace std;

//___________________________________________________________________
TDeviceDigitalScan::TDeviceDigitalScan() :
TDeviceChipVisitor(),
fHitData( nullptr ),
fScanConfig( nullptr )
{
    
}

//___________________________________________________________________
TDeviceDigitalScan::TDeviceDigitalScan( shared_ptr<TDevice> aDevice,
                                       shared_ptr<TScanConfig> aScanConfig ) :
TDeviceChipVisitor( aDevice ),
fHitData( nullptr ),
fScanConfig( nullptr )
{
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
    if ( fHitData ) {
        delete[] fHitData;
    }
    fHits.clear();
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
void TDeviceDigitalScan::Init()
{
    if ( !fScanConfig ) {
        throw runtime_error( "TDeviceDigitalScan::Init() - can not use a null pointer for the scan config !" );
    }
    try {
        TDeviceChipVisitor::Init();
    } catch ( std::exception &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    
    // allocate memory for array of hit pixels
    
    fHitData = new int[ fDevice->GetNChips() * NPRIORITY_ENCODERS * NADDRESSES ];
    
    // fill with zeroes
    
    ClearHitData();
}

//___________________________________________________________________
void TDeviceDigitalScan::WriteDataToFile( const char *fName, bool Recreate )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceDigitalScan::WriteDataToFile() - not initialized ! Please use Init() first." );
    }

    char  fNameChip[100];
    FILE *fp;
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    
    for ( unsigned int ichip = 0; ichip < fDevice->GetNChips(); ichip ++ ) {
        
        if ( !((fDevice->GetChipConfig(ichip))->IsEnabled()) ) {
            if ( GetVerboseLevel() > kTERSE ) {
                cout << "TDeviceDigitalScan::WriteDataToFile() - Chip ID "
                << fDevice->GetChipId(ichip) << " : disabled chip, skipped." <<  endl;
            }
            continue;
        }
        int chipId = fDevice->GetChipId(ichip) & 0xf;
        int ctrInt = fDevice->GetChipConfig(ichip)->GetControlInterface();
        if ( !HasData(ichip) ) {
            if ( GetVerboseLevel() > kTERSE ) {
                cout << "TDeviceDigitalScan::WriteDataToFile() - Chip ID "
                << fDevice->GetChipId(ichip) << " : no data, skipped." <<  endl;
            }
            continue;  // write files only for chips with data
        }
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::WriteDataToFile() - Chip ID = "<< fDevice->GetChipId(ichip) << endl;
        }
        if ( fDevice->GetNChips() > 1 ) {
            sprintf( fNameChip, "%s_chip%d_%d.dat", fNameTemp, chipId, ctrInt );
        } else {
            sprintf( fNameChip, "%s.dat", fNameTemp );
        }
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::WriteDataToFile() - Writing data to file "<< fNameChip << endl;
        }
        if ( Recreate ) fp = fopen(fNameChip, "w");
        else            fp = fopen(fNameChip, "a");
        for ( unsigned int icol = 0; icol < NPRIORITY_ENCODERS; icol ++ ) {
            for ( unsigned int iaddr = 0; iaddr < NADDRESSES; iaddr ++ ) {
                unsigned int index = GetHitDataIndex( ichip, icol, iaddr );
                if ( fHitData[index] > 0 ) {
                    fprintf( fp, "%d %d %d\n",
                            icol,
                            iaddr,
                            fHitData[index] );
                }
            }
        }
        if (fp) fclose (fp);
    }
}

//___________________________________________________________________
void TDeviceDigitalScan::Go()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceDigitalScan::Go() - not initialized ! Please use Init() first." );
    }

    const int myNTriggers = fScanConfig->GetNInj();
    const int myMaskStages = fScanConfig->GetNMaskStages();
    const int myPixPerRegion = fScanConfig->GetPixPerRegion();

    unsigned char buffer[1024*4000];
    int n_bytes_data, n_bytes_header, n_bytes_trailer, errors8b10b = 0, nClosedEvents = 0;
    unsigned int nBad       = 0;
    unsigned int nSkipped   = 0;
    
    shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(fDevice->GetBoard( 0 ));

    shared_ptr<TReadoutBoardDAQ> myDAQBoard = dynamic_pointer_cast<TReadoutBoardDAQ>(fDevice->GetBoard( 0 ));

    if ( myMOSAIC ) {
        myMOSAIC->StartRun();
    }
    
    TBoardDecoder boardDecoder;

    for ( int istage = 0; istage < myMaskStages; istage ++ ) {
        
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::Go() - Mask stage " << istage << endl;
        }
        DoConfigureMaskStage( myPixPerRegion, istage );
        
        //uint16_t Value;
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_CMU_DMU_CONFIG, Value );
        //cout << "CMU DMU Config: 0x" << std::hex << Value << std::dec << endl;
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_FROMU_STATUS1, Value );
        //cout << "Trigger counter before: " << Value << endl;
        (fDevice->GetBoard( 0 ))->Trigger(myNTriggers);
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_FROMU_STATUS1, Value );
        //cout << "Trigger counter after: " << Value << endl;
        
        //(fDevice->GetBoard( 0 ))->SendOpCode( (uint16_t)Alpide::OPCODE_DEBUG );
        //(fDevice->GetChip(0))->PrintDebugStream();
        
        unsigned int itrg = 0;
        unsigned int nTrials = 0;
        
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
                if ( GetVerboseLevel() > kVERBOSE ) {
                    cout << "TDeviceDigitalScan::Go() - received Event " << itrg << " with length " << n_bytes_data << endl;
                    for ( int iByte = 0; iByte < n_bytes_data; ++iByte ) {
                        printf ("%02x ", (int) buffer[iByte]);
                    }
                    cout << endl;
                }


                // decode readout board event
                shared_ptr<TBoardConfig> boardConfig = fDevice->GetBoardConfig( 0 );
                boardDecoder.SetBoardType( boardConfig->GetBoardType() );
                boardDecoder.DecodeEvent( buffer, n_bytes_data, n_bytes_header, n_bytes_trailer );
                //cout << "Closed data counter: " <<  boardDecoder.GetMosaicEoeCount() << endl;
                if ( boardDecoder.GetMosaicEoeCount() ) {
                    nClosedEvents = boardDecoder.GetMosaicEoeCount();
                } else {
                    nClosedEvents = 1;
                }
                if ( boardDecoder.GetMosaicDecoder10b8bError() ) errors8b10b++;

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
                if ( GetVerboseLevel() > kVERBOSE ) {
                    cout << "TDeviceDigitalScan::Go() - total number of hits found: "
                         << fHits.size() << endl;
                }
                itrg+= nClosedEvents;
            }
        }
        MoveHitData();
    }
    
    cout << endl;
    if ( myMOSAIC ) {
        myMOSAIC->StopRun();
        cout << "TDeviceDigitalScan::Go() - Total number of 8b10b decoder errors: " << errors8b10b << endl;
    }
    if ( myDAQBoard ) {
        myDAQBoard->PowerOff();
    }
    cout << "TDeviceDigitalScan::Go() - Number of corrupt events:             " << nBad       << endl;
    cout << "TDeviceDigitalScan::Go() - Number of skipped points:             " << nSkipped   << endl;

    
}

//___________________________________________________________________
void TDeviceDigitalScan::ClearHitData()
{
    if ( !fHitData ) {
        throw runtime_error( "TDeviceDigitalScan::ClearHitData() - no array of hit pixels defined !" );
    }
    for ( unsigned int ichip = 0; ichip < fDevice->GetNChips(); ichip ++ ) {
        for ( unsigned int icol = 0; icol < NPRIORITY_ENCODERS; icol ++ ) {
            for ( unsigned int iaddr = 0; iaddr < NADDRESSES; iaddr ++ ) {
                unsigned int index = GetHitDataIndex( ichip, icol, iaddr );
                fHitData[index] = 0;
            }
        }
    }
}

//___________________________________________________________________
void TDeviceDigitalScan::MoveHitData()
{
    if ( !fHitData ) {
        throw runtime_error( "TDeviceDigitalScan::MoveHitData() - no array of hit pixels defined !" );
    }

    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TDeviceDigitalScan::MoveHitData() - hit pixels : " << endl;
    }
    for ( unsigned int ihit = 0; ihit < fHits.size(); ihit ++ ) {
        int chipId  = (fHits.at(ihit))->GetChipId();
        int dcol    = (fHits.at(ihit))->GetDoubleColumn();
        int region  = (fHits.at(ihit))->GetRegion();
        int address = (fHits.at(ihit))->GetAddress();
        if ( GetVerboseLevel() > kVERBOSE ) {
            cout << "\t chip:dcol:region:address "
                << chipId << ":" << dcol << ":" << region << ":" << address << endl;
        }
        if ((chipId < 0) || (dcol < 0) || (region < 0) || (address < 0)) {
            cout << "TDeviceDigitalScan::MoveHitData() - Bad pixel coordinates ( <0), skipping hit" << endl;
        } else {
            int ichip = fDevice->GetChipIndexById( chipId );
            int index = GetHitDataIndex( ichip, dcol + region * 16, address );
            fHitData[index] ++;
        }
    }
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TDeviceDigitalScan::MoveHitData() --------------------- done" << endl;
    }
    fHits.clear();
}

//___________________________________________________________________
bool TDeviceDigitalScan::HasData( const unsigned int ichip )
{
    for ( unsigned int icol = 0; icol < NPRIORITY_ENCODERS; icol ++ ) {
        for (unsigned int iaddr = 0; iaddr < NADDRESSES; iaddr ++) {
            unsigned int index = GetHitDataIndex( ichip, icol, iaddr );
            if ( fHitData[index] > 0 ) return true;
        }
    }
    return false;
}

//___________________________________________________________________
int TDeviceDigitalScan::GetHitDataIndex( const unsigned int ichip,
                                         const unsigned int icol,
                                         const unsigned int iadd )
{
    if ( !fHitData ) {
        throw runtime_error( "TDeviceDigitalScan::GetHitDataIndex() - no array of hit pixels defined !" );
    }
    if ( ichip >= fDevice->GetNChips() ) {
        throw domain_error( "TDeviceDigitalScan::GetHitDataIndex() - Invalid chip index !" );
    }
    if ( icol >= NPRIORITY_ENCODERS ) {
        throw domain_error( "TDeviceDigitalScan::GetHitDataIndex() - Invalid double-column index !" );
    }
    if ( iadd >= NADDRESSES ) {
        throw domain_error( "TDeviceDigitalScan::GetHitDataIndex() - Invalid address index !" );
    }
    int index = ichip*NPRIORITY_ENCODERS*NADDRESSES + icol*NADDRESSES + iadd;
    return index;
}
