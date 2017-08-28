#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilderOBSingleMosaic.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "TChipConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include "TAlpide.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderOBSingleMosaic::TDeviceBuilderOBSingleMosaic() : TDeviceBuilder()
{ }

//___________________________________________________________________
TDeviceBuilderOBSingleMosaic::~TDeviceBuilderOBSingleMosaic()
{ }

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilderOBSingleMosaic::SetDeviceType( const TDeviceType dt )
{
    if ( fCurrentDevice->IsConfigFrozen() || fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    fCurrentDevice->SetDeviceType( dt );
    fCurrentDevice->SetStartChipId( 16 ); // master OB chip
    fCurrentDevice->SetNChips( 1 ); // overwrite the value read in config file
}

//___________________________________________________________________
void TDeviceBuilderOBSingleMosaic::CreateDeviceConfig()
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    fCurrentDevice->SetBoardType( TBoardType::kBOARD_MOSAIC );
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fCurrentDevice->AddBoardConfig( newBoardConfig );
    
    auto newChipConfig = make_shared<TChipConfig>( fCurrentDevice->GetStartChipId() );
    newChipConfig->SetEnableSlave( false ); // with no slave
    fCurrentDevice->AddChipConfig( newChipConfig );
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );
   
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderOBSingleMosaic::CreateDeviceConfig() - done" << endl;
    }
    fCurrentDevice->FreezeConfig();
}

// Setup for a single chip in outer barrel mode with MOSAIC
// Assumption : ITS adaptors are used to interface the chip and the MOSAIC
// (see https://twiki.cern.ch/twiki/bin/view/ALICE/ALPIDE-adaptor-boards)
//___________________________________________________________________
void TDeviceBuilderOBSingleMosaic::InitSetup()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    try {
        if ( fCurrentDevice->GetDeviceType() != TDeviceType::kOBCHIP_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderOBSingleMosaic::InitSetup() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderOBSingleMosaic::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderOBSingleMosaic::InitSetup() - start" << endl;
    }
    
    shared_ptr<TChipConfig> chipConfig  = fCurrentDevice->GetChipConfig(0);
    int control  = chipConfig->GetParamValue("CONTROLINTERFACE");
    int receiver = chipConfig->GetParamValue("RECEIVER");
    
    if ( receiver < 0 ) {
        receiver = 3;   // HSData is connected to pins for first chip on a stave
        chipConfig->SetParamValue("RECEIVER", receiver);
    }
    if ( control  < 0 ) {
        control  = 0;
        chipConfig->SetParamValue("CONTROLINTERFACE", control);
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = ((dynamic_pointer_cast<TBoardConfigMOSAIC>) (fCurrentDevice->GetBoardConfig(0)));
    boardConfig->SetInvertedData( false );
    boardConfig->SetSpeedMode( Mosaic::RCV_RATE_400 );
    
    auto myBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fCurrentDevice->AddBoard( myBoard );
    
    auto alpide = make_shared<TAlpide>( chipConfig );
    alpide->SetReadoutBoard( fCurrentDevice->GetBoard(0) );
    fCurrentDevice->AddChip(alpide);
    (fCurrentDevice->GetBoard(0))->AddChipConfig( chipConfig );
    
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderOBSingleMosaic::InitSetup() - end" << endl;
    }
    fCurrentDevice->FreezeSetup();
    CountEnabledChipsPerBoard();
}
