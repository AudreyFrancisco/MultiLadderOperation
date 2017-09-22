#include "TAlpideDecoder.h"
#include "TAlpide.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TChipConfig.h"
#include "Common.h"
#include "TDevice.h"
#include "TDeviceDigitalScan.h"
#include "TErrorCounter.h"
#include "THisto.h"
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
fScanHisto( nullptr ),
fScanConfig( nullptr ),
fErrorCounter( nullptr ),
fChipDecoder( nullptr ),
fBoardDecoder( nullptr ),
fNTriggers( 0 ),
fNMaskStages( 0 ),
fNPixPerRegion( 0 )
{
    fScanHisto = make_shared<TScanHisto>();
    fErrorCounter = make_shared<TErrorCounter>();
    fChipDecoder = make_unique<TAlpideDecoder>( fScanHisto, fErrorCounter );
    fBoardDecoder = make_unique<TBoardDecoder>();
}

//___________________________________________________________________
TDeviceDigitalScan::TDeviceDigitalScan( shared_ptr<TDevice> aDevice,
                                       shared_ptr<TScanConfig> aScanConfig ) :
TDeviceChipVisitor( aDevice ),
fScanHisto( nullptr ),
fScanConfig( nullptr ),
fErrorCounter( nullptr ),
fChipDecoder( nullptr ),
fNTriggers( 0 ),
fNMaskStages( 0 ),
fNPixPerRegion( 0 )
{
    fScanHisto = make_shared<TScanHisto>();
    try {
        SetScanConfig( aScanConfig );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        exit(0);
    }
    fErrorCounter = make_shared<TErrorCounter>();
    fChipDecoder = make_unique<TAlpideDecoder>( fScanHisto, fErrorCounter );
    fBoardDecoder = make_unique<TBoardDecoder>();
}

//___________________________________________________________________
TDeviceDigitalScan::~TDeviceDigitalScan()
{
    if ( fScanHisto ) fScanHisto.reset();
    if ( fScanConfig ) fScanConfig.reset();
    if ( fErrorCounter ) fErrorCounter.reset();
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
void TDeviceDigitalScan::SetVerboseLevel( const int level )
{
    fChipDecoder->SetVerboseLevel( level );
    fBoardDecoder->SetVerboseLevel( level );
    fScanHisto->SetVerboseLevel( level );
    fErrorCounter->SetVerboseLevel( level );
    TDeviceChipVisitor::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceDigitalScan::Init()
{
    if ( !fScanConfig ) {
        throw runtime_error( "TDeviceDigitalScan::Init() - can not use a null pointer for the scan config !" );
    }
    if ( !fScanHisto ) {
        throw runtime_error( "TDeviceDigitalScan::Init() - can not use a null pointer for the map of scan histo !" );
    }
    
    InitScanParameters();
    AddHisto();
    fErrorCounter->Init( fScanHisto );
    
    try {
        TDeviceChipVisitor::Init();
    } catch ( std::exception &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    
    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        shared_ptr<TReadoutBoard> myBoard = fDevice->GetBoard(iboard);
        if ( !myBoard ) {
            throw runtime_error( "TDeviceDigitalScan::Init() - no readout board found!" );
        }
        // readout reset
        myBoard->SendOpCode( (uint16_t)AlpideOpCode::RORST );

        shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(myBoard);

        myMOSAIC->StartRun();
    }
}

//___________________________________________________________________
void TDeviceDigitalScan::Terminate()
{
    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {

        shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(fDevice->GetBoard( iboard ));
        
        shared_ptr<TReadoutBoardDAQ> myDAQBoard = dynamic_pointer_cast<TReadoutBoardDAQ>(fDevice->GetBoard( iboard ));

        if ( myMOSAIC ) {
            myMOSAIC->StopRun();
        }
        if ( myDAQBoard ) {
            myDAQBoard->PowerOff();
        }
    }
    FindDiscordantPixels();
    cout << endl;
    fErrorCounter->FindCorruptedHits();
    fErrorCounter->Dump();
    fIsTerminated = true;
}

//___________________________________________________________________
void TDeviceDigitalScan::WriteDataToFile( const char *fName, bool Recreate )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceDigitalScan::WriteDataToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceDigitalScan::WriteDataToFile() - not terminated ! Please use Terminate() first." );
    }
    fChipDecoder->WriteDataToFile( fName, Recreate );
}

