#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilderMFTLadder.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "TReadoutBoardMOSAIC.h"
#include "TAlpide.h"
#include "MosaicSrc/mruncontrol.h"
#include "TChipConfig.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderMFTLadder::TDeviceBuilderMFTLadder() : TDeviceBuilder()
{ }

//___________________________________________________________________
TDeviceBuilderMFTLadder::~TDeviceBuilderMFTLadder()
{ }

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilderMFTLadder::SetDeviceType( const TDeviceType dt )
{
    if ( fCurrentDevice->IsConfigFrozen() || fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    fCurrentDevice->SetDeviceType( dt );
    switch ( fCurrentDevice->GetDeviceType() )
    {
        case TDeviceType::kMFT_LADDER5:
            fCurrentDevice->SetStartChipId( 4 ); // imposed by FPC
            fCurrentDevice->SetNChips( 5 ); // overwrite the value read in config file
            break;
        case TDeviceType::kMFT_LADDER4:
            fCurrentDevice->SetStartChipId( 5 ); // imposed by FPC
            fCurrentDevice->SetNChips( 4 ); // overwrite the value read in config file
            break;
        case TDeviceType::kMFT_LADDER3:
            fCurrentDevice->SetStartChipId( 6 ); // imposed by FPC
            fCurrentDevice->SetNChips( 3 ); // overwrite the value read in config file
            break;
        case TDeviceType::kMFT_LADDER2:
            fCurrentDevice->SetStartChipId( 7 ); // imposed by FPC
            fCurrentDevice->SetNChips( 2 ); // overwrite the value read in config file
            break;
        default:
            throw runtime_error( "TDeviceBuilderMFTLadder::SetDeviceType() - device is not a MFT ladder, doing nothing." );
            return;
    }
}

//___________________________________________________________________
void TDeviceBuilderMFTLadder::CreateDeviceConfig()
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    if ( !fCurrentDevice->IsMFTLadder() ) {
        throw runtime_error( "TDeviceBuilderMFTLadder::CreateDeviceConfig() - unknown device type, doing nothing." );
    }
    fCurrentDevice->SetBoardType( TBoardType::kBOARD_MOSAIC );
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fCurrentDevice->AddBoardConfig( newBoardConfig );
    
    for (int ichip = 0; ichip < fCurrentDevice->GetNChips(); ichip ++) {
        int chipID = fCurrentDevice->GetStartChipId() + ichip;
        auto newChipConfig = make_shared<TChipConfig>( chipID );
        fCurrentDevice->AddChipConfig( newChipConfig );
    }
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );
    
    if ( fVerboseLevel ) {
        cout << "TDeviceBuilderMFTLadder::CreateDeviceConfig() - done" << endl;
    }
    fCurrentDevice->FreezeConfig();
}

// Setup definition for MFT ladder with MOSAIC
//    - all chips connected to same control interface
//    - each chip has its own receiver, mapping is a non-trivial function of RCVMAP
//___________________________________________________________________
void TDeviceBuilderMFTLadder::InitSetup()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    try {
        if ( !fCurrentDevice->IsMFTLadder() ) {
            throw runtime_error( "TDeviceBuilderMFTLadder::InitSetup() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderMFTLadder::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    if ( fVerboseLevel ) {
        cout << "TDeviceBuilderMFTLadder::InitSetup() - start" << endl;
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = ((dynamic_pointer_cast<TBoardConfigMOSAIC>)(fCurrentDevice->GetBoardConfig(0)));
    boardConfig->SetInvertedData( false );
    Mosaic::TReceiverSpeed speed;
    
    switch ( (fCurrentDevice->GetChipConfig(0))->GetParamValue("LINKSPEED")) {
        case 400:
            speed = Mosaic::RCV_RATE_400;
            break;
        case 600:
            speed = Mosaic::RCV_RATE_600;
            break;
        case 1200:
            speed = Mosaic::RCV_RATE_1200;
            break;
        default:
            cout << "TDeviceBuilderMFTLadder::InitSetup() - Warning: invalid link speed, using 1200" << endl;
            speed = Mosaic::RCV_RATE_1200;
            break;
    }
    cout << "TDeviceBuilderMFTLadder::InitSetup() - Speed mode = " << speed << endl;
    boardConfig->SetSpeedMode( speed );
    
    auto newBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fCurrentDevice->AddBoard( newBoard );
    
    for (int i = 0; i < fCurrentDevice->GetNChips(); i++) {
        
        shared_ptr<TChipConfig> chipConfig = fCurrentDevice->GetChipConfig(i);
        int control  = chipConfig->GetParamValue("CONTROLINTERFACE");
        int receiver = chipConfig->GetParamValue("RECEIVER");
        auto alpide = make_shared<TAlpide>( chipConfig );
        alpide->SetReadoutBoard( fCurrentDevice->GetBoard(0) );
        fCurrentDevice->AddChip( alpide );
        
        if (control  < 0) {
            control = 0;
            chipConfig->SetParamValue("CONTROLINTERFACE", control);
        }
        if (receiver < 0) {
            // For MFT, the last chip (far from connector) is always:
            // - mapped with the RCVMAP[0]
            // - with chipId = 8
            // - at position GetNChips()-1 in the vector of chips.
            // And the chip id increases from the first chip (at position 0 in
            // the vector of chips, close to connector) to the last chip.
            // As a result:
            // RCVMAP[0] = 3, chipId = 8 (last chip on all types of ladder)
            // RCVMAP[1] = 5, chipId = 7 (1st chip on 2-chips ladder)
            // RCVMAP[2] = 7, chipId = 6 (1st chip on 3-chips ladder)
            // RCVMAP[3] = 8, chipId = 5 (1st chip on 4-chips ladder)
            // RCVMAP[4] = 6, chipId = 4 (1st chip on 5-chips ladder)
            int index = fCurrentDevice->GetNChips() - 1 - i;
            receiver = boardConfig->GetRCVMAP(index);
            chipConfig->SetParamValue("RECEIVER", receiver);
        }
        (fCurrentDevice->GetBoard(0))->AddChip( chipConfig->GetChipId(), control, receiver );
    }
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    fCurrentDevice->FreezeSetup();
}
