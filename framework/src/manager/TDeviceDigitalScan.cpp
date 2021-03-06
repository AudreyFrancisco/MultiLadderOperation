#include "TAlpideDecoder.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TChipConfig.h"
#include "Common.h"
#include "TDevice.h"
#include "TDeviceChipVisitor.h"
#include "TDeviceDigitalScan.h"
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
TDeviceDigitalScan::TDeviceDigitalScan() :
TDeviceMaskScan()
{ }

//___________________________________________________________________
TDeviceDigitalScan::TDeviceDigitalScan( shared_ptr<TDevice> aDevice,
                                       shared_ptr<TScanConfig> aScanConfig ) :
TDeviceMaskScan( aDevice, aScanConfig )
{ }

//___________________________________________________________________
TDeviceDigitalScan::~TDeviceDigitalScan()
{ }

//___________________________________________________________________
void TDeviceDigitalScan::Terminate()
{
    TDeviceChipVisitor::Terminate();
    
    CollectDiscordantPixels();
    cout << endl;
    fErrorCounter->ClassifyCorruptedHits();
    fErrorCounter->Dump();
}

//___________________________________________________________________
void TDeviceDigitalScan::WriteDataToFile( bool Recreate )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceDigitalScan::WriteDataToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceDigitalScan::WriteDataToFile() - not terminated ! Please use Terminate() first." );
    }
    if ( !fScanHisto ) {
        throw runtime_error( "TDeviceDigitalScan::WriteDataToFile() - scan histo is a null pointer !" );
    }
    
    char  fNameChip[100];
    FILE *fp;
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName.c_str());
    strtok( fNameTemp, "." );
    string suffix( fNameTemp );
    
    for ( unsigned int ichip = 0; ichip < fDevice->GetNWorkingChips(); ichip++ ) {
        
        common::TChipIndex aChipIndex = fDevice->GetWorkingChipIndex( ichip );

        if ( !HasData( aChipIndex ) ) {
            if ( GetVerboseLevel() > kSILENT ) {
                cout << "TDeviceDigitalScan::WriteDataToFile() - ";
                common::DumpId( aChipIndex );
                cout << " : no data, skipped." <<  endl; 
            }
            continue;  // write files only for chips with data
        }
        string filename = common::GetFileName( aChipIndex, suffix );
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::WriteDataToFile() - ";
            common::DumpId( aChipIndex );
            cout << endl;
        }
        strcpy( fNameChip, filename.c_str());
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::WriteDataToFile() - Writing data to file "<< fNameChip << endl;
        }
        if ( Recreate ) fp = fopen(fNameChip, "w");
        else            fp = fopen(fNameChip, "a");
        if ( !fp ) {
            throw runtime_error( "TDeviceDigitalScan::WriteDataToFile() - output file not found." );
        }
        TPixHit pixhit;
        pixhit.SetPixChipIndex( aChipIndex );
        for ( unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++ ) {
            for ( unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++ ) {
                pixhit.SetDoubleColumn( icol );
                pixhit.SetAddress( iaddr );
                unsigned int column = pixhit.GetColumn();
                unsigned int row = pixhit.GetRow();
                double hits = (*fScanHisto)(aChipIndex,icol,iaddr);
                if (hits > 0) {
                    fprintf(fp, "%d %d %d\n", row, column, (int)hits);
                }
            }
        }
        if (fp) fclose (fp);
    }
}

//___________________________________________________________________
void TDeviceDigitalScan::WriteCorruptedHitsToFile( bool Recreate )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceDigitalScan::WriteCorruptedHitsToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceDigitalScan::WriteCorruptedHitsToFile() - not terminated ! Please use Terminate() first." );
    }
    fErrorCounter->WriteCorruptedHitsToFile( fName.c_str(), Recreate );
}

//___________________________________________________________________
void TDeviceDigitalScan::DrawAndSaveToFile()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceDigitalScan::DrawAndSaveToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceDigitalScan::DrawAndSaveToFile() - not terminated ! Please use Terminate() first." );
    }
    fErrorCounter->DrawAndSaveToFile( fName.c_str() );
}

