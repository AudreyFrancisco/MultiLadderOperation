#include <stdlib.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstring>
#include "TBoardConfigMOSAIC.h"

using namespace std;

const int TBoardConfigMOSAIC::RCVMAP[] = { 3, 5, 7, 8, 6, 4, 2, 1, 0 };

//___________________________________________________________________
TBoardConfigMOSAIC::TBoardConfigMOSAIC( const char *AConfigFileName ) : TBoardConfig()
{
    fBoardType = TBoardType::kBOARD_MOSAIC;

	// Default values set
	TCPPort = (int)MosaicBoardConfig::DEF_TCPPORT;
	NumberOfControlInterfaces = (int)MosaicBoardConfig::MAX_CTRLINT;
	ControlInterfacePhase = (int)MosaicBoardConfig::DEF_CTRLINTPHASE;
	RunCtrlAFThreshold = (int)MosaicBoardConfig::DEF_CTRLAFTHR;
	RunCtrlLatMode = (int)MosaicLatencyMode::latencyModeEoe; 
	RunCtrlTimeout = (int)MosaicBoardConfig::DEF_CTRLTIMEOUT;
	pollDataTimeout = (int)MosaicBoardConfig::DEF_POLLDATATIMEOUT; // milliseconds
	Inverted = (int)MosaicBoardConfig::DEF_POLARITYINVERSION;
	SpeedMode = (int)MosaicReceiverSpeed::RCV_RATE_400;
	ManchesterDisable = (int)MosaicBoardConfig::DEF_MANCHESTERDISABLE;
  	MasterSlaveModeOn = (int)MosaicBoardConfig::DEF_MASTERSLAVEMODEENABLE;
	MasterSlaveMode = (int)MosaicBoardConfig::DEF_MASTERSLAVEMODE; 
	TrgRecorderEnable = (int)MosaicBoardConfig::DEF_TRGRECORDERENABLE;
	DeviceId = 0;

	if (AConfigFileName) { // Read Configuration file
		try {
			if(AConfigFileName == NULL || strlen(AConfigFileName) == 0) throw std::invalid_argument("TBoardConfigMOSAIC::TBoardConfigMOSAIC() - invalid filename");
				fhConfigFile = fopen(AConfigFileName,"r"); // opens the file
			} catch (...) {
				throw std::invalid_argument("TBoardConfigMOSAIC::TBoardConfigMOSAIC() - file does not exist !");
			}
	}
    InitParamMap();
}

//___________________________________________________________________
TBoardConfigMOSAIC::~TBoardConfigMOSAIC()
{
    if ( fhConfigFile ) {
        fclose( fhConfigFile );
    }
}

//___________________________________________________________________
void TBoardConfigMOSAIC::InitParamMap()
{
	fSettings["TCPPORTNUMBER"]             = &TCPPort;
	fSettings["NUMBEROFCONTROLINTERFACES"] = &NumberOfControlInterfaces;
	fSettings["CONTROLINTERFACEPHASE"]     = &ControlInterfacePhase;
	fSettings["CONTROLAFTHRESHOLD"]        = &RunCtrlAFThreshold;
	fSettings["CONTROLLATENCYMODE"]        = &RunCtrlLatMode;
	fSettings["CONTROLTIMEOUT"]            = &RunCtrlTimeout;
	fSettings["POLLINGDATATIMEOUT"]        = &pollDataTimeout;
	fSettings["DATALINKPOLARITY"]          = &Inverted;
	fSettings["DATALINKSPEED"]             = &SpeedMode;
	fSettings["MANCHESTERDISABLED"]        = &ManchesterDisable;
  	fSettings["MASTERSLAVEMODEON"]         = &MasterSlaveModeOn;
	fSettings["MASTERSLAVEMODE"]           = &MasterSlaveMode;
	fSettings["TRGRECORDERENABLE"]         = &TrgRecorderEnable;
	fSettings["DEVICEID"]                  = &DeviceId;
  
	TBoardConfig::InitParamMap();
}

//___________________________________________________________________
MosaicReceiverSpeed TBoardConfigMOSAIC::GetSpeedMode()
{
	switch(SpeedMode) {
	case 0:
		return(MosaicReceiverSpeed::RCV_RATE_400);
		break;
	case 1:
		return(MosaicReceiverSpeed::RCV_RATE_600);
		break;
	case 2:
		return(MosaicReceiverSpeed::RCV_RATE_1200);
		break;
	default:
		return(MosaicReceiverSpeed::RCV_RATE_400);
		break;
	}
}

//___________________________________________________________________
int TBoardConfigMOSAIC::GetRCVMAP( const unsigned int i ) const
{
    try {
        if ( i>= RCVMAPsize ) {
            cerr << "TBoardConfigMOSAIC::GetRCVMAP() - index = " << i << endl;
            throw out_of_range("TBoardConfigMOSAIC::GetRCVMAP() - index out of range!");
        }
        return RCVMAP[i];
    } catch ( std::out_of_range &err ) {
        exit(EXIT_FAILURE);
    }
}

//___________________________________________________________________
void TBoardConfigMOSAIC::SetSpeedMode(MosaicReceiverSpeed ASpeedMode)
{
	switch(ASpeedMode) {
	case MosaicReceiverSpeed::RCV_RATE_400:
		SpeedMode = 0;
		break;
	case MosaicReceiverSpeed::RCV_RATE_600:
		SpeedMode = 1;
		break;
	case MosaicReceiverSpeed::RCV_RATE_1200:
		SpeedMode = 2;
		break;
	default:
		SpeedMode = 0;
		break;
	}
	return;
}


// ----- private methods ----

// sets the IP address
//___________________________________________________________________
void TBoardConfigMOSAIC::SetIPaddress(const char *AIPaddress)
{
	std::cout << "TBoardConfigMOSAIC::SetIPaddress() - IP Address " << AIPaddress << std::endl ;
	try {
		if(AIPaddress == NULL) throw std::invalid_argument("TBoardConfigMOSAIC::SetIPaddress() - invalid IP number");
		strcpy(IPAddress, AIPaddress);
	} catch (...) {
		throw std::invalid_argument("TBoardConfigMOSAIC::SetIPaddress() - bad IP parameter specification");
	}
	return;
}
