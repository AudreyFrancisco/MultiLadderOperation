#ifndef READOUTBOARD_H
#define READOUTBOARD_H

#include <cstdint>
#include <vector>
#include <memory>

class TChipConfig;
class TBoardConfig;
enum class TTriggerSource;

//************************************************************
// abstract base class for all readout boards
//************************************************************

class TReadoutBoard {
    
protected:

    virtual int WriteChipRegister(std::uint16_t Address, std::uint16_t Value, std::uint8_t chipId = 0)  = 0;
    virtual int ReadChipRegister(std::uint16_t Address, std::uint16_t &Value, std::uint8_t chipID = 0) = 0;
    int GetControlInterface( const std::uint8_t chipId) const;
    int GetChipById( const std::uint8_t chipId) const;

    friend class TAlpide;     // could be reduced to the relevant methods ReadRegister, WriteRegister

public:
    
    TReadoutBoard();
    TReadoutBoard( std::shared_ptr<TBoardConfig> config );
    virtual ~TReadoutBoard();
    
    void AddChipConfig( std::shared_ptr<TChipConfig> newChipConfig );
    
    virtual std::weak_ptr<TBoardConfig> GetConfig() = 0;
    std::weak_ptr<TChipConfig> GetChipConfig(const int iChip) {return fChipPositions.at(iChip);}
    int GetReceiver(const std::uint8_t chipId) const;

    virtual int  ReadRegister      (std::uint16_t Address, std::uint32_t &Value) = 0;
    virtual int  WriteRegister     (std::uint16_t Address, std::uint32_t Value)  = 0;
    
    // sends op code to all control interfaces
    virtual int  SendOpCode        (std::uint16_t  OpCode) = 0;
    // sends op code to control interface belonging to chip chipId
    virtual int  SendOpCode        (std::uint16_t  OpCode, std::uint8_t chipId) = 0;
    
    virtual int  SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay) = 0;
    virtual void SetTriggerSource  (TTriggerSource triggerSource) = 0;
    virtual int  Trigger           (int nTriggers) = 0;
    virtual int  ReadEventData     (int &NBytes, unsigned char *Buffer) = 0; // TODO: max buffer size not needed??
    
protected:
    
    std::vector<std::weak_ptr<TChipConfig>> fChipPositions;

};

#endif  /* READOUTBOARD_H */
