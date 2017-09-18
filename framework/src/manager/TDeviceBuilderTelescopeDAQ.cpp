#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilderTelescopeDAQ.h"
#include "TBoardConfig.h"
#include "TBoardConfigDAQ.h"
#include "TChipConfig.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderTelescope::TDeviceBuilderTelescope() : TDeviceBuilderWithDAQBoards()
{ }

//___________________________________________________________________
TDeviceBuilderTelescope::~TDeviceBuilderTelescope()
{ }

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilderTelescope::SetDeviceType( const TDeviceType dt )
{
    if ( fCurrentDevice->IsConfigFrozen() || fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    fCurrentDevice->SetDeviceType( dt );
    fCurrentDevice->SetStartChipId( 16 ); // master OB chip
    // fNChips taken from the config file
}

//___________________________________________________________________
void TDeviceBuilderTelescope::SetNChips( const int number )
{
    try {
        fCurrentDevice->SetNChips( number );
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
}

//___________________________________________________________________
void TDeviceBuilderTelescope::SetVerboseLevel( const int level )
{
    if ( level > kTERSE ) {
        cout << "TDeviceBuilderTelescope::SetVerboseLevel() - " << level << endl;
    }
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceBuilderTelescope::CreateDeviceConfig()
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    if ( fCurrentDevice->GetNChips() < 1 ) {
        throw runtime_error( "TDeviceBuilderTelescope::CreateDeviceConfig() - TELESCOPE not useable with less than 1 chips." );
    }
    fCurrentDevice->SetBoardType( TBoardType::kBOARD_DAQ );
    int nBoards = fCurrentDevice->GetNChips(); // one DAQ board per chip
    for (int iboard = 0; iboard < nBoards; iboard ++) {
        auto newBoardConfig = make_shared<TBoardConfigDAQ>();
        fCurrentDevice->AddBoardConfig( newBoardConfig );
    }
    for (int i = 0; i < nBoards; i++) {
        auto newChipConfig = make_shared<TChipConfig>( fCurrentDevice->GetStartChipId() );
        newChipConfig->SetEnableSlave( false ); // with no slave
        fCurrentDevice->AddChipConfig( newChipConfig );
    }
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );

    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderTelescope::CreateDeviceConfig() - done" << endl;
    }
    fCurrentDevice->FreezeConfig();
}

//___________________________________________________________________
void TDeviceBuilderTelescope::InitSetup()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    try {
        if ( fCurrentDevice->GetDeviceType() != TDeviceType::kTELESCOPE ) {
            throw runtime_error( "TDeviceBuilderTelescope::InitSetup() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_DAQ ) {
            throw runtime_error( "TDeviceBuilderTelescope::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderTelescope::InitSetup() - start" << endl;
    }
    
    // FIXME: not implemeted yet
    cout << "TDeviceBuilderTelescope::InitSetup() - NOT IMPLEMENTED YET" << endl;
    
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderTelescope::InitSetup() - end" << endl;
    }
    fCurrentDevice->FreezeSetup();
    CountEnabledChipsPerBoard();
    PropagateVerbosityToBoards();
}

