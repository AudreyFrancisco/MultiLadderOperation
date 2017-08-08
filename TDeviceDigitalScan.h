#ifndef DEVICE_DIGITAL_SCAN_H
#define DEVICE_DIGITAL_SCAN_H

/**
 * \class TDeviceFifoTest
 *
 * \brief This class runs the digital scan all enabled chips in the device.
 *
 * \author Andry Rakotozafindrabe
 *
 * \remark
 *
 *
 * \note
 * Most of the code was moved from the original main_digitalscan.cpp
 */

#include <memory>
#include <vector>
#include <array>
#include "TDeviceChipVisitor.h"

class TPixHit;
class TScanConfig;
class TDevice;
class AlpideDecoder;

template <class T, size_t CHIP, size_t ROW, size_t COL>
using HitArray = std::array<std::array<std::array<T, COL>, ROW>, CHIP>;

class TDeviceDigitalScan : public TDeviceChipVisitor {
    
    /// maximum number of chips in a device
    static const int MAX_NCHIPS = 16;
    
    /// number of priority encoders (double columns) in the pixel matrix
    static const int NPRIORITY_ENCODERS = 512;
    
    /// number of addresses in the pixel matrix
    static const int NADDRESSES = 1024;
    
    /// max number of trials ending with timeout for each injection for each chip
    static const int MAXTRIALS = 3;

    /// max number of bad chip events per chip for each injection
    static const int MAXNBAD = 10;

    /// array of hits pixels
    HitArray<int, MAX_NCHIPS, NPRIORITY_ENCODERS, NADDRESSES> fHitData;
    
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
    
    /// write hit data to a text file
    void WriteDataToFile( const char *fName, bool Recreate = true );

    /// perform the digital scan of the device
    void Go();
    
protected:
    
    friend class AlpideDecoder;

private:
    
    /// zeroes all elements of the hit data array
    void ClearHitData();
    
    /// copy the hit data from the vector of TPixHit to the array of hits
    void CopyHitData();
    
    /// check if there is any hit for the requested chip id
    bool HasData( const int chipId );
    
};

#endif
