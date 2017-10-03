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
#include "TDeviceChipVisitor.h"

class TScanHisto;
class TScanConfig;
class TErrorCounter;
class TAlpideDecoder;
class TBoardDecoder;
class TDevice;

class TDeviceMaskScan : public TDeviceChipVisitor {
    
protected:
    
    /// max number of trials ending with timeout for each injection for each chip
    static const unsigned int MAXTRIALS = 3;
    
    /// max number of bad chip events per chip for each injection
    static const unsigned int MAXNBAD = 10;
    
    /// map to histograms (one per chip) of hit pixels
    std::shared_ptr<TScanHisto> fScanHisto;
    
    /// scan configuration
    std::shared_ptr<TScanConfig> fScanConfig;
    
    /// dedicated error counter
    std::shared_ptr<TErrorCounter> fErrorCounter;
    
    /// chip decoder
    std::unique_ptr<TAlpideDecoder> fChipDecoder;
    
    /// board decoder
    std::unique_ptr<TBoardDecoder> fBoardDecoder;
    
    /// number of triggers
    int fNTriggers;
    
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
    
    /// set the scan configuration
    void SetScanConfig( std::shared_ptr<TScanConfig> aScanConfig );
    
    /// propagate the verbosity level to data members
    void SetVerboseLevel( const int level );
    
    /// initialization
    void Init();
    
    /// perform the mask scan of the device
    virtual void Go() = 0;
    
protected:
    
    /// allocate memory for histogram of hit pixels for each enabled chip
    void AddHisto();
    
    /// retrieve scan parameters from the configuration file
    virtual void InitScanParameters();
    
    /// configure readout boards
    void ConfigureBoards();
    
    /// configure chips
    void ConfigureChips();
    
    /// read data from a given readout board
    unsigned int ReadEventData( const unsigned int iboard );
    
    /// start the readout
    void StartReadout();
    
    /// stop the readout
    void StopReadout();

};

#endif
