#ifndef BOARDCONFIGMOSAIC_H
#define BOARDCONFIGMOSAIC_H

#include "TBoardConfig.h"
//#include "MosaicSrc/mruncontrol.h"
#include "mruncontrol.h"
#include "mdictionary.h"
#include <stdio.h>
#include <cstdint>


/// \class TBoardConfigMOSAIC
/// \brief Derived TBoardConfig class for MOSAIC board
/// \author A.Franco - INFN BARI
/// \details ver.0.1 3/8/2016

class TBoardConfigMOSAIC : public TBoardConfig {

private:
	FILE* fhConfigFile; ///< the file handle of the Configuration File

protected:

    void InitParamMap();

    char IPAddress[30];
	
    int TCPPort;
    int NumberOfControlInterfaces;
	int ControlInterfacePhase;
	int RunCtrlAFThreshold;
	int RunCtrlLatMode;
	int RunCtrlTimeout;
	int pollDataTimeout;
    int Inverted;
    int SpeedMode;
	int ManchesterDisable; // default 0: enable manchester encoding; 1: disable
  	int MasterSlaveModeOn; // default 1: activated; 0: off (backward compatibility with old firmware)
	int MasterSlaveMode; // default 0: alone; 1: master; 2: slave
	int TrgRecorderEnable; // default 0: disable trigger recording; 1: enable
	int DeviceId; // the Id of the ladder (or stave) being read by the MOSAIC
    
    // Receiver map to data links
    static const int RCVMAP[];
    static const unsigned int RCVMAPsize = 9;

public:
	TBoardConfigMOSAIC(const char *fName = 0);
    virtual ~TBoardConfigMOSAIC();

	// Info methods

	// Getters
	char *        GetIPaddress              () { return(IPAddress); }
	std::uint16_t GetTCPport                () { return((std::uint16_t)TCPPort); }
	std::uint16_t GetCtrlInterfaceNum       () { return((std::uint16_t)NumberOfControlInterfaces); }
	std::uint16_t GetCtrlInterfacePhase     () { return((std::uint16_t)ControlInterfacePhase); }
	std::uint32_t GetCtrlAFThreshold        () { return((std::uint32_t)RunCtrlAFThreshold); }
	std::uint16_t GetCtrlLatMode            () { return((std::uint16_t)RunCtrlLatMode); }
	std::uint32_t GetCtrlTimeout            () { return((std::uint32_t)RunCtrlTimeout); }
	std::uint32_t GetPollingDataTimeout     () { return((std::uint32_t)pollDataTimeout); }
	std::uint32_t GetManchesterDisable      () { return ((uint32_t)ManchesterDisable); }
	bool          IsMasterSlaveModeOn       () { return (bool)MasterSlaveModeOn; }
  	std::uint32_t GetMasterSlaveMode        () { return ((uint32_t)MasterSlaveMode); }
	bool 		  IsTrgRecorderEnable       () { return (bool)TrgRecorderEnable; }
	bool          IsInverted                () { return((bool)Inverted); }
    MosaicReceiverSpeed GetSpeedMode();
    int GetRCVMAP( const unsigned int i ) const;
	int GetDeviceId() const { return DeviceId; }

	// Setters
	void SetIPaddress          (const char *AIPaddress);
	void SetTCPport            (const std::uint16_t APort)                 { TCPPort = (int)APort; }
	void SetCtrlInterfaceNum   (const std::uint16_t ACtrlInterfaceNumber)  { NumberOfControlInterfaces = (int)ACtrlInterfaceNumber; }
	void SetCtrlInterfacePhase (const std::uint16_t ACtrlInterfacePhase)   { ControlInterfacePhase = (int)ACtrlInterfacePhase; }
	void SetCtrlAFThreshold    (const std::uint32_t ACtrlAFThreshold)      { RunCtrlAFThreshold = (int)ACtrlAFThreshold; }
	void SetCtrlLatMode        (const std::uint16_t ARunCtrlLatencyMode)   { RunCtrlLatMode = (int)ARunCtrlLatencyMode; }
	void SetCtrlTimeout        (const std::uint32_t ARunCtrlTimeout)       { RunCtrlTimeout = (int)ARunCtrlTimeout; }
	void SetPollingDataTimeout (const std::uint32_t APollDataTimeout)      { pollDataTimeout = (int)APollDataTimeout; }
	void SetManchesterDisable  (const std::uint32_t AIsManchesterDisabled) { ManchesterDisable = (int)AIsManchesterDisabled; }
	void SetMasterSlaveModeOn  (const int AMasterSlaveModeOn)              { MasterSlaveModeOn = AMasterSlaveModeOn; }
	void SetMasterSlaveMode    (const std::uint32_t AMasterSlaveMode)      { MasterSlaveMode = (int)AMasterSlaveMode; }
	void SetTrgRecorderEnable  (const int ATrgRecorderEnable)              { TrgRecorderEnable = ATrgRecorderEnable; }
    void SetInvertedData       (const bool value)                          { Inverted = (int)value; }
	void SetSpeedMode          (const MosaicReceiverSpeed ASpeedMode);
	void SetDeviceId           (const int id)                              { DeviceId = id; }
};

#endif   /* BOARDCONFIGMOSAIC_H */
