#ifndef DEVICE_H
#define DEVICE_H

/** 
 * \class TDevice
 *
 * \brief Container of configs (for chips and boards), of chips and boards.
 *
 * \author Andry Rakotozafindrabe
 *
 * This class owns the chips instances, their associated boards
 * instances, and their respective configurations instances. It isolates the code for
 * the representation of the device under test. The code for construction is in the
 * TDeviceBuilder class. This design pattern has been adopted since TDevice has
 * a complex internal structure. The builder class will handle the construction with
 * great care, with steps depending on the TDevice type that is wanted.
 * Hence, chips, boards, and their configurations are all instantiated by
 * the classes inherited from TDeviceBuilder base class.
 */

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
    unsigned int fNWorkingChips;
    unsigned int fNChips;
    unsigned int fNModules;
    unsigned int fStartChipId;
    TBoardType fBoardType;
    TDeviceType fDeviceType;
    std::vector<std::shared_ptr<TReadoutBoard>> fBoards;
    std::vector<std::shared_ptr<TAlpide>> fChips;
    std::vector<std::shared_ptr<TBoardConfig>> fBoardConfigs;
    std::vector<std::shared_ptr<TChipConfig>> fChipConfigs;
    std::vector<unsigned int> fNWorkingChipsPerBoard;
    
public:
    #pragma mark - Constructors/destructor
    TDevice();
    virtual ~TDevice();

    #pragma mark - setters
    void SetNChips( const unsigned int number );
    void SetNModules( const unsigned int number );
    void SetStartChipId( const unsigned int Id );
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
    void AddNWorkingChipCounterPerBoard( const unsigned int nChips );

    #pragma mark - getters
    std::shared_ptr<TReadoutBoard>  GetBoard( const unsigned int iBoard );
    std::shared_ptr<TBoardConfig>   GetBoardConfig( const unsigned int iBoard );
    std::shared_ptr<TReadoutBoard>  GetBoardByChip( const unsigned int iChip );
    unsigned int                    GetBoardIndexByChip( const unsigned int iChip );
    std::shared_ptr<TBoardConfig>   GetBoardConfigByChip( const unsigned int iChip );
    std::shared_ptr<TAlpide>        GetChip( const unsigned int iChip );
    std::shared_ptr<TAlpide>        GetChipById( const unsigned int chipId );
    unsigned int                    GetChipId( const unsigned int iChip ) const;
    unsigned int                    GetChipIndexById( const unsigned int chipId ) const;
    std::shared_ptr<TChipConfig>    GetChipConfig( const unsigned int iChip );
    std::shared_ptr<TChipConfig>    GetChipConfigById( const unsigned int chipId );
    inline TBoardType               GetBoardType() const { return fBoardType; }
    inline TDeviceType              GetDeviceType() const { return fDeviceType; }
    unsigned int                    GetNChips() const;
    unsigned int                    GetChipConfigsVectorSize() const;
    unsigned int                    GetNBoards( const bool useBoardConfigVector = true ) const;
    unsigned int                    GetNModules() const { return fNModules; }
    inline unsigned int             GetNWorkingChips() const { return fNWorkingChips; }
    unsigned int                    GetStartChipId();
    bool                            IsMFTLadder() const;
    inline bool                     IsConfigFrozen() const { return fCreatedConfig; }
    inline bool                     IsSetupFrozen() const { return fInitialisedSetup; }
    unsigned int                    GetNWorkingChipsPerBoard( const unsigned int iBoard ) const;

};


#endif  /* DEVICE_H */
