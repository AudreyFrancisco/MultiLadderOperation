#include "TAlpideDecoder.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TChipConfig.h"
#include "TDevice.h"
#include "TDeviceHitScan.h"
#include "TErrorCounter.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TScanConfig.h"
#include "THisto.h"
#include "mdictionary.h"
#include "TStorePixHit.h"
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <string.h>

using namespace std;

//___________________________________________________________________
TDeviceHitScan::TDeviceHitScan() :
TDeviceChipVisitor(),
fScanConfig( nullptr ),
fScanHisto( nullptr ),
fErrorCounter( nullptr ),
fChipDecoder( nullptr ),
fBoardDecoder( nullptr ),
fNTriggers( 0 ),
fStorePixHit( nullptr ),
fProduceTTree( false )
{
    fErrorCounter = make_shared<TErrorCounter>();
    fBoardDecoder = make_unique<TBoardDecoder>();
    fStorePixHit = make_shared<TStorePixHit>();

}

//___________________________________________________________________
TDeviceHitScan::TDeviceHitScan( shared_ptr<TDevice> aDevice,
                                shared_ptr<TScanConfig> aScanConfig,
                                const bool produceTTree ) :
TDeviceChipVisitor( aDevice ),
fScanConfig( nullptr ),
fScanHisto( nullptr ),
fErrorCounter( nullptr ),
fChipDecoder( nullptr ),
fNTriggers( 0 ),
fStorePixHit( nullptr ),
fProduceTTree( produceTTree )
{
    try {
        SetScanConfig( aScanConfig );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        exit( EXIT_FAILURE );
    }
    fScanHisto = make_shared<TScanHisto>();
    fErrorCounter = make_shared<TErrorCounter>( aDevice->GetDeviceType() );
    fStorePixHit = make_shared<TStorePixHit>();
    fChipDecoder  = make_unique<TAlpideDecoder>( aDevice, fErrorCounter, fStorePixHit );
    fBoardDecoder = make_unique<TBoardDecoder>();

}

//___________________________________________________________________
TDeviceHitScan::~TDeviceHitScan()
{
    if ( fErrorCounter ) fErrorCounter.reset();
    if ( fScanConfig ) fScanConfig.reset();
    if ( fScanHisto ) fScanHisto.reset();
}

//___________________________________________________________________
void TDeviceHitScan::SetScanConfig( shared_ptr<TScanConfig> aScanConfig )
{
    if ( !aScanConfig ) {
        throw runtime_error( "TDeviceHitScan::SetScanConfig() - can not use a null pointer !" );
    }
    fScanConfig = aScanConfig;
}

