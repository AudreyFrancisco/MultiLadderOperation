#ifndef DEVICE_CHIP_VISITOR_H
#define DEVICE_CHIP_VISITOR_H

/**
 * \class TDeviceChipVisitor
 *
 * \brief This class forward configure operations to each chip in the device.
 *
 * \author Andry Rakotozafindrabe
 *
 * The methods iterate over all chips in the device to perform the requested configure
 * operation. The configure operation itself is delegated to the TAlpide methods.
 * As can be seen in these TAlpide methods: some operations have no effect on
 * OB slave chips, and disabled chips are systematically skipped.
 */

#include <memory>
#include "TVerbosity.h"

class TDevice;
enum class AlpidePulseType;

class TDeviceChipVisitor : public TVerbosity {

protected:

    std::shared_ptr<TDevice> fDevice;
    bool fIsInitDone;
    
public:
    
    #pragma mark - Constructors/destructor
    TDeviceChipVisitor();
    TDeviceChipVisitor( std::shared_ptr<TDevice> aDevice );
    virtual ~TDeviceChipVisitor();
    
    #pragma mark - setters
    void SetDevice( std::shared_ptr<TDevice> aDevice );

    /// Propagate verbosity down to each Alpide in the device
    void SetVerboseLevel( const int level );
    
    #pragma mark - initialization
    virtual void Init();
    
    #pragma mark - forward operations to each Alpide in the device
    void DoApplyStandardDACSettings( const float backBias );
    void DoBaseConfigPLL();
    void DoBaseConfigMask();
    void DoBaseConfigDACs();
    void DoBaseConfig();
    void DoConfigureFROMU();
    void DoConfigureBuffers();
    void DoConfigureCMU();
    void DoConfigureDTU_TEST1();
    void DoConfigureMaskStage( int nPix, const int iStage );
    void DoDumpConfig();
    
};


#endif
