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
    fDevice( nullptr )
{
    
}

//___________________________________________________________________
TDeviceChipVisitor::TDeviceChipVisitor( shared_ptr<TDevice> aDevice ) : TVerbosity(),
    fDevice( nullptr )
{
    try {
        SetDevice( aDevice );
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
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
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->SetVerboseLevel( level );
    }
}

#pragma mark - initialization

//___________________________________________________________________
void TDeviceChipVisitor::Init()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::Init() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::Init() - no chip found !" );
    }
    shared_ptr<TReadoutBoard> myBoard = fDevice->GetBoard(0);
    if ( !myBoard ) {
        throw runtime_error( "TDeviceChipVisitor::Init() - no readout board found!" );
    }
    
    // -- global reset chips

    myBoard->SendOpCode( (uint16_t)AlpideOpCode::GRST );
    
    // TODO: check if this is needed ?
    // -- pixel matrix reset
    // (does not affect the PULSE_EN and MASK_EN latches)

    myBoard->SendOpCode( (uint16_t)AlpideOpCode::PRST );
    
    // -- init in-pixel registers
    // (the pixel latches do not provide a reset mechanism,
    // the value after power-on is unknown)

    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->Init();
        // disable mask
        fDevice->GetChip(i)->WritePixRegAll( AlpidePixConfigReg::MASK_ENABLE, false );
        // disable pulse
        fDevice->GetChip(i)->WritePixRegAll( AlpidePixConfigReg::PULSE_ENABLE, false );
    }
}

#pragma mark - forward configure operations to each Alpide in the device

//___________________________________________________________________
void TDeviceChipVisitor::DoApplyStandardDACSettings( const float backBias )
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoApplyStandardDACSettings() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoApplyStandardDACSettings() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ApplyStandardDACSettings( backBias );
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoBaseConfigPLL()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigPLL() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigPLL() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->BaseConfigPLL();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoBaseConfigMask()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigMask() - can not use a null pointer !" );
    }
   if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigMask() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->BaseConfigMask();
    }
}


//___________________________________________________________________
void TDeviceChipVisitor::DoBaseConfigDACs()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigDACs() - can not use a null pointer !" );
    }
   if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfigDACs() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->BaseConfigDACs();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoBaseConfig()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfig() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoBaseConfig() - no chip found !" );
    }
    shared_ptr<TReadoutBoard> myBoard = fDevice->GetBoard(0);
    if ( !myBoard ) {
        throw runtime_error( "TDeviceFifoTest::DoConfigureCMU() - no readout board found!" );
    }

    // configure chip(s)
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
            fDevice->GetChip(i)->BaseConfig();
    }

    // readout reset
    myBoard->SendOpCode( (uint16_t)AlpideOpCode::RORST );
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureFROMU()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureFROMU() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureFROMU() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureFROMU();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureBuffers()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureBuffers() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureBuffers() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureBuffers();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureCMU()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureCMU() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureCMU() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureCMU();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureDTU_TEST1()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureDTU_TEST1() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureDTU_TEST1() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureDTU_TEST1();
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoConfigureMaskStage( int nPix, const int iStage )
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureMaskStage() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureMaskStage() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->ConfigureMaskStage( nPix, iStage );
    }
}

//___________________________________________________________________
void TDeviceChipVisitor::DoDumpConfig()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureMaskStage() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipVisitor::DoConfigureMaskStage() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->DumpConfig();
    }
}

