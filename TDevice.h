#ifndef DEVICE_H
#define DEVICE_H

/// \class TDevice
/// \brief Container of configs (for chips and boards), of chips and boards.
///
/// This class owns the chips instances, their associated boards
/// instances, and their respective configurations instances. It isolates the code for
/// the representation of the device under test. The code for construction is in the
/// TDeviceBuilder class. This design pattern has been adopted since TDevice has
/// a complex internal structure. The builder class will handle the construction with
/// great care, with steps depending on the TDevice type that is wanted.
/// Hence, chips, boards, and their configurations are all instantiated by
/// the classes inherited from TDeviceBuilder base class.

#include <vector>
#include <memory>
#include "TVerbosity.h"

class TAlpide;
class TReadoutBoard;
class TReadoutBoardDAQ;
class TChipConfig;
class TBoardConfig;

// definition of standard setup types:
//   - single chip in OB mode with MOSAIC
//   - single chip in IB mode with MOSAIC
//   - MFT 5-chips ladder with MOSAIC
//   - etc ...
enum class TDeviceType {
    kCHIP_DAQ,
    kTELESCOPE,
    kOBHIC,
    kIBHIC,
    kMFT_LADDER5,
    kMFT_LADDER4,
    kMFT_LADDER3,
    kMFT_LADDER2,
    kOBCHIP_MOSAIC,
    kIBCHIP_MOSAIC,
    kHALFSTAVE,
    kUNKNOWN
};

enum class TBoardType;

class TDevice : public TVerbosity {
    
    bool fCreatedConfig;
    bool fInitialisedSetup;
    int fNWorkingChips;
    int fNChips;
    int fNModules;
    int fStartChipId;
    TBoardType fBoardType;
    TDeviceType fDeviceType;
    std::vector<std::shared_ptr<TReadoutBoard>> fBoards;
    std::vector<std::shared_ptr<TAlpide>> fChips;
    std::vector<std::shared_ptr<TBoardConfig>> fBoardConfigs;
    std::vector<std::shared_ptr<TChipConfig>> fChipConfigs;
    
public:
    #pragma mark - Constructors/destructor
    TDevice();
    virtual ~TDevice();

    #pragma mark - setters
    void SetNChips( const int number );
    void SetNModules( const int number );
    void SetStartChipId( const int Id );
    void SetBoardType( const TBoardType bt );
    void SetDeviceType( const TDeviceType dt );

    #pragma mark - toggle config creation and setup initialisation to true
    inline void FreezeConfig() { fCreatedConfig = true; }
    inline void FreezeSetup() { fInitialisedSetup = true; }
    
    #pragma mark - add an item to one of the vectors
    void AddBoard( std::shared_ptr<TReadoutBoard> newBoard );
    void AddBoardConfig( std::shared_ptr<TBoardConfig> newBoardConfig );
    void AddChip( std::shared_ptr<TAlpide> newChip );
    void AddChipConfig( std::shared_ptr<TChipConfig> newChipConfig );
    void IncrementWorkingChipCounter();

    #pragma mark - getters
    std::shared_ptr<TReadoutBoard>  GetBoard(const int iBoard);
    std::shared_ptr<TBoardConfig>   GetBoardConfig(const int iBoard);
    std::shared_ptr<TReadoutBoard>  GetBoardByChip(const int iChip);
    std::shared_ptr<TBoardConfig>   GetBoardConfigByChip(const int iChip);
    std::shared_ptr<TAlpide>        GetChip(const int iChip);
    std::shared_ptr<TAlpide>        GetChipById(const int chipId);
    int                             GetChipIndexById(const int chipId) const;
    std::shared_ptr<TChipConfig>    GetChipConfig(const int iChip);
    std::shared_ptr<TChipConfig>    GetChipConfigById(const int chipId);
    inline TBoardType               GetBoardType() const { return fBoardType; }
    inline TDeviceType              GetDeviceType() const { return fDeviceType; }
    int                             GetNChips() const;
    int                             GetChipConfigsVectorSize() const;
    int                             GetNBoards( const bool useBoardConfigVector = true ) const;
    int                             GetNModules() const { return fNModules; }
    inline int                      GetNWorkingChips() const { return fNWorkingChips; }
    int                             GetStartChipId();
    bool                            IsMFTLadder() const;
    inline bool                     IsConfigFrozen() const { return fCreatedConfig; }
    inline bool                     IsSetupFrozen() const { return fInitialisedSetup; }

};


#endif  /* DEVICE_H */