//___________________________________________________________________
void TDeviceDigitalScan::Go()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceDigitalScan::Go() - not initialized ! Please use Init() first." );
    }

    int NStages = fNMaskStages;
    if ( fNMaskStages < 0 ) {
        NStages = 1;
    }
    
    unsigned int nHitsTot = 0, nHitsLastStage = 0;

    for ( int istage = 0; istage < NStages; istage ++ ) {
        
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::Go() - Mask stage "
                 << std::dec << istage << endl;
        }
        if ( fNMaskStages < 0 ) {
            DoConfigureMaskStage( fNPixPerRegion, fNMaskStages );
        } else {
            DoConfigureMaskStage( fNPixPerRegion, istage );
        }
        
        
        // uint16_t Value;
        // (fDevice->GetChip(0))->ReadRegister( AlpideRegister::CMU_DMU_CONFIG, Value );
        // cout << "CMU DMU Config: 0x" << std::hex << Value << std::dec << endl;
        // (fDevice->GetChip(0))->ReadRegister( AlpideRegister::CMU_DMU_STATUS, Value );
        // cout << "Trigger counter before: " << Value << endl;
        for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
            // Send triggers for all boards
            (fDevice->GetBoard( ib ))->Trigger(fNTriggers);
        }
        // (fDevice->GetChip(0))->ReadRegister( AlpideRegister::CMU_DMU_STATUS, Value );
        // cout << "Trigger counter after: " << Value << endl;
        // (fDevice->GetBoard( 0 ))->SendOpCode( (uint16_t)AlpideOpCode::DEBUG );
        // (fDevice->GetChip(0))->PrintDebugStream();
        
        if ( istage ) {
            nHitsLastStage = fChipDecoder->GetNHits();
        }
        usleep(2000);

        // Read data for all boards
        for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
            ReadEventData( ib );
        }
        
        nHitsTot = fChipDecoder->GetNHits() - nHitsLastStage;
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::Go() - stage "
                 << std::dec << istage << " , found n hits = " << nHitsTot << endl;
        }
        usleep(200);
    }
}

//___________________________________________________________________
void TDeviceDigitalScan::AddHisto()
{
    common::TChipIndex id;
    
    THisto histo ("DigScanHisto", "DigScanHisto",
                  common::MAX_DCOL+1, 0, common::MAX_DCOL,
                  common::MAX_ADDR+1, 0, common::MAX_ADDR);
    
    for ( unsigned int ichip = 0; ichip < fDevice->GetNChips(); ichip++ ) {
        if ( fDevice->GetChipConfig(ichip)->IsEnabled() ) {
            id.boardIndex   = fDevice->GetBoardIndexByChip(ichip);
            id.dataReceiver = fDevice->GetChipConfig(ichip)->GetParamValue("RECEIVER");
            id.deviceType   = fDevice->GetDeviceType();
            id.deviceId     = fDevice->GetDeviceId();
            id.chipId       = fDevice->GetChipId(ichip);
            fScanHisto->AddHisto( id, histo );
        }
    }
    fScanHisto->FindChipList();
    if ( GetVerboseLevel() > kSILENT ) {
        cout << endl << "TDeviceDigitalScan::AddHisto() - generated map with " << std::dec << fScanHisto->GetSize() << " elements" << endl;
    }
    return;
}

//___________________________________________________________________
void TDeviceDigitalScan::CollectDiscordantPixels()
{
    bool isFullMatrix = (( fNMaskStages == 512 ) && ( fNPixPerRegion == 32 ));
    
    if ( !isFullMatrix  ) {
        cout << "TDeviceDigitalScan::CollectDiscordantPixels() - not implemented when only part of the pixel matrix is tested. Please test the full matrix." << endl;
        return;
    }
    for ( unsigned int ichip = 0; ichip < fScanHisto->GetChipListSize(); ichip++ ) {
        
        for (unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++) {
            for (unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++) {
                
                common::TChipIndex idx = fScanHisto->GetChipIndex(ichip);
                double nhits = (*fScanHisto)(idx,icol,iaddr);
                if ( nhits == 0 ) {
                    fErrorCounter->AddDeadPixel( idx, icol, iaddr );
                } else {
                    if ( nhits < fNTriggers ) {
                        fErrorCounter->AddInefficientPixel( idx, icol, iaddr, nhits );
                    } else {
                        if ( nhits > fNTriggers ) {
                            fErrorCounter->AddHotPixel( idx, icol, iaddr, nhits );
                        }
                    }
                }
                
            } // end of loop on iaddr
        } // end of loop on icol
        
    } // end of loop on ichip
}

//___________________________________________________________________
bool TDeviceDigitalScan::HasData( const common::TChipIndex idx )
{
    if ( !fScanHisto->IsValidChipIndex( idx ) ) {
        return false;
    }
    return fScanHisto->HasData( idx );
}

