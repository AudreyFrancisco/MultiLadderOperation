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
#include <map>

class TScanConfig;
class TScanHisto;
class TDevice;
class TSCurveAnalysis;

class TDeviceThresholdScan : public TDeviceMaskScan {
    
    /// value of charge (VPULSEH - fChargeStart) to be injected, at the start of the scan
    unsigned int fChargeStart;

    /// increase of the value of fChargeStart at each step of the scan
    int fChargeStep;

    /// value of charge (VPULSEH - fChargeStop) to be injected, at the end of the scan
    unsigned int fChargeStop;
    
    /// number of steps on the injected charge during the scan
    unsigned int fNChargeSteps;
        
    /// list of histogram maps (one per step of the injected charge)
    std::deque<std::shared_ptr<TScanHisto>> fHistoQue;
    
    /// S-curve analyzer (one per chip index)
    std::map<int, std::shared_ptr<TSCurveAnalysis>> fAnalyserCollection;
    
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
    
    /// propagate the verbosity level to data members
    void SetVerboseLevel( const int level );
    
    /// get hits for a given chip, double-column, address and a given amplification step
    unsigned int GetHits( const common::TChipIndex aChipIndex,
                         const unsigned int icol,
                         const unsigned int iaddr,
                         const unsigned int iampl );
    
    /// return the value (in DAC units) of the injected charge at the i-th step of the scan
    unsigned int GetInjectedCharge( const unsigned int iampl ) const;
    
    /// terminate
    void Terminate();
    
    /// perform the digital scan of the device
    void Go();
    
    /// write raw hit data to a text file
    void WriteDataToFile( bool Recreate = true );
    
    /// draw and save threshold, noise and chi2/ndf distributions
    void DrawAndSaveToFile();

protected:
    
    /// allocate memory for histogram of hit pixels for each enabled chip
    void AddHisto();
    
    /// dump scan parameters
    void DumpScanParameters();

    /// retrieve scan parameters from the configuration file
    void InitScanParameters();
    
    /// check if there is any hit for the requested chip index
    bool HasData( const common::TChipIndex idx );
    
    /// add an S-curve analyzer for a given chip
    void AddChipSCurveAnalyzer( const common::TChipIndex idx );
    
    /// analyze the data obtained from the threshold scan of the device
    void AnalyzeData();

    /// Fill the S-curve for a given pixel then extract threshold and noise
    void AnalyzePixelSCurve( const common::TChipIndex aChipIndex,
                            const unsigned int icol,
                            const unsigned int iaddr );
    
};

#endif
