#include "TAlpideDecoder.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TChipConfig.h"
#include "Common.h"
#include "TDevice.h"
#include "TDeviceChipVisitor.h"
#include "TDeviceThresholdScan.h"
#include "TErrorCounter.h"
#include "THisto.h"
#include "TReadoutBoard.h"
#include "TScanConfig.h"
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <string.h>

using namespace std;

//___________________________________________________________________
TDeviceThresholdScan::TDeviceThresholdScan() :
TDeviceMaskScan(),
fChargeStart( 0 ),
fChargeStep( 0 ),
fChargeStop( 0 ),
fNChargeSteps( 0 )
{
    
}

//___________________________________________________________________
TDeviceThresholdScan::TDeviceThresholdScan( shared_ptr<TDevice> aDevice,
                                            shared_ptr<TScanConfig> aScanConfig ) :
TDeviceMaskScan( aDevice, aScanConfig ),
fChargeStart( 0 ),
fChargeStep( 0 ),
fChargeStop( 0 ),
fNChargeSteps( 0 )
{
    
}

//___________________________________________________________________
TDeviceThresholdScan::~TDeviceThresholdScan()
{
    fHistoQue.clear();
}

//___________________________________________________________________
void TDeviceThresholdScan::Init()
{
    try {
        TDeviceMaskScan::Init();
    } catch ( std::exception &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    fScanHisto = fHistoQue.front();
    if ( !fScanHisto ) {
        throw runtime_error( "TDeviceThresholdScan::Init() - can not use a null pointer for the map of scan histo !" );
    }
    fErrorCounter->Init( fScanHisto, fNTriggers );
}

//___________________________________________________________________
void TDeviceThresholdScan::Go()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceThresholdScan::Go() - not initialized ! Please use Init() first." );
    }
    
    if ( fNMaskStages < 0 ) {
        fNMaskStages = 1;
    }

    unsigned int nHitsTot = 0, nHitsLastStage = 0;
    
    for ( int istage = 0; istage < fNMaskStages; istage ++ ) { //---------------- loop on pixels
        
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceThresholdScan::Go() - Mask stage "
            << std::dec << istage << endl;
        }
        DoConfigureMaskStage( fNPixPerRegion, istage );
        int deltaV = fChargeStart;
        
        for ( unsigned int iampl = 0; iampl < fNChargeSteps; iampl++ ) { //-- loop on VPulse low
            
            if ( deltaV >= fChargeStop ) { break; }
            // use current charge
            DoConfigureVPulseLow( deltaV );
            // Send triggers for all boards
            for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
                (fDevice->GetBoard( ib ))->Trigger(fNTriggers);
            }
            if ( istage  && ( iampl == fNChargeSteps-1) ) {
                nHitsLastStage = fChipDecoder->GetNHits();
            }
            try {
                fScanHisto = fHistoQue.at( iampl );
            } catch ( std::exception &err ) {
                cerr << "TDeviceThresholdScan::Go() - Error, stage "
                     << std::dec << istage << " , iampl " << iampl
                     << " , " << err.what() << endl;
                exit( EXIT_FAILURE );
            }
            fChipDecoder->SetScanHisto( fScanHisto );
            // Read data for all boards
            for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
                ReadEventData( ib );
            }
            // next charge
            deltaV += fChargeStep;
            usleep(1000);
        } //---------------------------------------------------------- end of loop on VPulse low
    
        nHitsTot = fChipDecoder->GetNHits() - nHitsLastStage;
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceThresholdScan::Go() - stage "
            << std::dec << istage << " , found n hits = " << nHitsTot << endl;
        }
        usleep(200);
    } // end of loop on pixels
}

//___________________________________________________________________
void TDeviceThresholdScan::Terminate()
{
    TDeviceChipVisitor::Terminate();
    
}

