#include <stdlib.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstring>
#include "TBoardConfigMOSAIC.h"

using namespace std;
using namespace BoardConfigMOSAIC;

const int TBoardConfigMOSAIC::RCVMAP[] = { 3, 5, 7, 8, 6, 4, 2, 1, 0 };

//___________________________________________________________________
TBoardConfigMOSAIC::TBoardConfigMOSAIC( const char *AConfigFileName ) : TBoardConfig()
{
    fBoardType = TBoardType::kBOARD_MOSAIC;

	// Default values set
	NumberOfControlInterfaces = MAX_MOSAICCTRLINT;
	TCPPort = DEF_TCPPORT;
	ControlInterfacePhase = DEF_CTRLINTPHASE;
	RunCtrlAFThreshold = DEF_CTRLAFTHR;
	RunCtrlLatMode = DEF_CTRLLATMODE; // 0 := latencyModeEoe, 1 := latencyModeTimeout, 2 := latencyModeMemory
	RunCtrlTimeout = DEF_CTRLTIMEOUT;
	pollDataTimeout = DEF_POLLDATATIMEOUT; // milliseconds
	Inverted = DEF_POLARITYINVERSION;
	SpeedMode = DEF_SPEEDMODE;

	if (AConfigFileName) { // Read Configuration file
		try {
			if(AConfigFileName == NULL || strlen(AConfigFileName) == 0) throw std::invalid_argument("MOSAIC Config : invalid filename");
			fhConfigFile = fopen(AConfigFileName,"r"); // opens the file
			} catch (...) {
				throw std::invalid_argument("MOSAIC Config : file not exists !");
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
	fSettings["NUMBEROFCONTROLINTERFACES"] = &NumberOfControlInterfaces;
	fSettings["TCPPORTNUMBER"] = &TCPPort;
	fSettings["CONTROLINTERFACEPHASE"] = &ControlInterfacePhase;
	fSettings["CONTROLAFTHRESHOLD"] = &RunCtrlAFThreshold;
	fSettings["CONTROLLATENCYMODE"] = &RunCtrlLatMode;
	fSettings["CONTROLTIMEOUT"] = &RunCtrlTimeout;
	fSettings["POLLINGDATATIMEOUT"] = &pollDataTimeout;
	fSettings["DATALINKPOLARITY"] = &Inverted;
	fSettings["DATALINKSPEED"] = &SpeedMode;

	TBoardConfig::InitParamMap();
}

//___________________________________________________________________
Mosaic::TReceiverSpeed TBoardConfigMOSAIC::GetSpeedMode()
{
	switch(SpeedMode) {
	case 0:
		return(Mosaic::RCV_RATE_400);
		break;
	case 1:
		return(Mosaic::RCV_RATE_600);
		break;
	case 2:
		return(Mosaic::RCV_RATE_1200);
		break;
	default:
		return(Mosaic::RCV_RATE_400);
		break;
	}
}

//___________________________________________________________________
int TBoardConfigMOSAIC::GetRCVMAP( const int i ) const
{
    try {
        if ( (i < 0) || (i>= RCVMAPsize) ) {
            cerr << "TBoardConfigMOSAIC::GetRCVMAP() - index = " << i << endl;
            throw out_of_range("TBoardConfigMOSAIC::GetRCVMAP() - index out of range!");
        }
        return RCVMAP[i];
    } catch ( std::out_of_range &err ) {
        exit(0);
    }
}

//___________________________________________________________________
void TBoardConfigMOSAIC::SetSpeedMode(Mosaic::TReceiverSpeed ASpeedMode)
{
	switch(ASpeedMode) {
	case Mosaic::RCV_RATE_400:
		SpeedMode = 0;
		break;
	case Mosaic::RCV_RATE_600:
		SpeedMode = 1;
		break;
	case Mosaic::RCV_RATE_1200:
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
	std::cout << "IP Address " << AIPaddress << std::endl ;
	try {
		if(AIPaddress == NULL) throw std::invalid_argument("MOSAIC Config : invalid IP number");
		strcpy(IPAddress, AIPaddress);
	} catch (...) {
		throw std::invalid_argument("MOSAIC Config : bad IP parameter specification");
	}
	return;
}
