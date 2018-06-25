#ifndef M_DICTIONARY_H
#define M_DICTIONARY_H

#include <cstdint>

enum class MosaicReceiverSpeed { // Receiver data rate (in Mbps)
    RCV_RATE_400,
    RCV_RATE_600,
    RCV_RATE_1200
};

enum class MosaicLatencyMode { // previously in mruncontrol.h
    latencyModeEoe		= 0,
	latencyModeTimeout	= 1,
	latencyModeMemory	= 2
};

enum class MosaicConfigBits : std::uint32_t { // previously in mruncontrol.h
    CFG_EXTCLOCK_SEL_BIT = (1 << 0), // 0: internal clock - 1: external clock
    CFG_CLOCK_20MHZ_BIT  = (1 << 1), // 0: 40 MHz clock	- 1: 20 MHz clock
    CFG_RATE_MASK        = (0x03 << 2),
    CFG_RATE_1200        = (0 << 2),
    CFG_RATE_600         = (0x01 << 2),
    CFG_RATE_400         = (0x02 << 2)
  };

// status register bits
enum class MosaicStatusBits { // previously BOARD_STATUS_BITS in TReadoutBoardMosaic.h
	BOARD_STATUS_FEPLL_LOCK			= 0x0001,
	BOARD_STATUS_EXTPLL_LOCK		= 0x0002,
	BOARD_STATUS_GTPLL_LOCK			= 0x0004,
	BOARD_STATUS_GTP_RESET_DONE		= 0x3ff0000
};  

enum class MosaicOpCode : std::uint8_t { // previously in controlinterface.h
	OPCODE_STROBE_2		= 0xb1,			// Trigger
	OPCODE_GRST			= 0xd2,			// Global chip reset
	OPCODE_RORST		= 0x63,			// Readout reset
	OPCODE_PRST			= 0xe4,			// Pixel matrix reset
	OPCODE_STROBE_6		= 0x55,			// Trigger
	OPCODE_BCRST		= 0x36,			// Bunch Counter Reset
	OPCODE_ECRST		= 0x87,			// Event Counter Reset
	OPCODE_PULSE		= 0x78,			// Calibration Pulse trigger
	OPCODE_STROBE_10	= 0xc9,			// Trigger
	OPCODE_RSVD1		= 0xaa,			// Reserved for future use
	OPCODE_RSVD2		= 0x1b,			// Reserved for future use
	OPCODE_WROP			= 0x9c,			// Start Unicast or Multicast Write
	OPCODE_STROBE_14	= 0x2d,			// Trigger
	OPCODE_RDOP			= 0x4e			// Start Unicast Read
};

enum class MosaicReadWriteN { // previously in i2cbus.h
	I2C_Write = 0,
	I2C_Read = 1
};

enum class MosaicReadWriteFlags : std::uint32_t { // previously in i2cbus.h
	RWF_start	= 0x01,
	RWF_stop	= 0x02,
	RWF_dontAck	= 0x04
};

enum class MosaicIPbus : int { // previously in ipbus.h
    DEFAULT_PACKET_SIZE     = 1400,
    DEFAULT_UDP_PORT        = 2000,
    DEFAULT_TCP_BUFFER_SIZE = (512 * 1024), // if set to 0 : automatic
    DEFAULT_TCP_PORT        = 3333,
    HEADER_SIZE	            = 64,
    DATA_INPUT_BUFFER_SIZE  = 64 * 1024,
    IPBUS_PROTOCOL_VERSION  = 2,
	WRONG_PROTOCOL_VERSION  = 3,
	RCV_LONG_TIMEOUT        = 2000, // timeout in ms for the first rx datagrams
	RCV_SHORT_TIMEOUT       = 100 // timeout in ms for rx datagrams
};

// IPBus info codes (errors)
enum class MosaicIPbusInfoCode { // previously in ipbus.h // ? std::uint8_t 
	infoCodeSuccess 		= 0x0,
	infoCodeBadHeader 		= 0x1,
	infoCodeBusErrRead 		= 0x2,
	infoCodeBusErrWrite		= 0x3,
	infoCodeBusTimeoutRead	= 0x4,
	infoCodeBusTimeoutWrite	= 0x5,
	infoCodeBufferOverflaw	= 0x6,
	infoCodeBufferUnderflaw	= 0x7,
	infoCodeRequest 		= 0xf
};

// IPBus type of transaction
enum class MosaicIPbusTransaction { // previously in ipbus.h // ? std::uint8_t
	typeIdRead		 		= 0x0,
	typeIdWrite		 		= 0x1,
	typeIdNIRead		 	= 0x2,
	typeIdNIWrite		 	= 0x3,
	typeIdRMWbits		 	= 0x4,
	typeIdRMWsum		 	= 0x5,
	typeIdIdle			 	= 0xf
};

enum class MosaicBoardConfig { // previously in TBoardConfigMOSAIC.h
    MAX_TRANRECV              = 10,
	// default values for the board config
	MAX_CTRLINT               = 12, // max number of control interfaces
    DEF_TCPPORT               = 2000,
    DEF_CTRLINTPHASE          = 2,
    DEF_CTRLAFTHR             = 1250000,
	DEF_CTRLLATMODE           = 0,
    DEF_CTRLTIMEOUT           = 0,
    DEF_POLLDATATIMEOUT       = 500, // milliseconds
    DEF_POLARITYINVERSION     = 0,
	DEF_MANCHESTERDISABLE     = 0, // default 0: enable manchester encoding; 1: disable
	DEF_MASTERSLAVEMODEENABLE = 1, // default 1: activated; 0: off (backward compatibility with old firmware)
	DEF_MASTERSLAVEMODE       = 0, // default 0: alone; 1: master; 2: slave
	DEF_TRGRECORDERENABLE     = 0  // default 0: disable trigger recording; 1: enable
};

// singleton used to retrieve the correct type for the enum
class MosaicDict 
{
	static MosaicDict s;
	int i;
	MosaicDict(int x) : i(x) { }
	MosaicDict&
	operator=(MosaicDict&); // Disallowed
	MosaicDict(const MosaicDict&); // Disallowed

public:

	static MosaicDict& instance() { return s; }

	int           receiverSpeed( const MosaicReceiverSpeed v ) { return (int)v; }
	int           latencyMode( const MosaicLatencyMode v ) { return (int)v; }
	std::uint32_t configBits( const MosaicConfigBits v ) { return (std::uint32_t)v; }
	int           statusBits( const MosaicStatusBits v ) { return (int)v; }
	std::uint8_t  opCode( const MosaicOpCode v ) { return (std::uint8_t)v; }
	int           readWriteN( const MosaicReadWriteN v ) { return (int)v; }
	std::uint32_t readWriteFlags( const MosaicReadWriteFlags v ) { return (std::uint32_t)v;}
	int           iPbus( const MosaicIPbus v ) { return (int)v; }
	int           iPbusInfoCode( const MosaicIPbusInfoCode v ) { return (int)v; }
	int           iPbusTransaction( const MosaicIPbusTransaction v ) { return (int)v; }
	int           boardConfig( const MosaicBoardConfig v ) { return (int)v; }

};
// MosaicDict::instance().opCode()
#endif