//___________________________________________________________________
void TDeviceDigitalScan::WriteCorruptedHitsToFile( const char *fName, bool Recreate )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceDigitalScan::WriteCorruptedHitsToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceDigitalScan::WriteCorruptedHitsToFile() - not terminated ! Please use Terminate() first." );
    }
    fErrorCounter->WriteCorruptedHitsToFile( fName, Recreate );
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
        //DoActivateReadoutMode();
        
        
        //uint16_t Value;
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_CMU_DMU_CONFIG, Value );
        //cout << "CMU DMU Config: 0x" << std::hex << Value << std::dec << endl;
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_FROMU_STATUS1, Value );
        //cout << "Trigger counter before: " << Value << endl;
        for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
            // Send triggers for all boards
            (fDevice->GetBoard( ib ))->Trigger(fNTriggers);
        }
        //(fDevice->GetChip(0))->ReadRegister( Alpide::REG_FROMU_STATUS1, Value );
        //cout << "Trigger counter after: " << Value << endl;
        
        //(fDevice->GetBoard( 0 ))->SendOpCode( (uint16_t)Alpide::OPCODE_DEBUG );
        //(fDevice->GetChip(0))->PrintDebugStream();
        
        if ( istage ) {
            nHitsLastStage = fChipDecoder->GetNHits();
        }

        for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {

            // Read data for all boards
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
void TDeviceDigitalScan::InitScanParameters()
{
    fNTriggers = fScanConfig->GetNInj();
    fNMaskStages = fScanConfig->GetNMaskStages();
    fNPixPerRegion = fScanConfig->GetPixPerRegion();
}

//___________________________________________________________________
void TDeviceDigitalScan::ConfigureBoards()
{
    shared_ptr<TReadoutBoard> theBoard;

    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        theBoard = fDevice->GetBoard( iboard );
        if ( !theBoard ) {
            throw runtime_error( "TDeviceDigitalScan::ConfigureBoard() - no readout board found." );
        }
        
        if ( fDevice->GetBoardConfig(iboard)->GetBoardType() == TBoardType::kBOARD_DAQ ) { // DAQ board
            
            // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
            // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
            const bool enablePulse = true;
            const bool enableTrigger = false;
            const int triggerDelay = 0;
            const int pulseDelay = 2 * fDevice->GetBoardConfig(iboard)->GetParamValue( "STROBEDELAYBOARD" );
            theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
            theBoard->SetTriggerSource( TTriggerSource::kTRIG_EXT );
            
        } else { // MOSAIC board
            
            const bool enablePulse = true;
            const bool enableTrigger = true;
            const int triggerDelay = fDevice->GetBoardConfig(iboard)->GetParamValue("STROBEDELAYBOARD");
            const int pulseDelay = fDevice->GetBoardConfig(iboard)->GetParamValue("PULSEDELAY");
            theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
            theBoard->SetTriggerSource( TTriggerSource::kTRIG_INT );
        }
    }
}

//___________________________________________________________________
void TDeviceDigitalScan::ConfigureChips()
{
    DoActivateConfigMode();
    DoBaseConfig();
    DoActivateReadoutMode();
}

