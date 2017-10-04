#ifndef DEVICE_THRESHOLD_SCAN_H
#define DEVICE_THRESHOLD_SCAN_H

/**
 * \class TDeviceThresholdScan
 *
 * \brief This class runs the regular threshold scan over all enabled chips in the device.
 *
 * \author Andry Rakotozafindrabe
 *
 * \note
 * The code was inspired from the original main_threshold.cpp written by ITS team.
 */

#include "TDeviceMaskScan.h"
#include <deque>

class TScanConfig;
class TScanHisto;
class TDevice;

class TDeviceThresholdScan : public TDeviceMaskScan {
    
    /// value of charge (VPULSEH - fChargeStart) to be injected, at the start of the scan
    int fChargeStart;

    /// increase of the value of fChargeStart at each step of the scan
    int fChargeStep;

    /// value of charge (VPULSEH - fChargeStop) to be injected, at the end of the scan
    int fChargeStop;
    
    /// number of steps on the injected charge during the scan
    unsigned int fNChargeSteps;
    
    /// pointer to current map of histograms (one per chip) of hit pixels
    std::shared_ptr<TScanHisto> fScanHisto;
    
    /// list of histogram maps (one per step of the injected charge)
    std::deque<std::shared_ptr<TScanHisto>> fHistoQue;
    
public:
    
    /// constructor
    TDeviceThresholdScan();
    
    /// constructor with a TDevice and a ScanConfig specified
    TDeviceThresholdScan( std::shared_ptr<TDevice> aDevice,
                       std::shared_ptr<TScanConfig> aScanConfig );
    
    /// destructor
    virtual ~TDeviceThresholdScan();
    
    /// initialization
    void Init();
    
    /// terminate
    void Terminate();
    
    /// perform the digital scan of the device
    void Go();
    
    /// write raw hit data to a text file
    void WriteDataToFile( const char *fName, bool Recreate = true );
    
protected:
    
    /// allocate memory for histogram of hit pixels for each enabled chip
    void AddHisto();
    
    /// dump scan parameters
    void DumpScanParameters();

    /// retrieve scan parameters from the configuration file
    void InitScanParameters();
    
    /// check if there is any hit for the requested chip index
    bool HasData( const common::TChipIndex idx );
    
};

#endif
