#ifndef DEVICE_HIT_SCAN_H
#define DEVICE_HIT_SCAN_H

/**
 * \class TDeviceHitScan
 *
 * \brief This class is the abstract base class for the noise, digital and the thresdold scan.
 *
 * \author Andry Rakotozafindrabe
 *
 */

#include <memory>
#include <vector>
#include "TDeviceChipVisitor.h"
#include "Common.h"

class TScanConfig;
class TScanHisto;
class TErrorCounter;
class TAlpideDecoder;
class TBoardDecoder;
class TDevice;

class TDeviceHitScan : public TDeviceChipVisitor {
    
protected:
    
    /// max number of trials ending with timeout for each injection for each chip
    static const unsigned int MAXTRIALS = 3;

    /// max number of bad chip events per chip for each injection
    static const unsigned int MAXNBAD = 10;
                
    /// scan configuration
    std::shared_ptr<TScanConfig> fScanConfig;
    
    /// map to histograms (one per chip) of hit pixels
    std::shared_ptr<TScanHisto> fScanHisto;

    /// dedicated error counter
    std::shared_ptr<TErrorCounter> fErrorCounter;
    
    /// chip decoder
    std::unique_ptr<TAlpideDecoder> fChipDecoder;
    
    /// board decoder
    std::unique_ptr<TBoardDecoder> fBoardDecoder;
    
    /// number of triggers
    int fNTriggers;
        
public:
    
    /// constructor
    TDeviceHitScan();
    
    /// constructor with a TDevice and a ScanConfig specified
    TDeviceHitScan( std::shared_ptr<TDevice> aDevice,
                     std::shared_ptr<TScanConfig> aScanConfig );
    
    /// destructor
    virtual ~TDeviceHitScan();
    
    /// set the scan configuration
    void SetScanConfig( std::shared_ptr<TScanConfig> aScanConfig );

    /// set the verbosity level
    virtual void SetVerboseLevel( const int level );

    /// toggle on/off the possibility to rescue a bad chip id
    void SetRescueBadChipId( const bool permit );
    
    /// initialization
    virtual void Init();
    
    /// perform the mask scan of the device
    virtual void Go() = 0;
    
    /// write raw hit data to a text file
    virtual void WriteDataToFile( const char *fName, bool Recreate = true ) = 0;
    
    /// draw and save hit map or distributions
    virtual void DrawAndSaveToFile( const char *fName ) = 0;
    
protected:
    
    /// allocate memory for histogram of hit pixels for each enabled chip
    virtual void AddHisto() = 0;
    
    /// retrieve scan parameters from the configuration file
    virtual void InitScanParameters();
    
    /// configure readout boards
    virtual void ConfigureBoards() = 0;
    
    /// configure chips
    void ConfigureChips();
    
    /// read data from a given readout board, for a given number of triggers (all if 0 is asked)
    unsigned int ReadEventData( const unsigned int iboard, int nTriggers = 0 );
    
    /// start the readout
    void StartReadout();
    
    /// stop the readout
    void StopReadout();

    /// global reset + read out reset + bunch crossing counter reset
    void DoBroadcastReset();
    
    /// check if there is any hit for the requested chip index
    virtual bool HasData( const common::TChipIndex idx ) = 0;

};

#endif
