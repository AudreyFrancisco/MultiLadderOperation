#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilderIBSingleMosaic.h"
#include "TBoardConfigMOSAIC.h"
#include "TChipConfig.h"
#include "TAlpide.h"
#include "TReadoutBoardMOSAIC.h"
#include "MosaicSrc/mruncontrol.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderIBSingleMosaic::TDeviceBuilderIBSingleMosaic() : TDeviceBuilder()
{ }

//___________________________________________________________________
TDeviceBuilderIBSingleMosaic::~TDeviceBuilderIBSingleMosaic()
{ }

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilderIBSingleMosaic::SetDeviceType( const TDeviceType dt )
{
    if ( fCurrentDevice->IsConfigFrozen() || fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    fCurrentDevice->SetDeviceType( dt );
    fCurrentDevice->SetStartChipId( 0 ); // imposed by carrier board (resistor removed)
    fCurrentDevice->SetNChips( 1 ); // overwrite the value read in config file
}

//___________________________________________________________________
void TDeviceBuilderIBSingleMosaic::CreateDeviceConfig()
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderIBSingleMosaic::CreateDeviceConfig() - start" << endl;
    }

    fCurrentDevice->SetBoardType( TBoardType::kBOARD_MOSAIC );
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fCurrentDevice->AddBoardConfig( newBoardConfig );
    
    auto newChipConfig = make_shared<TChipConfig>( fCurrentDevice->GetStartChipId() );
    fCurrentDevice->AddChipConfig( newChipConfig );
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );

    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderIBSingleMosaic::CreateDeviceConfig() - done" << endl;
    }
    fCurrentDevice->FreezeConfig();
}

// Setup for a single chip in inner barrel mode with MOSAIC
// Assumption : ICM_H board is used to interface the chip and the MOSAIC
//___________________________________________________________________
void TDeviceBuilderIBSingleMosaic::InitSetup()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    try {
        if ( fCurrentDevice->GetDeviceType() != TDeviceType::kIBCHIP_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderIBSingleMosaic::InitSetup() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderIBSingleMosaic::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    if ( GetVerboseLevel() > kTERSE ) {
        cout << "TDeviceBuilderIBSingleMosaic::InitSetup() - start" << endl;
    }
    
    shared_ptr<TChipConfig> chipConfig  = fCurrentDevice->GetChipConfig(0);
    int control  = chipConfig->GetParamValue("CONTROLINTERFACE");
    int receiver = chipConfig->GetParamValue("RECEIVER");
    
    if ( receiver < 0 ) {
        // Imposed by hardware of ICM_H board
        receiver = 4;
        chipConfig->SetParamValue("RECEIVER", receiver);
    }
    if ( control  < 0 ) {
        control  = 0;
        chipConfig->SetParamValue("CONTROLINTERFACE", control);
    }
    chipConfig->SetPreviousId(0xf); // see page 75 alpide manual (section 3.8.3)
    chipConfig->SetInitialToken(false); // see page 75 alpide manual (section 3.8.3)

    shared_ptr<TBoardConfigMOSAIC> boardConfig = ((dynamic_pointer_cast<TBoardConfigMOSAIC>) (fCurrentDevice->GetBoardConfig(0)));
    boardConfig->SetInvertedData( false );
    Mosaic::TReceiverSpeed speed;
    switch ( chipConfig->GetParamValue("LINKSPEED") ) {
        case (int)AlpideIBSerialLinkSpeed::IB400:
            speed = Mosaic::RCV_RATE_400;
            break;
        case (int)AlpideIBSerialLinkSpeed::IB600:
            speed = Mosaic::RCV_RATE_600;
            break;
        case (int)AlpideIBSerialLinkSpeed::IB1200:
            speed = Mosaic::RCV_RATE_1200;
            break;
        default:
            cout << "TDeviceBuilderIBSingleMosaic::InitSetup() - Warning: invalid link speed, using 1200" << endl;
            speed = Mosaic::RCV_RATE_1200;
            break;
    }
    boardConfig->SetSpeedMode( speed );
    
    auto myBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fCurrentDevice->AddBoard( myBoard );
    
    auto alpide = make_shared<TAlpide>( chipConfig );
    alpide->SetReadoutBoard( fCurrentDevice->GetBoard(0) );
    fCurrentDevice->AddChip( alpide );
    (fCurrentDevice->GetBoard(0))->AddChipConfig( chipConfig );
    
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }

    if ( GetVerboseLevel() > kTERSE ) {
        cout << "TDeviceBuilderIBSingleMosaic::InitSetup() - end" << endl;
    }
    fCurrentDevice->FreezeSetup();
}

