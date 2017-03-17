#include "TChipConfig.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;

const int  TChipConfig::VCASN   = 50;
const int  TChipConfig::VCASN2  = 64;
const int  TChipConfig::VCLIP   = 0;
const int  TChipConfig::VRESETD = 147;
const int  TChipConfig::ITHR    = 51;
const int  TChipConfig::IBIAS   = 64;
const int  TChipConfig::VCASP   = 86;

const bool TChipConfig::READOUT_MODE           = false;// triggered
const bool TChipConfig::ENABLE_CLUSTERING      = true;
const int  TChipConfig::MATRIX_READOUT_SPEED   = 1;
const int  TChipConfig::SERIAL_LINK_SPEED      = 1200;
const bool TChipConfig::ENABLE_SKEWING_GLOBAL  = false;
const bool TChipConfig::ENABLE_SKEWING_STARTRO = false;
const bool TChipConfig::ENABLE_CLOCK_GATING    = false;
const bool TChipConfig::ENABLE_CMU_READOUT     = false;

// timing values, to be refined
const int  TChipConfig::STROBE_DURATION = 80;       // 2 us
const int  TChipConfig::STROBE_GAP      = 4000;     // .1 ms
const int  TChipConfig::STROBE_DELAY    = 20;       // 500 ns
const int  TChipConfig::TRIGGER_DELAY   = 0;
const int  TChipConfig::PULSE_DURATION  = 500;      // 12.5 us

const int  TChipConfig::DCLK_RECEIVER   = 10;
const int  TChipConfig::DCLK_DRIVER     = 10;
const int  TChipConfig::MCLK_RECEIVER   = 10;
const int  TChipConfig::DCTRL_RECEIVER  = 10;
const int  TChipConfig::DCTRL_DRIVER    = 10;

const int  TChipConfig::PREVIOUS_ID        = 0x10;
const bool TChipConfig::INITIAL_TOKEN      = true;
const bool TChipConfig::DISABLE_MANCHESTER = false;
const bool TChipConfig::ENABLE_DDR         = true;

//___________________________________________________________________
TChipConfig::TChipConfig (int chipId, const char *fName)
{
    fChipId           = chipId;
    fEnabled          = true;
    fReceiver         = -1;
    fControlInterface = -1;
    
    // fill default values from constants
    fVCASN   = VCASN;
    fVCASN2  = VCASN2;
    fVCLIP   = VCLIP;
    fVRESETD = VRESETD;
    fITHR    = ITHR;
    fIBIAS   = IBIAS;
    
    fReadoutMode         = READOUT_MODE;
    fEnableClustering    = ENABLE_CLUSTERING;
    fMatrixReadoutSpeed  = MATRIX_READOUT_SPEED;
    fSerialLinkSpeed     = SERIAL_LINK_SPEED;
    fEnableSkewingGlobal = ENABLE_SKEWING_GLOBAL;
    fEnableClockGating   = ENABLE_CLOCK_GATING;
    fEnableCMUReadout    = ENABLE_CMU_READOUT;
    
    fStrobeDuration      = STROBE_DURATION;
    fStrobeGap           = STROBE_GAP;
    fStrobeDelay         = STROBE_DELAY;
    fTriggerDelay        = TRIGGER_DELAY;
    fPulseDuration       = PULSE_DURATION;
    
    fDclkReceiver        = DCLK_RECEIVER;
    fDclkDriver          = DCLK_DRIVER;
    fMclkReceiver        = MCLK_RECEIVER;
    fDctrlReceiver       = DCTRL_RECEIVER;
    fDctrlDriver         = DCTRL_DRIVER;
    
    fPreviousId          = PREVIOUS_ID;
    fInitialToken        = INITIAL_TOKEN;
    fDisableManchester   = DISABLE_MANCHESTER;
    fEnableDdr           = ENABLE_DDR;
    
    fEnabledSlave = false;
    
    if (fName) {
        // TODO: read information from file
    }
    
    InitParamMap();
}


//___________________________________________________________________
void TChipConfig::InitParamMap ()
{
    fSettings["CHIPID"]           = &fChipId;
    fSettings["RECEIVER"]         = &fReceiver;
    fSettings["CONTROLINTERFACE"] = &fControlInterface;
    fSettings["ENABLED"]          = &fEnabled;
    fSettings["ITHR"]             = &fITHR;
    fSettings["IDB"]              = &fIDB;
    fSettings["VCASN"]            = &fVCASN;
    fSettings["VCASN2"]           = &fVCASN2;
    fSettings["VCLIP"]            = &fVCLIP;
    fSettings["VRESETD"]          = &fVRESETD;
    fSettings["IBIAS"]            = &fIBIAS;
    fSettings["VCASP"]            =  &fVCASP;
    fSettings["VPULSEL"]          = &fVPULSEL;
    fSettings["VPULSEH"]          = &fVPULSEH;
    fSettings["VRESETP"]          = &fVRESETP;
    fSettings["VTEMP"]            = &fVTEMP;
    fSettings["IAUX2"]            = &fIAUX2;
    fSettings["IRESET"]           = &fIRESET;
    fSettings["STROBEDURATION"]   = &fStrobeDuration;
    fSettings["PULSEDURATION"]    = &fPulseDuration;
    fSettings["STROBEDELAYCHIP"]  = &fStrobeDelay;
    fSettings["READOUTMODE"]      = (int*)&fReadoutMode;
    fSettings["LINKSPEED"]        = &fSerialLinkSpeed;
}


//___________________________________________________________________
bool TChipConfig::SetParamValue (const char *Name, const char *Value)
{
    if (fSettings.find (Name) != fSettings.end()) {
        sscanf (Value, "%d", fSettings.find(Name)->second);
        return true;
    }
    
    return false;
}


//___________________________________________________________________
bool TChipConfig::SetParamValue (const char *Name, const int Value)
{
    if (fSettings.find (Name) != fSettings.end()) {
        *(fSettings.find(Name)->second) = Value;
        return true;
    }
    
    return false;
}

//___________________________________________________________________
int TChipConfig::GetParamValue (const char *Name)
{
    if (fSettings.find (Name) != fSettings.end()) {
        return *(fSettings.find(Name)->second);
    }
    return -1;
}

//___________________________________________________________________
void TChipConfig::SetEnableSlave( const bool value )
{
    if ( !IsOBMaster() ) {
        fEnabledSlave = false;
    } else {
        fEnabledSlave = value;
    }
    return;
}
