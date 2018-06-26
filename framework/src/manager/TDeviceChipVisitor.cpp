#include "TDeviceChipVisitor.h"
#include "AlpideDictionary.h"
#include "TAlpide.h"
#include "TDevice.h"
#include "TChipConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include <stdexcept>
#include <iostream>


using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceChipVisitor::TDeviceChipVisitor() : TVerbosity(),
    fDevice( nullptr ),
    fIsInitDone( false ),
    fIsTerminated( false )
{
    
}

//___________________________________________________________________
TDeviceChipVisitor::TDeviceChipVisitor( shared_ptr<TDevice> aDevice ) : TVerbosity(),
    fDevice( nullptr ),
    fIsInitDone( false ),
    fIsTerminated( false )
{
    try {
        SetDevice( aDevice );
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
}

//___________________________________________________________________
TDeviceChipVisitor::~TDeviceChipVisitor()
{
    if ( fDevice ) fDevice.reset();
 }

#pragma mark - setters

//___________________________________________________________________
void TDeviceChipVisitor::SetDevice( shared_ptr<TDevice> aDevice )
{
    if ( !aDevice ) {
        throw runtime_error( "TDeviceChipVisitor::SetDevice() - can not use a null pointer !" );
    }
    fDevice = aDevice;
}

//___________________________________________________________________
void TDeviceChipVisitor::SetVerboseLevel( const int level )
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::SetVerboseLevel() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::SetVerboseLevel() - no chip found !" );
    }
    if ( level > kTERSE ) {
        cout << "TDeviceChipVisitor::SetVerboseLevel() - " << level << endl;
    }
    TVerbosity::SetVerboseLevel( level );
    for ( unsigned int i = 0; i < fDevice->GetNChips(); i++ ) {
        fDevice->GetChip(i)->SetVerboseLevel( level );
    }
    for ( unsigned int i = 0; i < fDevice->GetNBoards(false); i++ ) {
        fDevice->GetBoard(i)->SetVerboseLevel( level );
    }
}

#pragma mark - init / terminate

//___________________________________________________________________
void TDeviceChipVisitor::Init()
{
    try {
        ConfigureBoards();
        DoBroadcastReset();
    } catch ( std::exception &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    fIsInitDone = true;

    ConfigureChips();
    if ( GetVerboseLevel() > kTERSE ) {
        DoDumpConfig();
    }
    
    StartReadout();
}

//___________________________________________________________________
void TDeviceChipVisitor::Terminate()
{
    StopReadout();
    fDevice->EnableClockOutputs( false );
    fIsTerminated = true;
}

#pragma mark - forward configure operations to each Alpide in the device

//___________________________________________________________________
void TDeviceChipVisitor::DoActivateConfigMode()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoActivateConfigMode() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ActivateConfigMode();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoApplyStandardDACSettings( const float backBias )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoApplyStandardDACSettings() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ApplyStandardDACSettings( backBias );
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoBaseConfigPLL()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigPLL() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->BaseConfigPLL();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoBaseConfigMask()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigMask() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->BaseConfigMask();
    }
}


//___________________________________________________________________
void TDeviceChipVisitor::DoBaseConfigDACs()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigDACs() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->BaseConfigDACs();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoBaseConfig()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfig() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
            fDevice->GetChip(i)->BaseConfig();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureFROMU()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureFROMU() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureFROMU();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureBuffers()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureBuffers() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureBuffers();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureCMU()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureCMU() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureCMU();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureDTU_TEST1()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureDTU_TEST1() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureDTU_TEST1();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureMaskStage( int nPix, const int iStage )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureMaskStage() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureMaskStage( nPix, iStage );
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoDumpConfig()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoDumpConfig() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->DumpConfig();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoActivateReadoutMode()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoActivateReadoutMode() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ActivateReadoutMode();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureVPulseLow( const unsigned int deltaV )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureVPulseLow() - not initialized ! Please use Init() first." );
    }
    for (unsigned int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureVPulseLow( deltaV );
    }
}

#pragma mark - readout board and chip configuration

//___________________________________________________________________
void TDeviceChipVisitor::DoBroadcastReset()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoBroadcastReset() - can not use a null pointer for the device !" );
    }
    if ( fIsInitDone ) {
        cerr << "TDeviceChipVisitor::DoBroadcastReset() - already done ! Doing nothing." << endl;
        return;
    }
    if ( fDevice->GetNBoards(false) == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoBroadcastReset() - no readout board found !" );
    }

    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        shared_ptr<TReadoutBoard> myBoard = fDevice->GetBoard( iboard );
        if ( !myBoard ) {
            throw runtime_error( "TDeviceChipVisitor::DoBroadcastReset() - no readout board found!" );
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
