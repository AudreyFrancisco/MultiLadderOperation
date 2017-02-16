#ifndef READOUTBOARD_H
#define READOUTBOARD_H

#include <stdint.h>
#include <vector>
#include <memory>

#include "MosaicSrc/mexception.h"
#include "TBoardConfig.h"

class TChip;

//************************************************************
// abstract base class for all readout boards
//************************************************************

class TReadoutBoard {
    
protected:

    virtual int WriteChipRegister   (uint16_t Address, uint16_t Value, uint8_t chipId = 0)  = 0;
    int         GetControlInterface (uint8_t chipId);
    int         GetReceiver         (uint8_t chipId);
    int         GetChipById         (uint8_t chipId);
    friend class TAlpide;     // could be reduced to the relevant methods ReadRegister, WriteRegister

public:
    
    TReadoutBoard();
    TReadoutBoard( TBoardConfig *config );
    ~TReadoutBoard();
    
    int          AddChip           (uint8_t chipId, int controlInterface, int receiver);
    void         SetChipEnable     (uint8_t chipId, bool Enable);
    
    void         SetControlInterface (uint8_t chipId, int controlInterface);
    void         SetReceiver         (uint8_t chipId, int receiver);
    
    TBoardConfig *GetConfig        () {return fBoardConfig;};
    
    virtual int  ReadRegister      (uint16_t Address, uint32_t &Value) = 0;
    virtual int  WriteRegister     (uint16_t Address, uint32_t Value)  = 0;
    
    virtual int  ReadChipRegister  (uint16_t Address, uint16_t &Value, uint8_t chipID = 0) = 0;
    
    // sends op code to all control interfaces
    virtual int  SendOpCode        (uint16_t  OpCode) = 0;
    // sends op code to control interface belonging to chip chipId
    virtual int  SendOpCode        (uint16_t  OpCode, uint8_t chipId) = 0;
    
    virtual int  SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay) = 0;
    virtual void SetTriggerSource  (TTriggerSource triggerSource) = 0;
    virtual int  Trigger           (int nTriggers) = 0;
    virtual int  ReadEventData     (int &NBytes, unsigned char *Buffer) = 0; // TODO: max buffer size not needed??
    
protected:
    
    TBoardConfig *fBoardConfig;
    std::vector<std::shared_ptr<TChip>> fChipPositions;

};

class TReadoutBoardError : public MException
{
public:
    explicit TReadoutBoardError( const std::string& __arg );
};



#endif  /* READOUTBOARD_H */
