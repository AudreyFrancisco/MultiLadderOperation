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
//#include "MosaicSrc/mruncontrol.h"
#include "mruncontrol.h"
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
            fCurrentDevice->SetStartChipId( 4 ); // imposed by FPC (chip near connector)
            fCurrentDevice->SetNChips( 5 ); // overwrite the value read in config file
            break;
        case TDeviceType::kMFT_LADDER4:
            fCurrentDevice->SetStartChipId( 5 ); // imposed by FPC (chip near connector)
            fCurrentDevice->SetNChips( 4 ); // overwrite the value read in config file
            break;
        case TDeviceType::kMFT_LADDER3:
            fCurrentDevice->SetStartChipId( 6 ); // imposed by FPC (chip near connector)
            fCurrentDevice->SetNChips( 3 ); // overwrite the value read in config file
            break;
        case TDeviceType::kMFT_LADDER2:
            fCurrentDevice->SetStartChipId( 7 ); // imposed by FPC (chip near connector)
            fCurrentDevice->SetNChips( 2 ); // overwrite the value read in config file
            break;
        default:
            throw runtime_error( "TDeviceBuilderMFTLadder::SetDeviceType() - device is not a MFT ladder, doing nothing." );
            return;
    }
}

//___________________________________________________________________
void TDeviceBuilderMFTLadder::SetVerboseLevel( const int level )
{
    if ( level > kTERSE ) {
        cout << "TDeviceBuilderMFTLadder::SetVerboseLevel() - " << level << endl;
    }
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceBuilderMFTLadder::SetDeviceId( const unsigned int number )
{
    if ( GetVerboseLevel() > kTERSE ) {
        cout << "TDeviceBuilderMFTLadder::SetDeviceId() - " << number << endl;
    }
    fCurrentDevice->SetDeviceId( number );
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
    
    for (unsigned int ichip = 0; ichip < fCurrentDevice->GetNChips(); ichip ++) {
        int chipID = fCurrentDevice->GetStartChipId() + ichip;
        auto newChipConfig = make_shared<TChipConfig>( chipID );
        fCurrentDevice->AddChipConfig( newChipConfig );
    }
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );
    
    if ( fVerboseLevel > kTERSE ) {
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
        exit( EXIT_FAILURE );
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderMFTLadder::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    if ( GetVerboseLevel() > kTERSE ) {
        cout << "TDeviceBuilderMFTLadder::InitSetup() - start" << endl;
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = ((dynamic_pointer_cast<TBoardConfigMOSAIC>)(fCurrentDevice->GetBoardConfig(0)));
    boardConfig->SetInvertedData( false );
    MosaicReceiverSpeed speed;
    
    switch ( (fCurrentDevice->GetChipConfig(0))->GetParamValue("LINKSPEED")) {
        case (int)AlpideIBSerialLinkSpeed::IB400:
            speed = MosaicReceiverSpeed::RCV_RATE_400;
            break;
        case (int)AlpideIBSerialLinkSpeed::IB600:
            speed = MosaicReceiverSpeed::RCV_RATE_600;
            break;
        case (int)AlpideIBSerialLinkSpeed::IB1200:
            speed = MosaicReceiverSpeed::RCV_RATE_1200;
            break;
        default:
            cout << "TDeviceBuilderMFTLadder::InitSetup() - Warning: invalid link speed, using 1200" << endl;
            speed = MosaicReceiverSpeed::RCV_RATE_1200;
            break;
    }
    if ( GetVerboseLevel() > kSILENT ) {
        cout << "TDeviceBuilderMFTLadder::InitSetup() - Speed mode = " << (int)speed << endl;
    }
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
        chipConfig->SetPreviousId(0xf); // see page 75 alpide manual (section 3.8.3)
        chipConfig->SetInitialToken(false); // see page 75 alpide manual (section 3.8.3)
        auto alpide = make_shared<TAlpide>( chipConfig );
        alpide->SetReadoutBoard( fCurrentDevice->GetBoard(0) );
        fCurrentDevice->AddChip( alpide );
        (fCurrentDevice->GetBoard(0))->AddChipConfig( chipConfig );
    }

    // Check if the device id was given when TSetup or in the config file.
    // We assume that the non-zero device id in the config file must be used if no 
    // id was given when TSetup (i.e. the device id still has its default value).
    const unsigned int configDeviceId = boardConfig->GetDeviceId();
    if ( (fCurrentDevice->GetDeviceId() == 0) && (configDeviceId != 0)  ) {
        fCurrentDevice->SetDeviceId( configDeviceId );
    }

    fCurrentDevice->EnableClockOutputs( true );
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderMFTLadder::InitSetup() - end" << endl;
    }
    fCurrentDevice->FreezeSetup();
    CountEnabledChipsPerBoard();
    PropagateVerbosityToBoards();
}