//___________________________________________________________________
unsigned int TDeviceDigitalScan::ReadEventData( const unsigned int iboard )
{
    unsigned char buffer[1024*4000];
    int n_bytes_data, n_bytes_header, n_bytes_trailer;
    
    unsigned int nHitsTot = 0, nHitsLastStage = 0;
    
    unsigned int itrg = 0;
    unsigned int nTrials = 0;
    
    while( itrg < fNTriggers * fDevice->GetNWorkingChipsPerBoard( iboard ) ) {
        
        unsigned int nBad       = 0;
        
        if ( (fDevice->GetBoard( iboard ))->ReadEventData(n_bytes_data, buffer) == -1) {
            
            // no event available in buffer yet, wait a bit
            usleep(100);
            nTrials ++;
            if ( nTrials == TDeviceDigitalScan::MAXTRIALS ) {
                if ( GetVerboseLevel() > kSILENT ) {
                    cout << "TDeviceDigitalScan::ReadEventData() - board "
                    << std::dec << iboard
                    << " , reached " << nTrials
                    << " timeouts, giving up on this point." << endl;
                }
                itrg = fNTriggers * fDevice->GetNWorkingChipsPerBoard( iboard ) ;
                fErrorCounter->IncrementNTimeout();
                nTrials = 0;
            }
            continue;
            
        } else {
            
            if ( GetVerboseLevel() > kVERBOSE ) {
                cout << "TDeviceDigitalScan::ReadEventData() - board "
                << std::dec << iboard
                << " , received event " << itrg << " with length "
                << n_bytes_data << endl;
                for ( int iByte = 0; iByte < n_bytes_data; ++iByte ) {
                    printf ("%02x ", (int) buffer[iByte]);
                }
                cout << endl;
            }
            
            // decode readout board event
            shared_ptr<TBoardConfig> boardConfig = fDevice->GetBoardConfig( iboard );
            fBoardDecoder->SetBoardType( boardConfig->GetBoardType() );
            if ( boardConfig->GetBoardType() == TBoardType::kBOARD_MOSAIC ) {
                shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(fDevice->GetBoard( iboard ));
                fBoardDecoder->SetFirmwareVersion( myMOSAIC->GetFwIdString() );
            }
            fBoardDecoder->DecodeEvent( buffer, n_bytes_data, n_bytes_header, n_bytes_trailer );
            if ( fBoardDecoder->GetMosaicDecoder10b8bError() ) {
                fErrorCounter->IncrementN8b10b();
            }
            if ( fBoardDecoder->GetMosaicTimeout() ) {
                fErrorCounter->IncrementNTimeout();
            }
            
            // decode Chip event
            int n_bytes_chipevent = n_bytes_data-n_bytes_header;// - n_bytes_trailer;
            if ( fBoardDecoder->GetMosaicEoeCount() < 2) {
                n_bytes_chipevent -= n_bytes_trailer;
            }
            bool isOk = fChipDecoder->DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, iboard, fBoardDecoder->GetMosaicChannel() );
            
            if ( !isOk ) {
                if ( GetVerboseLevel() > kSILENT ) {
                    cout << "TDeviceDigitalScan::ReadEventData() - board "
                    << std::dec << iboard
                    << " , found bad event " << endl;
                }
                fErrorCounter->IncrementNCorruptEvent();
                nBad++;
                if ( nBad > TDeviceDigitalScan::MAXNBAD ) continue;
                FILE* fDebug = fopen ("../../data/DebugData.dat", "a");
                if ( fDebug ) {
                    for ( int iByte=0; iByte<n_bytes_data; ++iByte ) {
                        fprintf (fDebug, "%02x ", (int) buffer[iByte]);
                    }
                    fprintf(fDebug, "\nFull Event:\n");
                    for (unsigned int ibyte = 0; ibyte < fDebugBuffer.size(); ibyte ++) {
                        fprintf (fDebug, "%02x ", (int) fDebugBuffer.at(ibyte));
                    }
                    fprintf(fDebug, "\n\n");
                    fclose( fDebug );
                }
            }
            itrg++;
        }
    }
    return itrg;
}

//___________________________________________________________________
void TDeviceDigitalScan::FindDiscordantPixels()
{
    bool isFullMatrix = (( fNMaskStages == 512 ) && ( fNPixPerRegion == 32 ));
    
    if ( !isFullMatrix  ) {
        cout << "TDeviceDigitalScan::FindDiscordantPixels() - not implemented when only part of the pixel matrix is tested. Please test the full matrix." << endl;
        return;
    }
    for ( unsigned int ichip = 0; ichip < fScanHisto->GetChipListSize(); ichip++ ) {
        
        for (unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++) {
            for (unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++) {
                
                common::TChipIndex idx = fScanHisto->GetChipIndex(ichip);
                if ( (*fScanHisto)(idx,icol,iaddr) != fNTriggers ) {
                    if ( (*fScanHisto)(idx,icol,iaddr) == 0 ) {
                        fErrorCounter->AddDeadPixel( idx, icol, iaddr );
                    }
                    if ( (*fScanHisto)(idx,icol,iaddr) < fNTriggers ) {
                        fErrorCounter->AddInefficientPixel( idx, icol, iaddr );
                    }
                    if ( (*fScanHisto)(idx,icol,iaddr) > fNTriggers ) {
                        fErrorCounter->AddHotPixel( idx, icol, iaddr );
                    }
                }
                
            } // end of loop on iaddr
        } // end of loop on icol
        
    } // end of loop on ichip
    
}

