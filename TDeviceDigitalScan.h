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
 * The TAlpideDecoder does not seem to work properly for the moment (to be fixed in
 * subsequent versions). Only empty frames are detected, which is not the case with
 * the low level software from Giuseppe.
 *
 * \note
 * Most of the code was moved from the original main_digitalscan.cpp written by ITS team.
 */

#include <memory>
#include <vector>
#include "TDeviceChipVisitor.h"

class TPixHit;
class TScanConfig;
class TDevice;
class AlpideDecoder;

class TDeviceDigitalScan : public TDeviceChipVisitor {
    
    /// number of priority encoders (double columns) in the pixel matrix
    static const unsigned int NPRIORITY_ENCODERS = 512;
    
    /// number of addresses in the pixel matrix
    static const unsigned int NADDRESSES = 1024;
    
    /// max number of trials ending with timeout for each injection for each chip
    static const unsigned int MAXTRIALS = 3;

    /// max number of bad chip events per chip for each injection
    static const unsigned int MAXNBAD = 10;

    /// array of hit pixels [ichip][icol][iaddr]
    int* fHitData;
    
    /// scan configuration
    std::shared_ptr<TScanConfig> fScanConfig;
    
    /// vector of hit pixels
    std::vector<std::shared_ptr<TPixHit>> fHits;
    
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
    
    /// initialization
    virtual void Init();
    
    /// write hit data to a text file
    void WriteDataToFile( const char *fName, bool Recreate = true );

    /// perform the digital scan of the device
    void Go();
    
protected:
    
    friend class AlpideDecoder;

private:
    
    /// zeroes all elements of the hit data array
    void ClearHitData();
    
    /// move the hit data from the vector of TPixHit to the array of hits
    void MoveHitData();
    
    /// check if there is any hit for the requested chip index
    bool HasData( const unsigned int ichip );
    
    /// return the index in the array of hit pixels for a given (ichip, icol, iadd)
    int GetHitDataIndex( const unsigned int ichip, const unsigned int icol, const unsigned int iadd );
    
};

#endif
