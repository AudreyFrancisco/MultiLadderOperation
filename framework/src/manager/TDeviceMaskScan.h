#ifndef DEVICE_MASK_SCAN_H
#define DEVICE_MASK_SCAN_H

/**
 * \class TDeviceMaskScan
 *
 * \brief This class is the abstract base class for the digital scan and the thresdold scan.
 *
 * \author Andry Rakotozafindrabe
 *
 */

#include <memory>
#include <vector>
#include "TDeviceHitScan.h"
#include "Common.h"

class TScanHisto;
class TScanConfig;
class TErrorCounter;
class TAlpideDecoder;
class TBoardDecoder;
class TDevice;

class TDeviceMaskScan : public TDeviceHitScan {
    
protected:

    /// number of mask stages
    int fNMaskStages;
    
    /// number of pixels per region
    int fNPixPerRegion;
    
public:
    
    /// constructor
    TDeviceMaskScan();
    
    /// constructor with a TDevice and a ScanConfig specified
    TDeviceMaskScan( std::shared_ptr<TDevice> aDevice,
                     std::shared_ptr<TScanConfig> aScanConfig );
    
    /// destructor
    virtual ~TDeviceMaskScan();

    /// initialization
    virtual void Init();
                
protected:
    
    /// retrieve scan parameters from the configuration file
    virtual void InitScanParameters();
    
    /// configure readout boards
    void ConfigureBoards();
    
};

#endif
