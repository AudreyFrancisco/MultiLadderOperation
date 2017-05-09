#ifndef DEVICE_CHIP_OPERATOR_H
#define DEVICE_CHIP_OPERATOR_H

/// \class TDeviceChipOperator
/// \brief This class forward configure operations to each chip in the device.
///
/// The methods iterate over all chips in the device to perform the requested configure
/// operation. Some operations have no effect or a different effect on chips with slave
/// compared to chips without slave. Disabled chips are skipped.

#include<memory>
#include "TVerbosity.h"

class TDevice;
enum class AlpidePulseType;

class TDeviceChipOperator : public TVerbosity {

private:

    std::shared_ptr<TDevice> fDevice;
    
public:
    
    #pragma mark - Constructors/destructor
    TDeviceChipOperator();
    TDeviceChipOperator( std::shared_ptr<TDevice> aDevice );
    virtual ~TDeviceChipOperator();
    
    #pragma mark - setters
    void SetDevice( std::shared_ptr<TDevice> aDevice );
    
    #pragma mark - forward configure operations to each Alpide in the device
    void DoConfigureFromu( const AlpidePulseType pulseType, const bool testStrobe );
    void DoConfigureFromu();
    void DoConfigureBuffers();
    void DoConfigureCMU();
    void DoConfigureMaskStage( int nPix, const int iStage );
    void DoBaseConfigPLL();
    void DoBaseConfigMask();
    void DoBaseConfigFromu();
    void DoBaseConfigDACs();
    void DoBaseConfig();
    
};


#endif