//___________________________________________________________________
void TDeviceHitScan::SetVerboseLevel( const int level )
{
    fScanHisto->SetVerboseLevel( level );
    fChipDecoder->SetVerboseLevel( level );
    fBoardDecoder->SetVerboseLevel( level );
    fErrorCounter->SetVerboseLevel( level );
    TDeviceChipVisitor::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceHitScan::SetRescueBadChipId( const bool permit )
{
    fChipDecoder->SetRescueBadChipId( permit );
}

//___________________________________________________________________
void TDeviceHitScan::Init()
{
    if ( !fScanConfig ) {
        throw runtime_error( "TDeviceHitScan::Init() - can not use a null pointer for the scan config !" );
    }
    InitScanParameters();
    AddHisto();
    try {
        TDeviceChipVisitor::Init();
    } catch ( std::exception &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    if ( !fScanHisto ) {
        throw runtime_error( "TDeviceHitScan::Init() - can not use a null pointer for the map of scan histo !" );
    }
    fChipDecoder->SetScanHisto( fScanHisto );
    fErrorCounter->Init( fScanHisto, fNTriggers );
    shared_ptr<TBoardConfigMOSAIC> myMOSAICboardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC>(fDevice->GetBoardConfig(0));
    if ( myMOSAICboardConfig->IsTrgRecorderEnable() ) {
        if ( IsTTreeActivated() ) {
            if ( GetVerboseLevel() > kTERSE ) 
                cout << "TDeviceHitScan::Init() - output TTree with hit pixels enabled" << endl;
                fStorePixHit->SetNames( fName.c_str(), fDevice->GetWorkingChipIndex(0) );
                fStorePixHit->Init();
        }
    } else {
        fProduceTTree = false;
    }
}

//___________________________________________________________________
void TDeviceHitScan::InitScanParameters()
{
    fNTriggers = fScanConfig->GetNInj();
}

//___________________________________________________________________
void TDeviceHitScan::ConfigureChips()
{
    DoActivateConfigMode();
    DoBaseConfig();
    DoActivateReadoutMode();
}

//___________________________________________________________________
unsigned int TDeviceHitScan::ReadEventData( const unsigned int iboard, int nTriggers )
{
    unsigned char buffer[1024*4000];
    int n_bytes_data, n_bytes_header, n_bytes_trailer;
        
    unsigned int itrg = 0;
    unsigned int nTrials = 0;
    uint32_t trgNum = 0;
	uint64_t trgTime = 0;

    if ( nTriggers <= 0 ) nTriggers = fNTriggers;
    
    while( itrg < nTriggers * fDevice->GetNWorkingChipsPerBoard( iboard ) ) {
        
        unsigned int nBad  = 0;
        int readDataFlag = (fDevice->GetBoard( iboard ))->ReadEventData(n_bytes_data, buffer);
        
        if ( readDataFlag == MosaicDict::kEMPTY_EVENT ) {
            
            // no event available in buffer yet, wait a bit
            usleep(100);
            nTrials ++;
            if ( nTrials == TDeviceHitScan::MAXTRIALS ) {
                if ( GetVerboseLevel() > kSILENT ) {
                    cout << "TDeviceHitScan::ReadEventData() - board "
                    << std::dec << iboard
                    << " , reached " << nTrials
                    << " timeouts, giving up on this point." << endl;
                }
                itrg = nTriggers * fDevice->GetNWorkingChipsPerBoard( iboard ) ;
                fErrorCounter->IncrementNTimeout();
                nTrials = 0;
            }
            continue;
            
        } else {

            shared_ptr<TBoardConfig> boardConfig = fDevice->GetBoardConfig( iboard );
            fBoardDecoder->SetBoardType( boardConfig->GetBoardType() );

            if ( boardConfig->GetBoardType() == TBoardType::kBOARD_MOSAIC ) {

                shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(fDevice->GetBoard( iboard ));
                fBoardDecoder->SetFirmwareVersion( myMOSAIC->GetFwIdString() );

                if ( readDataFlag == MosaicDict::kTRGRECORDER_EVENT ) {
                    trgNum = myMOSAIC->GetTriggerNum();
                    trgTime = myMOSAIC->GetTriggerTime();
                    if ( GetVerboseLevel() > kULTRACHATTY ) {
                        cout << "TDeviceHitScan::ReadEventData() - board " 
                             << std::dec << iboard << " trigger recorded " 
                             << trgNum << " @ " << trgTime << endl;
                    }
                    continue;
                }
            }
                        
            // decode readout board event
            fBoardDecoder->DecodeEvent( buffer, n_bytes_data, n_bytes_header, n_bytes_trailer );
            if ( fBoardDecoder->GetMosaicDecoder10b8bError() ) {
                fErrorCounter->IncrementN8b10b( fBoardDecoder->GetMosaicChannel() );
            }
            if ( fBoardDecoder->GetMosaicTimeout() ) {
                fErrorCounter->IncrementNTimeout();
            }

            if ( GetVerboseLevel() > kVERBOSE ) {
                cout << "TDeviceHitScan::ReadEventData() - board "
                << std::dec << iboard
                << " , received event " << itrg << " with length "
                << n_bytes_data << endl;
                for ( int iByte = 0; iByte < n_bytes_data; ++iByte ) {
                    printf ("%02x ", (int) buffer[iByte]);
                }
                cout << endl;
            }
            
            // decode Chip event
            int n_bytes_chipevent = n_bytes_data-n_bytes_header;// - n_bytes_trailer;
            if ( fBoardDecoder->GetMosaicEoeCount() < 2) {
                n_bytes_chipevent -= n_bytes_trailer;
            }
            bool isOk = fChipDecoder->DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent,
                                                  iboard,
                                                  fBoardDecoder->GetMosaicChannel(),
                                                  trgNum, trgTime );
            
            if ( !isOk ) {
                if ( GetVerboseLevel() > kSILENT ) {
                    cout << "TDeviceHitScan::ReadEventData() - board "
                    << std::dec << iboard
                    << " , found bad event " << endl;
                }
                fErrorCounter->IncrementNCorruptEvent();
                nBad++;
                if ( nBad > TDeviceHitScan::MAXNBAD ) continue;
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
void TDeviceHitScan::StartReadout()
{
    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        shared_ptr<TReadoutBoard> myBoard = fDevice->GetBoard(iboard);
        if ( !myBoard ) {
            throw runtime_error( "TDeviceHitScan::Init() - no readout board found!" );
        }
        
        shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(myBoard);
        
        myMOSAIC->StartRun();
    }
}

//___________________________________________________________________
void TDeviceHitScan::StopReadout()
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

//___________________________________________________________________
void TDeviceHitScan::DoBroadcastReset()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceHitScan::DoBroadcastReset() - can not use a null pointer for the device !" );
    }
    if ( fIsInitDone ) {
        cerr << "TDeviceHitScan::DoBroadcastReset() - already done ! Doing nothing." << endl;
        return;
    }
    if ( fDevice->GetNBoards(false) == 0 ) {
        throw runtime_error( "TDeviceHitScan::DoBroadcastReset() - no readout board found !" );
    }

    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        shared_ptr<TReadoutBoard> myBoard = fDevice->GetBoard( iboard );
        if ( !myBoard ) {
            throw runtime_error( "TDeviceHitScan::DoBroadcastReset() - no readout board found!" );
        }
        
        // -- global reset chips
        
        myBoard->SendBroadcastReset();
        myBoard->SendBroadcastROReset();
        myBoard->SendBroadcastBCReset();
        
        // TODO: check if AlpideOpCode::PRST is needed ?
        // -- pixel matrix reset
        // (does not affect the PULSE_EN and MASK_EN latches)
        
        myBoard->SendOpCode( (uint16_t)AlpideOpCode::PRST );
    }
}
