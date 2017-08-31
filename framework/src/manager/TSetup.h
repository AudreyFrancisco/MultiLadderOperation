#ifndef TSETUP_H
#define TSETUP_H

/**
 * \class TSetup
 *
 * \brief Read TDevice config from file and instruct TDeviceBuilder to build the device
 *
 * \author Andry Rakotozafindrabe
 *
 * This class read the TDevice configuration from the config file. It interacts with
 * the relevant daughter class of TDeviceBuilder, who builds the
 * components of the instance of TDevice that is described on the config file, i.e.
 * configurations (TScanConfig, vectors of TChipConfig, TBoardConfig), chips
 * (vector of TAlpide) and readout boards (vector of TReadoutBoard).
 *
 * \note
 * This class re-uses most of the code written in the (obsolete) TConfig class to read
 * and decode the input config file. It is also re-using most of the code in the function
 * written in the obsolete SetupHelpers to decode the line command parameters.
 *
 * \warning
 * The current code was not demonstrated yet to be able to handle a number N > 1 of
 * readout boards (currently only one is written). See for e.g.
 * TDeviceBuilder::SetDeviceParamValue().
 * For MFT, this is enough since the implemented device types (the different
 * types of MFT ladders) only need one readout board to be entirely read.
*/

#include <unistd.h>
#include <string.h>
#include <memory>
#include "TVerbosity.h"

enum class TDeviceType;

class TScanConfig;
class TDevice;
class TDeviceBuilder;

class TSetup : public TVerbosity {

public:

    #pragma mark - Constructors/destructor
    TSetup();
    virtual ~TSetup();

    #pragma mark - setters
    void SetConfigFileName( const std::string name );
    virtual void SetVerboseLevel( const int level );

    #pragma mark - getters
    inline std::string GetConfigurationFileName() const { return fConfigFileName; }
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
