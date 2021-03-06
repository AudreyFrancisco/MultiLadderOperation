#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilderOBSingleDAQ.h"
#include "TBoardConfig.h"
#include "TChipConfig.h"
#include "TAlpide.h"
#include "TReadoutBoardDAQ.h"


using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderOBSingleDAQ::TDeviceBuilderOBSingleDAQ() : TDeviceBuilderWithDAQBoards()
{ }

//___________________________________________________________________
TDeviceBuilderOBSingleDAQ::~TDeviceBuilderOBSingleDAQ()
{ }

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilderOBSingleDAQ::SetDeviceType( const TDeviceType dt )
{
    if ( fCurrentDevice->IsConfigFrozen() || fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    fCurrentDevice->SetDeviceType( dt );
    fCurrentDevice->SetStartChipId( 16 ); // master OB chip
    fCurrentDevice->SetNChips( 1 ); // overwrite the value read in config file
}

//___________________________________________________________________
void TDeviceBuilderOBSingleDAQ::SetVerboseLevel( const int level )
{
    if ( level > kTERSE ) {
        cout << "TDeviceBuilderOBSingleDAQ::SetVerboseLevel() - " << level << endl;
    }
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceBuilderOBSingleDAQ::CreateDeviceConfig()
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    fCurrentDevice->SetBoardType( TBoardType::kBOARD_DAQ );
    auto newBoardConfig = make_shared<TBoardConfigDAQ>();
    fCurrentDevice->AddBoardConfig( newBoardConfig );
    
    auto newChipConfig = make_shared<TChipConfig>( fCurrentDevice->GetStartChipId() );
    newChipConfig->SetEnableSlave( false ); // with no slave
    fCurrentDevice->AddChipConfig( newChipConfig );
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );

    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderOBSingleDAQ::CreateDeviceConfig() - done" << endl;
    }
    fCurrentDevice->FreezeConfig();
}

//___________________________________________________________________
void TDeviceBuilderOBSingleDAQ::InitSetup()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    try {
        if ( fCurrentDevice->GetDeviceType() != TDeviceType::kCHIP_DAQ ) {
            throw runtime_error( "TDeviceBuilderOBSingleDAQ::InitSetup() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_DAQ ) {
            throw runtime_error( "TDeviceBuilderOBSingleDAQ::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderOBSingleDAQ::InitSetup() - start" << endl;
    }
    
    InitLibUsb();
    //  The following code searches the USB bus for DAQ boards, creates them and adds them to the readout board vector:
    try {
        FindDAQBoards();
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    if ( fCurrentDevice->GetNBoards( false ) != 1 ) {
        throw runtime_error( "TDeviceBuilderOBSingleDAQ::InitSetup() - Error in creating readout board object." );
    }
    
    shared_ptr<TChipConfig> chipConfig = fCurrentDevice->GetChipConfig(0);
    chipConfig->SetParamValue("LINKSPEED", "-1");
    // values for control interface and receiver currently ignored for DAQ board
    int control = 0;
    chipConfig->SetParamValue("CONTROLINTERFACE", control);
    int receiver = 0;
    chipConfig->SetParamValue("RECEIVER", receiver);
    // for Cagliari DAQ board disable DDR and Manchester encoding
    chipConfig->SetEnableDdr( false );
    chipConfig->SetDisableManchester( true );
    
    // create chip object and connections with readout board
    auto alpide = make_shared<TAlpide>( chipConfig );
    alpide->SetReadoutBoard( fCurrentDevice->GetBoard(0) );
    fCurrentDevice->AddChip( alpide );
    (fCurrentDevice->GetBoard(0))->AddChipConfig( chipConfig );
    
    shared_ptr<TReadoutBoardDAQ> myDAQBoard = ((dynamic_pointer_cast<TReadoutBoardDAQ>)(fCurrentDevice->GetBoard(0)));
    PowerOnDaqBoard(myDAQBoard);
    
    if ( fVerboseLevel > kTERSE) {
        cout << "TDeviceBuilderOBSingleDAQ::InitSetup() - end" << endl;
    }
    fCurrentDevice->FreezeSetup();
    CountEnabledChipsPerBoard();
    PropagateVerbosityToBoards();
}


