#include "TDeviceChipOperator.h"
#include "AlpideDictionary.h"
#include "TAlpide.h"
#include "TDevice.h"
#include "TChipConfig.h"
#include <stdexcept>
#include <iostream>


using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceChipOperator::TDeviceChipOperator() : TVerbosity(),
    fDevice( nullptr )
{
    
}

//___________________________________________________________________
TDeviceChipOperator::TDeviceChipOperator( shared_ptr<TDevice> aDevice ) : TVerbosity(),
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
TDeviceChipOperator::~TDeviceChipOperator()
{
    if ( fDevice ) fDevice.reset();
}

#pragma mark - setters

//___________________________________________________________________
void TDeviceChipOperator::SetDevice( shared_ptr<TDevice> aDevice )
{
    if ( !aDevice ) {
        throw runtime_error( "TDeviceChipOperator::SetDevice() - can not use a null pointer !" );
    }
    fDevice = aDevice;
}

#pragma mark - forward configure operations to each Alpide in the device

//___________________________________________________________________
void TDeviceChipOperator::DoConfigureFromu( const AlpidePulseType pulseType,
                                          const bool testStrobe )
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoConfigureFromu() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        if ( fDevice->GetChipConfig(i)->HasEnabledSlave() ) continue;
        fDevice->GetChip(i)->ConfigureFromu(pulseType, testStrobe);
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoConfigureFromu()
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoConfigureFromu() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        if ( fDevice->GetChipConfig(i)->HasEnabledSlave() ) continue;
        fDevice->GetChip(i)->ConfigureFromu();
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoConfigureBuffers()
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoConfigureBuffers() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        fDevice->GetChip(i)->ConfigureBuffers();
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoConfigureCMU()
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoConfigureCMU() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        if ( fDevice->GetChipConfig(i)->HasEnabledSlave() ) continue;
        fDevice->GetChip(i)->ConfigureCMU();
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoConfigureMaskStage( int nPix, const int iStage )
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoConfigureMaskStage() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        fDevice->GetChip(i)->ConfigureMaskStage( nPix, iStage );
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoBaseConfigPLL()
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoBaseConfigPLL() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        fDevice->GetChip(i)->BaseConfigPLL();
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoBaseConfigMask()
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoBaseConfigMask() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        fDevice->GetChip(i)->BaseConfigMask();
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoBaseConfigFromu()
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoBaseConfigFromu() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        fDevice->GetChip(i)->BaseConfigFromu();
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoBaseConfigDACs()
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoBaseConfigDACs() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        fDevice->GetChip(i)->BaseConfigDACs();
    }
}

//___________________________________________________________________
void TDeviceChipOperator::DoBaseConfig()
{
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceChipOperator::DoBaseConfig() - no chip found !" );
    }
    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        if ( !(fDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        if ( fDevice->GetChipConfig(i)->HasEnabledSlave() ) {
            fDevice->GetChip(i)->BaseConfigPLL();
        } else {
            fDevice->GetChip(i)->BaseConfig();
        }
    }
}

