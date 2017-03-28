#ifndef BOARDCONFIGMOSAIC_H
#define BOARDCONFIGMOSAIC_H

#include "TBoardConfig.h"
#include "MosaicSrc/mruncontrol.h"
#include <stdio.h>

/// \class TBoardConfigMOSAIC
/// \brief Derived TBoardConfig class for MOSAIC board
/// \author A.Franco - INFN BARI
/// \details ver.0.1 3/8/2016

namespace BoardConfigMOSAIC {
    const int MAX_MOSAICCTRLINT = 2;
    const int MAX_MOSAICTRANRECV = 10;
    const int MOSAIC_HEADER_LENGTH = 64;
    
    const int DEF_TCPPORT = 2000;
    const int DEF_CTRLINTPHASE = 2;
    const int DEF_CTRLAFTHR = 1250000;
    const int DEF_CTRLLATMODE = 0;
    const int DEF_CTRLTIMEOUT = 0;
    const int DEF_POLLDATATIMEOUT = 500;
    const int DEF_POLARITYINVERSION = 0;
    const int DEF_SPEEDMODE = 0;
}

class TBoardConfigMOSAIC : public TBoardConfig {

private:
	FILE* fhConfigFile; ///< the file handle of the Configuration File

protected:

    void InitParamMap();

    int NumberOfControlInterfaces;
    int TCPPort;
	int ControlInterfacePhase;
	int RunCtrlAFThreshold;
	int RunCtrlLatMode;
	int RunCtrlTimeout;
	int pollDataTimeout;
    int Inverted;
    int SpeedMode;

    char IPAddress[30];
//	Mosaic::TReceiverSpeed  SpeedMode;
    
    // Receiver map to data links
    static const int RCVMAP[];
    static const int RCVMAPsize = 9;

public:
	TBoardConfigMOSAIC(const char *fName = 0);
    virtual ~TBoardConfigMOSAIC();

	// Info methods

	// Getters
	char *   GetIPaddress          () {return(IPAddress);}
	uint16_t GetCtrlInterfaceNum   () {return((uint16_t)NumberOfControlInterfaces);}
	uint16_t GetTCPport            () {return((uint16_t)TCPPort);}
	uint16_t GetCtrlInterfacePhase () {return((uint16_t)ControlInterfacePhase);}
	uint32_t GetCtrlAFThreshold    () {return((uint32_t)RunCtrlAFThreshold);}
	uint16_t GetCtrlLatMode        () {return((uint16_t)RunCtrlLatMode);}
	uint32_t GetCtrlTimeout        () {return((uint32_t)RunCtrlTimeout);}
	uint32_t GetPollingDataTimeout () {return((uint32_t)pollDataTimeout);}
	bool     IsInverted            () {return((bool)Inverted);}
    Mosaic::TReceiverSpeed    GetSpeedMode();
    int GetRCVMAP( const int i ) const;

	// Setters
	void SetIPaddress          (const char *AIPaddress);
	void SetTCPport            (uint16_t APort)                { TCPPort = (int)APort;}
	void SetCtrlInterfaceNum   (uint16_t ACtrlInterfaceNumber) { NumberOfControlInterfaces = (int)ACtrlInterfaceNumber;}
	void SetCtrlInterfacePhase (uint16_t ACtrlInterfacePhase)  { ControlInterfacePhase = (int)ACtrlInterfacePhase;}
	void SetCtrlAFThreshold    (uint32_t ACtrlAFThreshold)     { RunCtrlAFThreshold = (int)ACtrlAFThreshold;}
	void SetCtrlLatMode        (uint16_t ARunCtrlLatencyMode)  { RunCtrlLatMode = (int)ARunCtrlLatencyMode;}
	void SetCtrlTimeout        (uint32_t ARunCtrlTimeout)      { RunCtrlTimeout = (int)ARunCtrlTimeout;}
    void SetInvertedData       (bool IsInverted)               { Inverted = (int)IsInverted;};
	void SetPollingDataTimeout (uint32_t APollDataTimeout)     { pollDataTimeout = (int)APollDataTimeout;}
	void SetSpeedMode          (Mosaic::TReceiverSpeed ASpeedMode);

};

#endif   /* BOARDCONFIGMOSAIC_H */
