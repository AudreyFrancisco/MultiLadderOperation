#ifndef TSETUP_H
#define TSETUP_H

/// \class TSetup
/// \brief Read TDevice config from file and instruct TDeviceBuilder to build the device
///
/// This class read the TDevice configuration from the config file. It interacts with
/// the relevant daughter class of TDeviceBuilder, who builds the
/// components of the instance of TDevice that is described on the config file, i.e.
/// configurations (vector of TChipConfig, TBoardConfig), chips (vector of TAlpide) and
/// readout boards (vector of TReadoutBoard).

#include <unistd.h>
#include <string.h>
#include <memory>
#include "TVerbosity.h"

class TScanConfig;
class TDevice;
class TDeviceBuilder;

enum class TDeviceType;

class TSetup : public TVerbosity {

public:

    #pragma mark - Constructors/destructor
    TSetup();
    virtual ~TSetup();

    #pragma mark - setters
    void SetConfigFileName( const std::string name );

    #pragma mark - getters
    inline std::string  GetConfigurationFileName() const { return fConfigFileName; }
    std::weak_ptr<TDevice> GetDevice() { return fDevice; }
    std::weak_ptr<TScanConfig> GetScanConfig() { return fScanConfig; }
    
    #pragma mark - other public methods
    void DecodeCommandParameters( int argc, char **argv );
    void DumpConfigToFile( std::string fName );
    void ReadConfigFile();
    
private:
    
    #pragma mark - private methods
    void DecodeLine(const char* Line);
    void ParseLine( const char* Line, char* Param, char* Rest, int* Chip );
    TDeviceType ReadDeviceType( const char* deviceName );
    void InitDeviceBuilder( TDeviceType dt );

    
private:
    std::string fConfigFileName;
    FILE* fConfigFile;
    std::shared_ptr<TDeviceBuilder> fDeviceBuilder;
    std::shared_ptr<TDevice> fDevice;
    std::shared_ptr<TScanConfig> fScanConfig;
    static const std::string NEWALPIDEVERSION;
};

#endif
