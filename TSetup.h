#ifndef TSETUP_H
#define TSETUP_H

/// \class TSetup
/// \brief Container of configs (chips and boards), vector of chips, vector of boards
///
/// This class is a container for configurations (vector of TChipConfig, TBoardConfig),
/// chips (vector of TAlpide) and readout boards (vector of TReadoutBoard). It creates
/// and fills the configurations from the config file. The chips and readout boards are
/// then instanciated according to the configurations. A systematic check of the whole
/// system is conducted in order to disable non-working chips in case of device with
/// multiple chips.

#include <unistd.h>
#include <vector>
#include <string.h>
#include <memory>
#include <libusb-1.0/libusb.h>
#include "TBoardConfig.h"

class TAlpide;
class TReadoutBoard;
class TReadoutBoardDAQ;
class TChipConfig;
class TScanConfig;

// definition of standard setup types:
//   - single chip in OB mode with MOSAIC
//   - single chip in IB mode with MOSAIC
//   - MFT 5-chips ladder with MOSAIC
//   - etc ...
namespace Setup {
    enum TDeviceType {
        TYPE_CHIP_DAQ,
        TYPE_TELESCOPE,
        TYPE_OBHIC,
        TYPE_IBHIC,
        TYPE_MFT_LADDER5,
        TYPE_MFT_LADDER4,
        TYPE_MFT_LADDER3,
        TYPE_MFT_LADDER2,
        TYPE_OBCHIP_MOSAIC,
        TYPE_IBCHIP_MOSAIC,
        TYPE_HALFSTAVE,
        TYPE_UNKNOWN
    };
    static struct libusb_context *fContext = 0;
}

class TSetup {

public:

    #pragma mark - Constructors/destructor
    TSetup();
    virtual ~TSetup();

    #pragma mark - setters
    inline void         SetVerboseLevel( const int level ) { fVerboseLevel = level; }
    void                SetConfigFileName( const std::string name );

    #pragma mark - getters
    std::shared_ptr<TReadoutBoard>  GetBoard(const int iBoard);
    std::shared_ptr<TBoardConfig>   GetBoardConfig(const int iBoard);
    inline TBoardType               GetBoardType() const { return fBoardType; }
    std::shared_ptr<TAlpide>        GetChip(const int iChip);
    std::shared_ptr<TChipConfig>    GetChipConfig(const int iChip);
    std::shared_ptr<TChipConfig>    GetChipConfigById(const int chipId);
    inline std::string              GetConfigurationFileName() const { return fConfigFileName; }
    inline TDeviceType              GetDeviceType() const { return fDeviceType; }
    int                             GetNChips() const;
    int                             GetNBoards() const { return (int)fBoardConfigs.size(); }
    int                             GetNModules() const { return fNModules; }
    inline int                      GetNWorkingChips() const { return fNWorkingChips; }
    std::shared_ptr<TScanConfig>    GetScanConfig() { return fScanConfig; }
    int                             GetStartChipID() const;
    bool                            IsMFTLadder() const;
    
    #pragma mark - other public methods
    void DecodeCommandParameters( int argc, char **argv );
    void DumpConfigToFile( std::string fName );
    void InitSetup();
    void ReadConfigFile();

    
private:
    
    #pragma mark - other private methods
    void CheckControlInterface();
    void DecodeLine(const char* Line);
    void EnableSlave( const int mychip );
    void MakeDaisyChain();
    void ParseLine( const char* Line, char* Param, char* Rest, int* Chip );
    void ReadDeviceType( const char* deviceName );
    #pragma mark - device creation
    void CreateDeviceConfig();
    void CreateHalfStave();
    void CreateIB();
    void CreateIBSingleMosaic();
    void CreateMFTLadder();
    void CreateOB();
    void CreateOBSingleDAQ();
    void CreateOBSingleMosaic();
    void CreateTelescopeDAQ();
    #pragma mark - setup initialisation
    void InitSetupHalfStave();
    void InitSetupIB();
    void InitSetupIBSingleMosaic();
    void InitSetupMFTLadder();
    void InitSetupOB();
    void InitSetupOBSingleDAQ();
    void InitSetupOBSingleMosaic();
    void InitSetupTelescopeDAQ();
    #pragma mark - Specific to DAQ board settings
    bool AddDAQBoard( std::shared_ptr<libusb_device> device );
    void FindDAQBoards();
    void InitLibUsb();
    bool IsDAQBoard( std::shared_ptr<libusb_device> device );
    void PowerOnDaqBoard( shared_ptr<TReadoutBoardDAQ> aDAQBoard );

    
private:
    bool fInitialisedSetup;
    bool fCreatedConfig;
    int fVerboseLevel;
    std::string fConfigFileName;
    int fNWorkingChips;
    int fNChips;
    int fNModules;
    int fStartChipId;
    TBoardType fBoardType;
    TDeviceType fDeviceType;
    std::shared_ptr<TScanConfig> fScanConfig;
    FILE* fConfigFile;
    std::vector<std::shared_ptr<TReadoutBoard>> fBoards;
    std::vector<std::shared_ptr<TAlpide>> fChips;
    std::vector<std::shared_ptr<TBoardConfig>> fBoardConfigs;
    std::vector<std::shared_ptr<TChipConfig>> fChipConfigs;
    static const int DEFAULT_MODULE_ID = 1;
    static const std::string NEWALPIDEVERSION;
};

#endif
