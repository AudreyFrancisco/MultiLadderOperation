#ifndef DEVICE_DIGITAL_SCAN_H
#define DEVICE_DIGITAL_SCAN_H

/**
 * \class TDeviceDigitalScan
 *
 * \brief This class runs the digital scan over all enabled chips in the device.
 *
 * \author Andry Rakotozafindrabe
 *
 * \note
 * The code was inspired from the original main_digitalscan.cpp written by ITS team.
 */

#include "TDeviceMaskScan.h"

class TScanConfig;
class TScanHisto;
class TDevice;

class TDeviceDigitalScan : public TDeviceMaskScan {
    
public:

    /// constructor
    TDeviceDigitalScan();
    
    /// constructor with a TDevice and a ScanConfig specified
    TDeviceDigitalScan( std::shared_ptr<TDevice> aDevice,
                       std::shared_ptr<TScanConfig> aScanConfig );
    
    /// destructor
    virtual ~TDeviceDigitalScan();
        
    /// terminate
    void Terminate();
    
    /// write raw hit data to a text file
    void WriteDataToFile( bool Recreate = true );

    /// write list of hit pixels with a bad flag in a text file
    void WriteCorruptedHitsToFile( bool Recreate = true );
    
    /// draw and save hit map of bad pixels and their firing frequency distribution
    void DrawAndSaveToFile();

    /// perform the digital scan of the device
    void Go();
    
protected:
    
    /// allocate memory for histogram of hit pixels for each enabled chip
    void AddHisto();

    /// look for any inefficient, dead or hot pixels and give them to the error counter
    void CollectDiscordantPixels();
    
    /// check if there is any hit for the requested chip index
    bool HasData( const common::TChipIndex idx );
    
};

#endif
