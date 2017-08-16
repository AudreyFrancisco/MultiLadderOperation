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
#include "BoardDecoder.h"

#include "MosaicSrc/wbb.h"
#include "MosaicSrc/ipbusudp.h"
#include "MosaicSrc/mruncontrol.h"
#include "MosaicSrc/i2cbus.h"
#include "MosaicSrc/controlinterface.h"
#include "MosaicSrc/pulser.h"
#include "MosaicSrc/mtriggercontrol.h"
#include "MosaicSrc/alpidercv.h"
#include "MosaicSrc/i2csyspll.h"
#include "MosaicSrc/mboard.h"
#include "MosaicSrc/mdatareceiver.h"
#include "MosaicSrc/mdatagenerator.h"
#include "MosaicSrc/TAlpideDataParser.h"

#include <memory>

// Constant Definitions

extern std::vector<unsigned char> fDebugBuffer;

class DummyReceiver : public MDataReceiver
{
public:
	DummyReceiver() {} ;
	~DummyReceiver() {} ;

	long parse(int numClosed) { return(dataBufferUsed); };
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
    
	int SendOpCode        (std::uint16_t  OpCode, std::uint8_t chipId);
	int SendOpCode        (std::uint16_t  OpCode);
        // Markus: changed trigger delay type from std::uint32_t to int, since changed upstream
	int SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay);
	void SetTriggerSource  (TTriggerSource triggerSource);
	int Trigger           (int nTriggers);
        // Markus: changed data type from char to unsigned char; check that no problem
        // (should be OK at least for memcpy)
	int ReadEventData     (int &nBytes, unsigned char *buffer);
	void StartRun();
	void StopRun();

    int ReadRegister      (std::uint16_t Address, std::uint32_t &Value) {
        std::cout << "TReadoutBoardMOSAIC::ReadRegister( " << Address << " , " << &Value << ") - doing nothing" << std::endl;
        return(0);};
	int WriteRegister     (std::uint16_t Address, std::uint32_t Value)  {
        std::cout << "TReadoutBoardMOSAIC::WriteRegister( " << Address << " , " << Value << ") - doing nothing" << std::endl;

        return(0);};

private:
	void init();
	void enableDefinedReceivers();
	void setPhase(int APhase, int ACii = 0) {
			fControlInterface[ACii]->setPhase(APhase);
			fControlInterface[ACii]->addSendCmd(ControlInterface::OPCODE_GRST);
			fControlInterface[ACii]->execute();
			return;
		};

	void setSpeedMode(Mosaic::TReceiverSpeed ASpeed);
	void setInverted (bool AInverted, int Aindex = -1);

	std::uint32_t decodeError();

protected:
    /// implementation of base class method to write chip registers
    int WriteChipRegister (std::uint16_t address, std::uint16_t value, std::uint8_t chipId =0, const bool doExecute = true  );

    /// implementation of base class method to read chip registers
    int ReadChipRegister  (std::uint16_t address, std::uint16_t &value, std::uint8_t chipId =0, const bool doExecute = true );
    
// Properties
private:
    
	std::weak_ptr<TBoardConfigMOSAIC> fBoardConfig;
    std::shared_ptr<MDataGenerator> fDataGenerator;
    std::shared_ptr<I2Cbus> fI2cBus;
    std::shared_ptr<Pulser>	fPulser;
    DummyReceiver* fDummyReceiver;
    std::shared_ptr<ControlInterface> fControlInterface[BoardConfigMOSAIC::MAX_MOSAICCTRLINT];
    std::shared_ptr<ALPIDErcv>	fAlpideRcv[BoardConfigMOSAIC::MAX_MOSAICTRANRECV];
    TAlpideDataParser* fAlpideDataParser[BoardConfigMOSAIC::MAX_MOSAICTRANRECV];
	//TBoardHeader 		theHeaderOfReadData;  // This will host the info catch from Packet header/trailer

private:
	// status register bits
	enum BOARD_STATUS_BITS {
		BOARD_STATUS_FEPLL_LOCK			= 0x0001,
		BOARD_STATUS_EXTPLL_LOCK		= 0x0002,
		BOARD_STATUS_GTPLL_LOCK			= 0x0004,
		BOARD_STATUS_GTP_RESET_DONE		= 0x3ff0000
	};
    
};
#endif    /* READOUTBOARDMOSAIC_H */
