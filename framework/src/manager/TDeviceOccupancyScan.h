#ifndef DEVICE_OCCUPANCY_SCAN_H
#define DEVICE_OCCUPANCY_SCAN_H

/**
 * \class TDeviceOccupancyScan
 *
 * \brief This class is the base class for the noise occupancy scan (int and ext triggers).
 *
 * \author Andry Rakotozafindrabe
 *
 */

#include <memory>
#include <map>
#include "TDeviceHitScan.h"
#include "TBoardConfig.h"
#include "Common.h"

class TScanHisto;
class TScanConfig;
class TErrorCounter;
class TAlpideDecoder;
class TBoardDecoder;
class TDevice;
class THitMapView;

class TDeviceOccupancyScan : public TDeviceHitScan {
    
protected:
    
    /// map to histograms (one per chip) of hit pixels
    std::shared_ptr<TScanHisto> fScanHisto;
    
    /// number of triggers per train
    int fNTriggersPerTrain;

    /// type of trigger (internal or external)
    TTriggerSource fTriggerSource;

    /// container of hit maps (one per chip index)
    std::map<int, std::shared_ptr<THitMapView>> fHitMapCollection;
        
public:
    
    /// constructor
    TDeviceOccupancyScan();
    
    /// constructor with a TDevice and a ScanConfig specified
    TDeviceOccupancyScan( std::shared_ptr<TDevice> aDevice,
                     std::shared_ptr<TScanConfig> aScanConfig );
    
    /// destructor
    virtual ~TDeviceOccupancyScan();
        
    /// initialisation
    void Init();

    /// propagate the verbosity level to data members
    void SetVerboseLevel( const int level );

    /// perform the mask scan of the device
    void Go();
    
    /// return true if the trigger is internally generated 
    bool IsInternalTrigger() const;

    /// fill hit map for all chips
    void FillHitMaps();

    /// write raw hit data to a text file
    void WriteDataToFile( const char *fName, bool Recreate = true );
    
    /// draw and save hit map or distributions
    void DrawAndSaveToFile( const char *fName );
    
protected:
    
    /// allocate memory for histogram of hit pixels for each enabled chip
    void AddHisto();

    /// add a hit map for a given chip to our collection
    void AddHitMapToCollection( const common::TChipIndex idx );

    /// retrieve scan parameters from the configuration file
    void InitScanParameters();
    
    /// configure readout boards
    void ConfigureBoards();
           
    /// check if there is any hit for the requested chip index
    bool HasData( const common::TChipIndex idx );


};

#endif
