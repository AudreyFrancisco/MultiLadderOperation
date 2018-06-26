/* -------------------------------------------------
 *   Derived TReadOutBoard Class for MOSAIC board
 *
 *   ver.0.1		3/8/2016
 *
 *  Auth.: A.Franco	-  INFN BARI
 *
 *  		HISTORY
 *
 *
 */
#ifndef READOUTBOARDMOSAIC_H
#define READOUTBOARDMOSAIC_H

#include <exception>
#include <string>
#include <deque>
#include <iostream>
#include <cstdint>


#include "TAlpide.h"
#include "TReadoutBoard.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
// MOSAIC includes
#include "mdictionary.h"
#include "mwbb.h"
#include "ipbusudp.h"
#include "mruncontrol.h"
#include "i2cbus.h"
#include "controlinterface.h"
#include "pulser.h"
#include "mtriggercontrol.h"
#include "alpidercv.h"
#include "i2csyspll.h"
#include "mboard.h"
#include "mcoordinator.h"
#include "mdatareceiver.h"
#include "trgrecorderparser.h"
#include "trgrecorder.h"
#include "TAlpideDataParser.h"

#include <memory>

// Constant Definitions

extern std::vector<unsigned char> fDebugBuffer;

class DummyReceiver : public MDataReceiver
{
public:
	DummyReceiver() {} ;
	~DummyReceiver() {} ;

    long parse(int numClosed)
    {
        (void)numClosed;
        return (dataBufferUsed);
    };
};

// -----------------------------------------------------

class TReadoutBoardMOSAIC : public TReadoutBoard, private MBoard
{
// Methods
public:
    TReadoutBoardMOSAIC();
	TReadoutBoardMOSAIC( std::shared_ptr<TBoardConfigMOSAIC> boardConfig );
	virtual ~TReadoutBoardMOSAIC();

    std::weak_ptr<TBoardConfig> GetConfig() {return fBoardConfig;}
    std::weak_ptr<TBoardConfigMOSAIC> GetConfigBoard() {return fBoardConfig;}
    
	int SendOpCode(std::uint16_t  OpCode, std::uint8_t chipId);
	int SendOpCode(std::uint16_t  OpCode);
        // Markus: changed trigger delay type from std::uint32_t to int, since changed upstream
	int SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay);
	void SetTriggerSource(TTriggerSource triggerSource);
    uint32_t GetTriggerCount();
	int Trigger(int nTriggers);
        // Markus: changed data type from char to unsigned char; check that no problem
        // (should be OK at least for memcpy)
	int ReadEventData(int &nBytes, unsigned char *buffer);
	void StartRun();
	void StopRun();

    int ReadRegister(std::uint16_t Address, std::uint32_t &Value) 
    {
        std::cout << "TReadoutBoardMOSAIC::ReadRegister( " << Address << " , " << &Value << ") - doing nothing" << std::endl;
        (void)Address;
        (void)Value;
        return(0);
    };
	int WriteRegister(std::uint16_t Address, std::uint32_t Value)  
    {
        std::cout << "TReadoutBoardMOSAIC::WriteRegister( " << Address << " , " << Value << ") - doing nothing" << std::endl;
        (void)Address;
        (void)Value;
        return(0);
    };
    void EnableControlInterfaces(const bool en);
    void EnableControlInterface(const unsigned int interface, const bool en);
    void EnableClockOutputs(const bool en);
    void EnableClockOutput(const unsigned int interface, const bool en);
    bool ClockOutputsEnabled() const { return fClockOuputsEnabled; }
    
    virtual void SetVerboseLevel( const int level );

    inline std::string GetFwIdString() const { return fTheVersionId; }
    inline int   GetFwMajVersion() const { return fTheVersionMaj; }
    inline int   GetFwMinVersion() const { return fTheVersionMin; }
    //std::shared_ptr<MCoordinator> GetCoordinatorHandle() { return fCoordinator; };
    std::string  GetRegisterDump();
    MCoordinator::mode_t GetCoordinatorMode() const;

    void SendBroadcastReset();
    void SendBroadcastROReset();
    void SendBroadcastBCReset();

private:
	void init();
    std::string getFirmwareVersion();
	void enableDefinedReceivers();
	void setPhase(const int APhase, const int ACii = 0);
	void setSpeedMode(MosaicReceiverSpeed ASpeed);
	void setInverted (bool AInverted, int Aindex = -1);
	std::uint32_t decodeError();

protected:
    /// implementation of base class method to write chip registers
    int WriteChipRegister (std::uint16_t address, std::uint16_t value, std::uint8_t chipId =0, const bool doExecute = true  );

    /// implementation of base class method to read chip registers
    int ReadChipRegister  (std::uint16_t address, std::uint16_t &value, std::uint8_t chipId =0, const bool doExecute = true );
    
private:
    
	std::weak_ptr<TBoardConfigMOSAIC> fBoardConfig;
    std::unique_ptr<I2Cbus> fI2cBus;
    std::unique_ptr<Pulser>	fPulser;
    DummyReceiver* fDummyReceiver;
    std::unique_ptr<ControlInterface> fControlInterface[(int)MosaicBoardConfig::MAX_CTRLINT];
    std::unique_ptr<ALPIDErcv>	fAlpideRcv[(int)MosaicBoardConfig::MAX_TRANRECV];
    std::unique_ptr<TrgRecorder> fTrgRecorder;
    TrgRecorderParser* fTrgDataParser;
    std::unique_ptr<MCoordinator> fCoordinator;
    TAlpideDataParser* fAlpideDataParser[(int)MosaicBoardConfig::MAX_TRANRECV];
	//TBoardHeader 		theHeaderOfReadData;  // This will host the info catch from Packet header/trailer YCM: FIXME, not used
    std::string fTheVersionId;  // Version properties
    int	fTheVersionMaj;
    int	fTheVersionMin;
    static I2CSysPll::pllRegisters_t sysPLLregContent;
    bool fClockOuputsEnabled;
};
#endif    /* READOUTBOARDMOSAIC_H */
