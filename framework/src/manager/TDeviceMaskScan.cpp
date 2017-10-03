#include "TAlpideDecoder.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TChipConfig.h"
#include "Common.h"
#include "TDevice.h"
#include "TDeviceMaskScan.h"
#include "TErrorCounter.h"
#include "THisto.h"
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
TDeviceMaskScan::TDeviceMaskScan() :
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
TDeviceMaskScan::TDeviceMaskScan( shared_ptr<TDevice> aDevice,
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
        exit( EXIT_FAILURE );
    }
    fErrorCounter = make_shared<TErrorCounter>();
    fChipDecoder = make_unique<TAlpideDecoder>( fScanHisto, fErrorCounter );
    fBoardDecoder = make_unique<TBoardDecoder>();
}

//___________________________________________________________________
TDeviceMaskScan::~TDeviceMaskScan()
{
    if ( fScanHisto ) fScanHisto.reset();
    if ( fScanConfig ) fScanConfig.reset();
    if ( fErrorCounter ) fErrorCounter.reset();
}

//___________________________________________________________________
void TDeviceMaskScan::SetScanConfig( shared_ptr<TScanConfig> aScanConfig )
{
    if ( !aScanConfig ) {
        throw runtime_error( "TDeviceMaskScan::SetScanConfig() - can not use a null pointer !" );
    }
    fScanConfig = aScanConfig;
}

//___________________________________________________________________
void TDeviceMaskScan::SetVerboseLevel( const int level )
{
    fChipDecoder->SetVerboseLevel( level );
    fBoardDecoder->SetVerboseLevel( level );
    fScanHisto->SetVerboseLevel( level );
    fErrorCounter->SetVerboseLevel( level );
    TDeviceChipVisitor::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceMaskScan::Init()
{
    if ( !fScanConfig ) {
        throw runtime_error( "TDeviceMaskScan::Init() - can not use a null pointer for the scan config !" );
    }
    if ( !fScanHisto ) {
        throw runtime_error( "TDeviceMaskScan::Init() - can not use a null pointer for the map of scan histo !" );
    }
    
    InitScanParameters();
    AddHisto();
    fErrorCounter->Init( fScanHisto, fNTriggers );
    try {
        TDeviceChipVisitor::Init();
    } catch ( std::exception &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
}

//___________________________________________________________________
void TDeviceMaskScan::AddHisto()
{
    common::TChipIndex id;
    
    THisto histo ("DigScanHisto", "DigScanHisto",
                  common::MAX_DCOL+1, 0, common::MAX_DCOL,
                  common::MAX_ADDR+1, 0, common::MAX_ADDR);
    
    for ( unsigned int ichip = 0; ichip < fDevice->GetNChips(); ichip++ ) {
        if ( fDevice->GetChipConfig(ichip)->IsEnabled() ) {
            id.boardIndex   = fDevice->GetBoardIndexByChip(ichip);
            id.dataReceiver = fDevice->GetChipConfig(ichip)->GetParamValue("RECEIVER");
            id.ladderId     = fDevice->GetLadderId();
            id.chipId       = fDevice->GetChipId(ichip);
            fScanHisto->AddHisto( id, histo );
        }
    }
    fScanHisto->FindChipList();
    if ( GetVerboseLevel() > kSILENT ) {
        cout << endl << "TDeviceMaskScan::AddHisto() - generated map with " << std::dec << fScanHisto->GetSize() << " elements" << endl;
    }
    return;
}

//___________________________________________________________________
void TDeviceMaskScan::InitScanParameters()
{
    fNTriggers = fScanConfig->GetNInj();
    fNMaskStages = fScanConfig->GetNMaskStages();
    fNPixPerRegion = fScanConfig->GetPixPerRegion();
}

//___________________________________________________________________
void TDeviceMaskScan::ConfigureBoards()
{
    shared_ptr<TReadoutBoard> theBoard;
    
    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        theBoard = fDevice->GetBoard( iboard );
        if ( !theBoard ) {
            throw runtime_error( "TDeviceMaskScan::ConfigureBoard() - no readout board found." );
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
void TDeviceMaskScan::ConfigureChips()
{
    DoActivateConfigMode();
    DoBaseConfig();
    DoActivateReadoutMode();
}

//___________________________________________________________________
unsigned int TDeviceMaskScan::ReadEventData( const unsigned int iboard )
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
            if ( nTrials == TDeviceMaskScan::MAXTRIALS ) {
                if ( GetVerboseLevel() > kSILENT ) {
                    cout << "TDeviceMaskScan::ReadEventData() - board "
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
                cout << "TDeviceMaskScan::ReadEventData() - board "
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
                fErrorCounter->IncrementN8b10b( fBoardDecoder->GetMosaicChannel() );
            }
            if ( fBoardDecoder->GetMosaicTimeout() ) {
                fErrorCounter->IncrementNTimeout();
            }
            
            // decode Chip event
            int n_bytes_chipevent = n_bytes_data-n_bytes_header;// - n_bytes_trailer;
            if ( fBoardDecoder->GetMosaicEoeCount() < 2) {
                n_bytes_chipevent -= n_bytes_trailer;
            }
            bool isOk = fChipDecoder->DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent,
                                                  iboard,
                                                  fBoardDecoder->GetMosaicChannel(),
                                                  fDevice->GetLadderId() );
            
            if ( !isOk ) {
                if ( GetVerboseLevel() > kSILENT ) {
                    cout << "TDeviceMaskScan::ReadEventData() - board "
                    << std::dec << iboard
                    << " , found bad event " << endl;
                }
                fErrorCounter->IncrementNCorruptEvent();
                nBad++;
                if ( nBad > TDeviceMaskScan::MAXNBAD ) continue;
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
void TDeviceMaskScan::StartReadout()
{
    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        shared_ptr<TReadoutBoard> myBoard = fDevice->GetBoard(iboard);
        if ( !myBoard ) {
            throw runtime_error( "TDeviceMaskScan::Init() - no readout board found!" );
        }
        // readout reset
        myBoard->SendOpCode( (uint16_t)AlpideOpCode::RORST );
        
        shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(myBoard);
        
        myMOSAIC->StartRun();
    }
}

//___________________________________________________________________
void TDeviceMaskScan::StopReadout()
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
}