//___________________________________________________________________
void TDeviceThresholdScan::WriteDataToFile( const char *fName, bool Recreate )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceThresholdScan::WriteDataToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceThresholdScan::WriteDataToFile() - not terminated ! Please use Terminate() first." );
    }
    if ( !fHistoQue.size() ) {
        throw runtime_error( "TDeviceThresholdScan::WriteDataToFile() - histo deque is empty !" );
    }
    
    char  fNameChip[100];
    FILE *fp;
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    string suffix( fNameTemp );
    
    const unsigned int chipListSize = (fHistoQue.front())->GetChipListSize();
    for ( unsigned int ichip = 0; ichip < chipListSize; ichip++ ) {
        
        fScanHisto = fHistoQue.front();
        common::TChipIndex aChipIndex = fScanHisto->GetChipIndex( ichip );
        
        if ( !HasData( aChipIndex ) ) {
            if ( GetVerboseLevel() > kSILENT ) {
                cout << "TDeviceThresholdScan::WriteDataToFile() - Chip ID = "
                << aChipIndex.chipId ;
                if ( aChipIndex.ladderId ) {
                    cout << " , Ladder ID = " << aChipIndex.ladderId;
                }
                cout << " : no data, skipped." <<  endl;
            }
            continue;  // write files only for chips with data
        }
        string filename = common::GetFileName( aChipIndex, suffix );
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceThresholdScan::WriteDataToFile() - Chip ID = "<< aChipIndex.chipId ;
            if ( aChipIndex.ladderId ) {
                cout << " , Ladder ID = " << aChipIndex.ladderId;
            }
            cout << endl;
        }
        strcpy( fNameChip, filename.c_str());
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceThresholdScan::WriteDataToFile() - Writing data to file "<< fNameChip << endl;
        }
        if ( Recreate ) fp = fopen(fNameChip, "w");
        else            fp = fopen(fNameChip, "a");
        if ( !fp ) {
            throw runtime_error( "TDeviceThresholdScan::WriteDataToFile() - output file not found." );
        }
        for ( unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++ ) {
            for ( unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++ ) {
                int icharge = fChargeStart;
                for ( unsigned int iampl = 0; iampl < fNChargeSteps; iampl ++ ) {
                    try {
                        fScanHisto = fHistoQue.at( iampl );
                    } catch ( std::exception &err ) {
                        cerr << "TDeviceThresholdScan::WriteDataToFile() - Error, icol "
                        << std::dec << icol << " , iaddr " << iaddr
                        << " , iampl " << iampl << err.what() << endl;
                        exit( EXIT_FAILURE );
                    }
                    double hits = (*fScanHisto)(aChipIndex,icol,iaddr);
                    if (hits > 0) {
                        fprintf(fp, "%d %d %d %d\n", icol, iaddr, icharge, (int)hits);
                    }
                    icharge += fChargeStep;
                }
            }
        }
        if (fp) fclose (fp);
    }
}

//___________________________________________________________________
void TDeviceThresholdScan::AddHisto()
{
    int currentCharge = fChargeStart;
    
    while ( currentCharge < fChargeStop ) {
        
        auto aScanHisto = make_shared<TScanHisto>();
        common::TChipIndex id;
        // histo for a given value of the injected charge
        THisto histo ("ThrScanHisto", "ThrScanHisto",
                      common::MAX_DCOL+1, 0, common::MAX_DCOL,
                      common::MAX_ADDR+1, 0, common::MAX_ADDR);
        
        for ( unsigned int ichip = 0; ichip < fDevice->GetNChips(); ichip++ ) {
            if ( fDevice->GetChipConfig(ichip)->IsEnabled() ) {
                id.boardIndex   = fDevice->GetBoardIndexByChip(ichip);
                id.dataReceiver = fDevice->GetChipConfig(ichip)->GetParamValue("RECEIVER");
                id.ladderId     = fDevice->GetLadderId();
                id.chipId       = fDevice->GetChipId(ichip);
                aScanHisto->AddHisto( id, histo );
            }
        }
        aScanHisto->FindChipList();
        if ( GetVerboseLevel() > kCHATTY ) {
            cout << endl << "TDeviceDigitalScan::AddHisto() - generated map with " << std::dec << aScanHisto->GetSize() << " elements" << endl;
        }
        fHistoQue.push_back( move(aScanHisto) );
        currentCharge += fChargeStep;
    }
    fNChargeSteps = fHistoQue.size();
    if ( GetVerboseLevel() > kSILENT ) {
        cout << endl << "TDeviceDigitalScan::AddHisto() - generated deque with " << std::dec << fNChargeSteps << " elements" << endl;
    }
}

//___________________________________________________________________
void TDeviceThresholdScan::DumpScanParameters()
{
    cout << "TDeviceThresholdScan::DumpScanParameters() - injected charge :" << endl;
    cout << "\t start   " << fChargeStart << endl;
    cout << "\t stop    " << fChargeStop << endl;
    cout << "\t step    " << fChargeStep << endl;
}

//___________________________________________________________________
void TDeviceThresholdScan::InitScanParameters()
{
    TDeviceMaskScan::InitScanParameters();
    fChargeStart = fScanConfig->GetChargeStart();
    fChargeStep  = fScanConfig->GetChargeStep();
    fChargeStop  = fScanConfig->GetChargeStop();

    // in case the input values were in a bad ordering
    if ( fChargeStart > fChargeStop ) {
        int buffer = fChargeStart;
        fChargeStart = fChargeStop;
        fChargeStop = buffer;
    }
    if ( fChargeStep < 0 ) {
        fChargeStep *= -1.;
    }
    if ( GetVerboseLevel() > kSILENT ) {
        DumpScanParameters();
    }
}

//___________________________________________________________________
bool TDeviceThresholdScan::HasData( const common::TChipIndex idx )
{
    for ( unsigned int iampl = 0; iampl < fNChargeSteps; iampl ++ ) {
        try {
            fScanHisto = fHistoQue.at( iampl );
        } catch ( std::exception &err ) {
            cerr << "TDeviceThresholdScan::HasData() - Error, iampl "
            << std::dec << iampl << err.what() << endl;
            exit( EXIT_FAILURE );
        }
        if ( !fScanHisto->IsValidChipIndex( idx ) ) {
            return false;
        }
        return fScanHisto->HasData( idx );
    }
    return false;
}
