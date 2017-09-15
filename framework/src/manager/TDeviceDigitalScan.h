#ifndef DEVICE_DIGITAL_SCAN_H
#define DEVICE_DIGITAL_SCAN_H

/**
 * \class TDeviceDigitalScan
 *
 * \brief This class runs the digital scan over all enabled chips in the device.
 *
 * \author Andry Rakotozafindrabe
 *
 * \remark
 * Missing: list of dead pixels.
 *
 * \note
 * Most of the code was moved from the original main_digitalscan.cpp written by ITS team.
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

class TDeviceDigitalScan : public TDeviceChipVisitor {
    
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
    std::unique_ptr<TErrorCounter> fErrorCounter;
    
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
    TDeviceDigitalScan();
    
    /// constructor with a TDevice and a ScanConfig specified
    TDeviceDigitalScan( std::shared_ptr<TDevice> aDevice,
                       std::shared_ptr<TScanConfig> aScanConfig );
    
    /// destructor
    virtual ~TDeviceDigitalScan();
    
    /// set the scan configuration
    void SetScanConfig( std::shared_ptr<TScanConfig> aScanConfig );
    
    /// propagate the verbosity level to data members
    virtual void SetVerboseLevel( const int level );
    
    /// initialization
    virtual void Init();
    
    /// terminate
    virtual void Terminate();
    
    /// write hit data to a text file
    void WriteDataToFile( const char *fName, bool Recreate = true );

    /// perform the digital scan of the device
    void Go();
    
protected:
    
    /// allocate memory for histogram of hit pixels for each enabled chip
    void AddHisto();
    
    /// retrieve scan parameters from the configuration file
    void InitScanParameters();
    
    /// configure readout boards
    virtual void ConfigureBoards();
    
    /// configure chips
    virtual void ConfigureChips();
    
    /// read data from a given readout board
    unsigned int ReadEventData( const unsigned int iboard );
    
};

#endif
