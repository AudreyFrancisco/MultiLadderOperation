#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilderIB.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "MosaicSrc/mruncontrol.h"
#include "TReadoutBoardMOSAIC.h"
#include "TChipConfig.h"
#include "TAlpide.h"


using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderIB::TDeviceBuilderIB() : TDeviceBuilder()
{ }

//___________________________________________________________________
TDeviceBuilderIB::~TDeviceBuilderIB()
{ }

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilderIB::SetDeviceType( const TDeviceType dt )
{
    if ( fCurrentDevice->IsConfigFrozen() || fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    fCurrentDevice->SetDeviceType( dt );
    fCurrentDevice->SetStartChipId( 0 ); // define the Id of the first chip
    fCurrentDevice->SetNChips( 9 ); // overwrite the value read in config file
}

//___________________________________________________________________
void TDeviceBuilderIB::SetVerboseLevel( const int level )
{
    if ( level > kTERSE ) {
        cout << "TDeviceBuilderIB::SetVerboseLevel() - " << level << endl;
    }
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceBuilderIB::CreateDeviceConfig()
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    fCurrentDevice->SetBoardType( TBoardType::kBOARD_MOSAIC );
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fCurrentDevice->AddBoardConfig( newBoardConfig );
    
    for (unsigned int ichip = 0; ichip < fCurrentDevice->GetNChips(); ichip ++) {
        int chipID = ichip;
        auto newChipConfig = make_shared<TChipConfig>( chipID );
        fCurrentDevice->AddChipConfig( newChipConfig );
    }
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );
    
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderIB::CreateDeviceConfig() - done" << endl;
    }
    fCurrentDevice->FreezeConfig();
}

// Setup definition for inner barrel stave with MOSAIC
//    - all chips connected to same control interface
//    - each chip has its own receiver, mapping defined in RCVMAP
//___________________________________________________________________
void TDeviceBuilderIB::InitSetup()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    try {
        if ( fCurrentDevice->GetDeviceType() != TDeviceType::kIBHIC ) {
            throw runtime_error( "TDeviceBuilderIB::InitSetup() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderIB::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderIB::InitSetup() - start" << endl;
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = ((dynamic_pointer_cast<TBoardConfigMOSAIC>)(fCurrentDevice->GetBoardConfig(0)));
    boardConfig->SetInvertedData( false );
    Mosaic::TReceiverSpeed speed;
    
    switch ((fCurrentDevice->GetChipConfig(0))->GetParamValue("LINKSPEED")) {
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
            cout << "TDeviceBuilderIB::InitSetup() - Warning: invalid link speed, using 1200" << endl;
            speed = Mosaic::RCV_RATE_1200;
            break;
    }
    cout << "TDeviceBuilderIB::InitSetup() - Speed mode = " << speed << endl;
    boardConfig->SetSpeedMode( speed );
    
    auto newBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fCurrentDevice->AddBoard( newBoard );
    
    for (unsigned int i = 0; i < fCurrentDevice->GetNChips(); i++) {
        
        shared_ptr<TChipConfig> chipConfig = fCurrentDevice->GetChipConfig(i);
        int control  = chipConfig->GetParamValue("CONTROLINTERFACE");
        int receiver = chipConfig->GetParamValue("RECEIVER");
        if (control  < 0) {
            control = 0;
            chipConfig->SetParamValue("CONTROLINTERFACE", control);
        }
        if (receiver < 0) {
            receiver = boardConfig->GetRCVMAP(i);
            chipConfig->SetParamValue("RECEIVER", receiver);
        }
        chipConfig->SetPreviousId(0xf); // see page 75 alpide manual (section 3.8.3)
        chipConfig->SetInitialToken(false); // see page 75 alpide manual (section 3.8.3)
        auto alpide = make_shared<TAlpide>( chipConfig );
        alpide->SetReadoutBoard( fCurrentDevice->GetBoard(0) );
        fCurrentDevice->AddChip( alpide );
        (fCurrentDevice->GetBoard(0))->AddChipConfig( chipConfig );
    }
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderIB::InitSetup() - end" << endl;
    }
    fCurrentDevice->FreezeSetup();
    CountEnabledChipsPerBoard();
}
