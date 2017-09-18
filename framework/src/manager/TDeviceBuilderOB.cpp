#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilderOB.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "TReadoutBoardMOSAIC.h"
#include "TChipConfig.h"
#include "TAlpide.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderOB::TDeviceBuilderOB() : TDeviceBuilderWithSlaveChips()
{ }

//___________________________________________________________________
TDeviceBuilderOB::~TDeviceBuilderOB()
{ }

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilderOB::SetDeviceType( const TDeviceType dt )
{
    if ( fCurrentDevice->IsConfigFrozen() || fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    fCurrentDevice->SetDeviceType( dt );
    fCurrentDevice->SetNChips( 15 ); // overwrite the value read in config file
}

//___________________________________________________________________
void TDeviceBuilderOB::SetVerboseLevel( const int level )
{
    if ( level > kTERSE ) {
        cout << "TDeviceBuilderOB::SetVerboseLevel() - " << level << endl;
    }
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceBuilderOB::CreateDeviceConfig()
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    fCurrentDevice->SetBoardType( TBoardType::kBOARD_MOSAIC );
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fCurrentDevice->AddBoardConfig( newBoardConfig );
    
    for (unsigned int ichip = 0; ichip < fCurrentDevice->GetNChips(); ichip ++) {
        if (ichip == 7) continue;
        int chipId = ichip + ((DEFAULT_MODULE_ID & 0x7) << 4);
        if ( ichip == 0 ) {
            fCurrentDevice->SetStartChipId( chipId ); // define the Id of the first chip
        }
        auto newChipConfig = make_shared<TChipConfig>( chipId );
        fCurrentDevice->AddChipConfig( newChipConfig );
    }
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );

    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderOB::CreateDeviceConfig() - done" << endl;
    }
    fCurrentDevice->FreezeConfig();
}

// Setup definition for outer barrel module with MOSAIC
//    - module ID (3 most significant bits of chip ID) defined by moduleId
//      (usually 1)
//    - chips connected to two different control interfaces
//    - masters send data to two different receivers (0 and 1)
//    - receiver number for slaves set to -1 (not connected directly to receiver)
//      (this ensures that a receiver is disabled only if the connected master is disabled)
//___________________________________________________________________
void TDeviceBuilderOB::InitSetup()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    try {
        if ( fCurrentDevice->GetDeviceType() != TDeviceType::kOBHIC ) {
            throw runtime_error( "TDeviceBuilderOB::InitSetup() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderOB::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderOB::InitSetup() - start" << endl;
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = ((dynamic_pointer_cast<TBoardConfigMOSAIC>)(fCurrentDevice->GetBoardConfig(0)));
    boardConfig->SetInvertedData(boardConfig->IsInverted()); // ???: circular definition? (AR)
    boardConfig->SetSpeedMode( Mosaic::RCV_RATE_400 );
    auto newBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fCurrentDevice->AddBoard( newBoard );
    
    for (unsigned int i = 0; i < fCurrentDevice->GetNChips(); i++) {
        
        shared_ptr<TChipConfig> chipConfig = fCurrentDevice->GetChipConfig(i);
        int chipId = chipConfig->GetChipId();
        int control = chipConfig->GetParamValue("CONTROLINTERFACE");
        int receiver = chipConfig->GetParamValue("RECEIVER");
        
        if (chipId%8!=0) chipConfig->SetParamValue("LINKSPEED", "-1"); // deactivate the DTU/PLL for none master chips
        
        if (i < 7) {              // first master-slave row
            if (control < 0) {
                control = 1;
                chipConfig->SetParamValue("CONTROLINTERFACE", control);
            }
            if (receiver < 0) {
                receiver = 9;
                chipConfig->SetParamValue("RECEIVER", receiver);
            }
        } else {                    // second master-slave row
            if (control < 0) {
                control = 0;
                chipConfig->SetParamValue("CONTROLINTERFACE", control);
            }
            if (receiver < 0) {
                receiver = 0;
                chipConfig->SetParamValue("RECEIVER", receiver);
            }
        }
        auto alpide = make_shared<TAlpide>( chipConfig );
        alpide->SetReadoutBoard( fCurrentDevice->GetBoard(0) );
        fCurrentDevice->AddChip( alpide );
        (fCurrentDevice->GetBoard(0))->AddChipConfig( chipConfig );
    }
    for (unsigned int i = 0; i < fCurrentDevice->GetNChips(); i++) {
        EnableSlave( i );
    }
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    sleep(5);
    MakeDaisyChain();
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderOB::InitSetup() - end" << endl;
    }
    fCurrentDevice->FreezeSetup();
    CountEnabledChipsPerBoard();
    PropagateVerbosityToBoards();
}
