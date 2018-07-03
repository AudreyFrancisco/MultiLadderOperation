#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilderHalfStave.h"
#include "TAlpide.h"
#include "TChipConfig.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardMOSAIC.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderHalfStave::TDeviceBuilderHalfStave() : TDeviceBuilderWithSlaveChips()
{ }

//___________________________________________________________________
TDeviceBuilderHalfStave::~TDeviceBuilderHalfStave()
{ }

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilderHalfStave::SetNModules( const int number )
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    fCurrentDevice->SetNModules( number );
}

//___________________________________________________________________
void TDeviceBuilderHalfStave::SetVerboseLevel( const int level )
{
    if ( level > kTERSE ) {
        cout << "TDeviceBuilderHalfStave::SetVerboseLevel() - " << level << endl;
    }
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceBuilderHalfStave::CreateDeviceConfig()
{
    if ( fCurrentDevice->IsConfigFrozen() ) {
        return;
    }
    if ( !fCurrentDevice->GetNModules() ) {
        throw runtime_error( "TDeviceBuilderHalfStave::CreateDeviceConfig() - no module!" );
    }
    fCurrentDevice->SetBoardType( TBoardType::kBOARD_MOSAIC );
    const int nBoards = 2;
    for (int iboard = 0; iboard < nBoards; iboard++) {
        auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
        fCurrentDevice->AddBoardConfig( newBoardConfig );
    }
    for (unsigned int imod = 0; imod < fCurrentDevice->GetNModules(); imod++) {
        int moduleId = imod + 1;
        for (int i = 0; i < 15; i++) {
            if (i == 7) continue;
            int chipId = i + ((moduleId & 0x7) << 4);
            auto newChipConfig = make_shared<TChipConfig>( chipId );
            fCurrentDevice->AddChipConfig( newChipConfig );
        }
    }
    fCurrentDevice->SetNChips( fCurrentDevice->GetChipConfigsVectorSize() );
    
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderHalfStave::CreateDeviceConfig() - done" << endl;
    }
    fCurrentDevice->FreezeConfig();
}

// implicit assumptions on the setup in this method
// - chips of master 0 of all modules are connected to 1st mosaic, chips of master 8 to 2nd MOSAIC
//___________________________________________________________________
void TDeviceBuilderHalfStave::InitSetup()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    try {
        if ( fCurrentDevice->GetDeviceType() != TDeviceType::kHALFSTAVE ) {
            throw runtime_error( "TDeviceBuilderHalfStave::InitSetup() - wrong device type." );
        }
    } catch (std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    try {
        if ( fCurrentDevice->GetBoardType() != TBoardType::kBOARD_MOSAIC ) {
            throw runtime_error( "TDeviceBuilderHalfStave::InitSetup() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderHalfStave::InitSetup() - start" << endl;
    }
    for (unsigned int i = 0; i < fCurrentDevice->GetNBoards(true); i++) {
        shared_ptr<TBoardConfigMOSAIC> boardConfig = ((dynamic_pointer_cast<TBoardConfigMOSAIC>)(fCurrentDevice->GetBoardConfig(i)));
        boardConfig->SetInvertedData(false);  //already inverted in the adapter plug ?
        boardConfig->SetSpeedMode(MosaicReceiverSpeed::RCV_RATE_400);
        auto newBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
        fCurrentDevice->AddBoard( newBoard );
    }
    
    for (unsigned int i = 0; i < fCurrentDevice->GetNChips(); i++) {
        shared_ptr<TChipConfig> chipConfig = fCurrentDevice->GetChipConfig(i);
        int          chipId     = chipConfig->GetChipId();

        // to be checked when final layout of adapter fixed
        int ci  = 0;
        int rcv = (chipId & 0x7) ? -1 : 9*ci; //FIXME
        chipConfig->SetReceiver( rcv );
        chipConfig->SetControlInterface( ci );
        
        int          mosaic     = (chipId & 0x1000) ? 1:0;
        auto alpide = make_shared<TAlpide>( chipConfig );
        fCurrentDevice->AddChip( alpide );
        (fCurrentDevice->GetChip(i))->SetReadoutBoard(fCurrentDevice->GetBoard(mosaic));
        (fCurrentDevice->GetBoard(mosaic))->AddChipConfig(chipConfig);
    }
    for (unsigned int i = 0; i < fCurrentDevice->GetNChips(); i++) {
        EnableSlave( i );
    }
    fCurrentDevice->EnableClockOutputs( true );
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    sleep(5);
    MakeDaisyChain();
    if ( fVerboseLevel > kTERSE ) {
        cout << "TDeviceBuilderHalfStave::InitSetup() - end" << endl;
    }
    fCurrentDevice->FreezeSetup();
    CountEnabledChipsPerBoard();
    PropagateVerbosityToBoards();
}
