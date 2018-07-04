#ifndef MULTI_DEVICE_OPERATOR_H
#define MULTI_DEVICE_OPERATOR_H

/** 
 * \class TMultiDeviceOperator
 *
 * \brief This class knows several devices and coordinates operations asked to them
 *
 * \author Andry Rakotozafindrabe
 *
 * The class can coordinates Master/Slaves MOSAIC boards for TDeviceOccupancyScan
 * or TDeviceDigitalScan operations running simultaneously on several devices.
 * A config file has to be given by the user for each MOSAIC board to be added. 
 * Obviously, only an identical type of operation can be performed for all boards.
 * 
 */

#include <memory>
#include <vector>
#include <string>
#include <thread>
#include "TVerbosity.h"

class TDevice;
class TDeviceHitScan;
class TSetup;

enum class MultiDeviceScanType {
        kDIGITAL_SCAN,
        kNOISE_OCC_SCAN
};

class TMultiDeviceOperator : public TVerbosity {

private: 

    /// vector of setup, owned by the class
    std::vector<std::shared_ptr<TSetup>> fSetups;

    /// vector of devices to be operated in a synchronous way (only references)
    std::vector<std::shared_ptr<TDevice>> fDevices;

    /// vector of operators on the devices, owned by the class
    std::vector<std::shared_ptr<TDeviceHitScan>> fDeviceOperators;

    /// vector of threads
    std::vector<std::thread> fReadingThreadList;  
    

    /// type of scan to be performed on all devices
    MultiDeviceScanType fScanType;

    /// total number of devices
    unsigned int fNDevices;

    /// boolean used to check if the admission of an additional setup is closed
    bool fIsAdmissionClosed;

    /// boolean will be set to true if all device operators are properly initialized
    bool fIsInitDone;

    /// part (prefix) of the name of the output files
    std::string fName;

public: 

    /// default constructor
    TMultiDeviceOperator();
    
    /// destructor
    virtual ~TMultiDeviceOperator();

    /// set the type of scan
    void SetScanType( const MultiDeviceScanType value );

    /// method that sets part of the name of the output files
    void SetPrefixFilename( std::string prefixFileName ) { fName = prefixFileName; }

    /// add a setup
    void AddSetup( const std::string aConfigFileName ); 

    /// close the admission to any additional setup
    void CloseAdmission(); 

    /// perform the scan of the devices
    void Go();

    /// finish the job
    void Terminate();

    /// return the number of devices
    unsigned int GetNDevices() const { return fNDevices; }

    /// return the status of the admission 
    bool IsAdmissionClosed() const { return fIsAdmissionClosed; }

    /// read data for a given device (i.e. a board)
    void ReadEventDataByDevice( const unsigned int id, const int nTriggers = 0 );

    /// read data for all devices
    void ReadEventData( const int nTriggers = 0 );

private:

    /// check if all device operators were correctly initialized
    void CheckInitAllOk();

    /// perform a digital scan on all devices
    void GoDigitalScan();

    /// perform a noise occupancy scan on all devices
    void GoNoiseOccScan();

    /// forward mask stage operations to each Alpide in all devices
    void DoConfigureMaskStage( int nPix, const int iStage );

    /// send triggers (they will only be sent from the Master MOSAIC board)
    void DoTrigger( const int nTriggers );   

    /// finish the digital scans
    void TerminateDigitalScan();

    /// finish the noise occupancy scans 
    void TerminateNoiseOccScan();

};

#endif  
