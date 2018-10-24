#include "TChipConfig.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "AlpideDictionary.h"

using namespace std;

const int  TChipConfig::ITHR    = 51;
const int  TChipConfig::IDB     = 29;
const int  TChipConfig::VCASN   = 50;
const int  TChipConfig::VCASN2  = 62;
const int  TChipConfig::VCLIP   = 0;
const int  TChipConfig::VRESETD = 147;
const int  TChipConfig::VCASP   = 86;
const int  TChipConfig::VPULSEL = 0;
const int  TChipConfig::VPULSEH = 170;
const int  TChipConfig::IBIAS   = 64;

const int  TChipConfig::VRESETP = 117;
const int  TChipConfig::IRESET  = 50;

const bool TChipConfig::READOUT_MODE           = false;
const bool TChipConfig::ENABLE_CLUSTERING      = true;
const bool TChipConfig::MATRIX_READOUT_SPEED   = true;
const int  TChipConfig::SERIAL_LINK_SPEED      = (int)AlpideIBSerialLinkSpeed::IB1200;
const bool TChipConfig::ENABLE_SKEWING_GLOBAL  = false;
const bool TChipConfig::ENABLE_SKEWING_STARTRO = false;
const bool TChipConfig::ENABLE_CLOCK_GATING    = false;
const bool TChipConfig::ENABLE_CMU_READOUT     = false;

const int  TChipConfig::PIXEL_MEB_MASK = 0;
const bool TChipConfig::ENABLE_INTERNAL_STROBE = false;
const bool TChipConfig::ENABLE_BUSY_MONITORING = true;
const int  TChipConfig::TEST_PULSE_MODE = (int)AlpideTestPulseMode::ANALOGUE;
const bool TChipConfig::ENABLE_TEST_STROBE = false;
const bool TChipConfig::ENABLE_ROTATE_PULSE_LINES = false;

// timing values, to be refined
// in units of clock cycles (assuming a clock period of 25 ns)
const int  TChipConfig::TRIGGER_DELAY   = 0;
const int  TChipConfig::STROBE_DURATION = 80;       // 2 us
const int  TChipConfig::STROBE_GAP      = 4000;     // .1 ms
const int  TChipConfig::STROBE_DELAY    = 20;       // 500 ns
const int  TChipConfig::PULSE_DURATION  = 500;      // 12.5 us

const int  TChipConfig::DCLK_RECEIVER   = 10;
const int  TChipConfig::DCLK_DRIVER     = 10;
const int  TChipConfig::MCLK_RECEIVER   = 10;
const int  TChipConfig::DCTRL_RECEIVER  = 10;
const int  TChipConfig::DCTRL_DRIVER    = 10;

const int  TChipConfig::PREVIOUS_ID        = 0x10; // outer barrel scenario
const bool TChipConfig::INITIAL_TOKEN      = true; // outer barrel scenario
const bool TChipConfig::DISABLE_MANCHESTER = false;
const bool TChipConfig::ENABLE_DDR         = true;

const int  TChipConfig::DTU_CFG_PLL_PHASE  = 8;
const int  TChipConfig::DTU_CFG_PLL_STAGES = 1;

const int  TChipConfig::DTU_DACS_PLL_CHARGE_PUMP    = 8;
const int  TChipConfig::DTU_DACS_HIGH_SPEED_DRIVER  = 8;
const int  TChipConfig::DTU_DACS_PREEMP_DRIVER      = 0;


const bool TChipConfig::DTU_TEST1_TEST_ENABLE = false;
const bool TChipConfig::DTU_TEST1_PRBS_ENABLE = false;
const bool TChipConfig::DTU_TEST1_SINGLE_MODE = false;
const int  TChipConfig::DTU_TEST1_PRBS_RATE = (int)AlpidePrbs7Mode::PRBS1200;
const bool TChipConfig::DTU_TEST1_BYPASS_8B10B = false;

#pragma mark - constructors/destructors

//___________________________________________________________________
TChipConfig::TChipConfig()
{
    fChipId           = TChipConfigData::kInitValue;
    fEnabled          = true;
    fEnabledSlave     = false;
    fReceiver         = TChipConfigData::kInitValue;
    fControlInterface = TChipConfigData::kInitValue;
    
    // fill default values from constants

    fITHR    = ITHR;
    fIDB     = IDB;
    fVCASN   = VCASN;
    fVCASN2  = VCASN2;
    fVCLIP   = VCLIP;
    fVRESETD = VRESETD;
    fVCASP   = VCASP;
    fVPULSEL = VPULSEL;
    fVPULSEH = VPULSEH;
    fIBIAS   = IBIAS;
    
    fVRESETP = VRESETP;
    fIRESET  = IRESET;

    fReadoutMode         = READOUT_MODE;
    fEnableClustering    = ENABLE_CLUSTERING;
    fMatrixReadoutSpeed  = MATRIX_READOUT_SPEED;
    fSerialLinkSpeed     = SERIAL_LINK_SPEED;
    fEnableSkewingGlobal = ENABLE_SKEWING_GLOBAL;
    fEnableClockGating   = ENABLE_CLOCK_GATING;
    fEnableCMUReadout    = ENABLE_CMU_READOUT;
    
    fMEBmask                = PIXEL_MEB_MASK;
    fEnableInternalStrobe   = ENABLE_INTERNAL_STROBE;
    fEnableBusyMonitoring   = ENABLE_BUSY_MONITORING;
    fTestPulseMode          = TEST_PULSE_MODE;
    fEnableTestStrobe       = ENABLE_TEST_STROBE;
    fEnableRotatePulseLines = ENABLE_ROTATE_PULSE_LINES;
    
    fTriggerDelay        = TRIGGER_DELAY;
    fStrobeDuration      = STROBE_DURATION;
    fStrobeGap           = STROBE_GAP;
    fStrobeDelay         = STROBE_DELAY;
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
    
    fDtuConfReg_PllPhase    = DTU_CFG_PLL_PHASE;
    fDtuConfReg_PllStages   = DTU_CFG_PLL_STAGES;

    fDtuDacs_PllChargePump      = DTU_DACS_PLL_CHARGE_PUMP;
    fDtuDacs_HighSpeedDriver    = DTU_DACS_HIGH_SPEED_DRIVER;
    fDtuDacs_PreEmpDriver       = DTU_DACS_PREEMP_DRIVER;
    
    fDtuTest1_TestEnable        = DTU_TEST1_TEST_ENABLE;
    fDtuTest1_PrbsEnable        = DTU_TEST1_PRBS_ENABLE;
    fDtuTest1_TestSingleMode    = DTU_TEST1_SINGLE_MODE;
    fDtuTest1_PrbsRate          = DTU_TEST1_PRBS_RATE;
    fDtuTest1_Bypass8b10b       = DTU_TEST1_BYPASS_8B10B;

    InitParamMap();
}

//___________________________________________________________________
TChipConfig::TChipConfig( const int chipId )
{
    fChipId           = chipId;
    fEnabled          = true;
    fEnabledSlave     = false;
    fReceiver         = TChipConfigData::kInitValue;
    fControlInterface = TChipConfigData::kInitValue;
    
    // fill default values from constants
    
    fITHR    = ITHR;
    fIDB     = IDB;
    fVCASN   = VCASN;
    fVCASN2  = VCASN2;
    fVCLIP   = VCLIP;
    fVRESETD = VRESETD;
    fVCASP   = VCASP;
    fVPULSEL = VPULSEL;
    fVPULSEH = VPULSEH;
    fIBIAS   = IBIAS;
    
    fVRESETP = VRESETP;
    fIRESET  = IRESET;
   
    fReadoutMode         = READOUT_MODE;
    fEnableClustering    = ENABLE_CLUSTERING;
    fMatrixReadoutSpeed  = MATRIX_READOUT_SPEED;
    fSerialLinkSpeed     = SERIAL_LINK_SPEED;
    fEnableSkewingGlobal = ENABLE_SKEWING_GLOBAL;
    fEnableClockGating   = ENABLE_CLOCK_GATING;
    fEnableCMUReadout    = ENABLE_CMU_READOUT;
    
    fMEBmask                = PIXEL_MEB_MASK;
    fEnableInternalStrobe   = ENABLE_INTERNAL_STROBE;
    fEnableBusyMonitoring   = ENABLE_BUSY_MONITORING;
    fTestPulseMode          = TEST_PULSE_MODE;
    fEnableTestStrobe       = ENABLE_TEST_STROBE;
    fEnableRotatePulseLines = ENABLE_ROTATE_PULSE_LINES;

    fTriggerDelay        = TRIGGER_DELAY;
    fStrobeDuration      = STROBE_DURATION;
    fStrobeGap           = STROBE_GAP;
    fStrobeDelay         = STROBE_DELAY;
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
    
    fDtuConfReg_PllPhase    = DTU_CFG_PLL_PHASE;
    fDtuConfReg_PllStages   = DTU_CFG_PLL_STAGES;
    
    fDtuDacs_PllChargePump      = DTU_DACS_PLL_CHARGE_PUMP;
    fDtuDacs_HighSpeedDriver    = DTU_DACS_HIGH_SPEED_DRIVER;
    fDtuDacs_PreEmpDriver       = DTU_DACS_PREEMP_DRIVER;
    
    fDtuTest1_TestEnable        = DTU_TEST1_TEST_ENABLE;
    fDtuTest1_PrbsEnable        = DTU_TEST1_PRBS_ENABLE;
    fDtuTest1_TestSingleMode    = DTU_TEST1_SINGLE_MODE;
    fDtuTest1_PrbsRate          = DTU_TEST1_PRBS_RATE;
    fDtuTest1_Bypass8b10b       = DTU_TEST1_BYPASS_8B10B;
    
    InitParamMap();
}

//___________________________________________________________________
TChipConfig::TChipConfig( const int chipId, const int ci, const int rc )
{
    fChipId           = chipId;
    fEnabled          = true;
    fEnabledSlave     = false;
    fReceiver         = rc;
    fControlInterface = ci;
    
    // fill default values from constants

    fITHR    = ITHR;
    fIDB     = IDB;
    fVCASN   = VCASN;
    fVCASN2  = VCASN2;
    fVCLIP   = VCLIP;
    fVRESETD = VRESETD;
    fVCASP   = VCASP;
    fVPULSEL = VPULSEL;
    fVPULSEH = VPULSEH;
    fIBIAS   = IBIAS;
    
    fVRESETP = VRESETP;
    fIRESET  = IRESET;

    fReadoutMode         = READOUT_MODE;
    fEnableClustering    = ENABLE_CLUSTERING;
    fMatrixReadoutSpeed  = MATRIX_READOUT_SPEED;
    fSerialLinkSpeed     = SERIAL_LINK_SPEED;
    fEnableSkewingGlobal = ENABLE_SKEWING_GLOBAL;
    fEnableClockGating   = ENABLE_CLOCK_GATING;
    fEnableCMUReadout    = ENABLE_CMU_READOUT;
    
    fMEBmask                = PIXEL_MEB_MASK;
    fEnableInternalStrobe   = ENABLE_INTERNAL_STROBE;
    fEnableBusyMonitoring   = ENABLE_BUSY_MONITORING;
    fTestPulseMode          = TEST_PULSE_MODE;
    fEnableTestStrobe       = ENABLE_TEST_STROBE;
    fEnableRotatePulseLines = ENABLE_ROTATE_PULSE_LINES;

    fTriggerDelay        = TRIGGER_DELAY;
    fStrobeDuration      = STROBE_DURATION;
    fStrobeGap           = STROBE_GAP;
    fStrobeDelay         = STROBE_DELAY;
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
    
    fDtuConfReg_PllPhase    = DTU_CFG_PLL_PHASE;
    fDtuConfReg_PllStages   = DTU_CFG_PLL_STAGES;
    
    fDtuDacs_PllChargePump      = DTU_DACS_PLL_CHARGE_PUMP;
    fDtuDacs_HighSpeedDriver    = DTU_DACS_HIGH_SPEED_DRIVER;
    fDtuDacs_PreEmpDriver       = DTU_DACS_PREEMP_DRIVER;
    
    fDtuTest1_TestEnable        = DTU_TEST1_TEST_ENABLE;
    fDtuTest1_PrbsEnable        = DTU_TEST1_PRBS_ENABLE;
    fDtuTest1_TestSingleMode    = DTU_TEST1_SINGLE_MODE;
    fDtuTest1_PrbsRate          = DTU_TEST1_PRBS_RATE;
    fDtuTest1_Bypass8b10b       = DTU_TEST1_BYPASS_8B10B;
    
    InitParamMap();
}

#pragma mark - private methods

//___________________________________________________________________
void TChipConfig::InitParamMap()
{
    fSettings["CHIPID"]                 = &fChipId;
    fSettings["ENABLED"]                = &fEnabled;
    fSettings["RECEIVER"]               = &fReceiver;
    fSettings["CONTROLINTERFACE"]       = &fControlInterface;

    fSettings["ITHR"]                   = &fITHR;
    fSettings["IDB"]                    = &fIDB;
    fSettings["VCASN"]                  = &fVCASN;
    fSettings["VCASN2"]                 = &fVCASN2;
    fSettings["VCLIP"]                  = &fVCLIP;
    fSettings["VRESETD"]                = &fVRESETD;
    fSettings["VCASP"]                  =  &fVCASP;
    fSettings["VPULSEL"]                = &fVPULSEL;
    fSettings["VPULSEH"]                = &fVPULSEH;
    fSettings["IBIAS"]                  = &fIBIAS;

    fSettings["VRESETP"]                = &fVRESETP;
    //fSettings["VTEMP"]                  = &fVTEMP; // TODO: uncomment when default value is known (see also TAlpide::BaseConfigDACs() )
    //fSettings["IAUX2"]                  = &fIAUX2; // TODO: uncomment when default value is known (see also TAlpide::BaseConfigDACs() )
    fSettings["IRESET"]                 = &fIRESET;

    fSettings["READOUTMODE"]            = (int*)&fReadoutMode;
    fSettings["ENABLECLUSTERING"]       = (int*)&fEnableClustering;
    fSettings["MATRIXREADOUTSPEED"]     = (int*)&fMatrixReadoutSpeed;
    fSettings["LINKSPEED"]              = &fSerialLinkSpeed;
    fSettings["ENABLESKEWINGGLOBAL"]    = (int*)&fEnableSkewingGlobal;
    fSettings["ENABLESKEWINGSTARTRO"]   = (int*)&fEnableSkewingStartRO;
    fSettings["ENABLECLOCKGATING"]      = (int*)&fEnableClockGating;
    fSettings["ENABLECMUREADOUT"]       = (int*)&fEnableCMUReadout;

    fSettings["PIXELMEBMASK"]           = &fMEBmask;
    fSettings["ENABLEINTERNALSTROBE"]   = (int*)&fEnableInternalStrobe;
    fSettings["ENABLEBUSYMONITORING"]   = (int*)&fEnableBusyMonitoring;
    fSettings["TESTPULSEMODE"]          = &fTestPulseMode;
    fSettings["ENABLETESTSTROBE"]       = (int*)&fEnableTestStrobe;
    fSettings["ENABLEROTATEPULSELINES"] = (int*)&fEnableRotatePulseLines;

    fSettings["TRIGGERDELAY"]           = &fTriggerDelay;
    fSettings["STROBEDURATION"]         = &fStrobeDuration;
    fSettings["STROBEGAP"]              = &fStrobeGap;
    fSettings["STROBEDELAYCHIP"]        = &fStrobeDelay;
    fSettings["PULSEDURATION"]          = &fPulseDuration;

    fSettings["DCLKRECEIVER"]           = &fDclkReceiver;
    fSettings["DCLKDRIVER"]             = &fDclkDriver;
    fSettings["MCLKRECEIVER"]           = &fMclkReceiver;
    fSettings["DCTRLRECEIVER"]          = &fDctrlReceiver;
    fSettings["DCTRLDRIVER"]            = &fDctrlDriver;
    
    fSettings["DISABLEMANCHESTER"]      = (int*)&fDisableManchester;
    fSettings["ENABLEDDR"]              = (int*)&fEnableDdr;
   
    fSettings["DTUCFGPLLPHASE"]      = &fDtuConfReg_PllPhase;
    fSettings["DTUCFGPLLSTAGES"]     = &fDtuConfReg_PllStages;
    
    fSettings["DTUDACSPLLCHARGEPUMP"]      = &fDtuDacs_PllChargePump;
    fSettings["DTUDACSHSDRIVER"]           = &fDtuDacs_HighSpeedDriver;
    fSettings["DTUDACSPREEMPDRIVER"]       = &fDtuDacs_PreEmpDriver;

    fSettings["DTUTEST1TESTENABLE"]  = (int*)&fDtuTest1_TestEnable;
    fSettings["DTUTEST1PRBSENABLE"]  = (int*)&fDtuTest1_PrbsEnable;
    fSettings["DTUTEST1SINGLEMODE"]  = (int*)&fDtuTest1_TestSingleMode;
    fSettings["DTUTEST1PRBSRATE"]    = &fDtuTest1_PrbsRate;
    fSettings["DTUTEST1BYPASS8B10B"] = (int*)&fDtuTest1_Bypass8b10b;

}

#pragma mark - setters

//___________________________________________________________________
bool TChipConfig::SetParamValue(const char *Name, const char *Value)
{
    if (fSettings.find (Name) != fSettings.end()) {
        sscanf (Value, "%d", fSettings.find(Name)->second);
        return true;
    }
    cerr << "TChipConfig::SetParamValue() - Unknown parameter `-" << Name << "`" << endl;
    return false;
}


//___________________________________________________________________
bool TChipConfig::SetParamValue(const char *Name, const int Value)
{
    if (fSettings.find (Name) != fSettings.end()) {
        *(fSettings.find(Name)->second) = Value;
        return true;
    }
    cerr << "TChipConfig::SetParamValue() - Unknown parameter `-" << Name << "`" << endl;
    return false;
}

//___________________________________________________________________
void TChipConfig::SetChipId( const int chipId )
{
    if ( chipId <= TChipConfigData::kInitValue ) {
        throw invalid_argument( "TChipConfig::SetChipId() - wrong initialization" );
    }
    fChipId = chipId;
}

//___________________________________________________________________
void TChipConfig::SetControlInterface( const int ci )
{
    if ( ci <= TChipConfigData::kInitValue ) {
        throw invalid_argument( "TChipConfig::SetControlInterface() - wrong initialization" );
    }
    fControlInterface = ci;
}

//___________________________________________________________________
void TChipConfig::SetReceiver( const int rc )
{
    if ( rc <= TChipConfigData::kInitValue ) {
        throw invalid_argument( "TChipConfig::SetReceiver() - wrong initialization" );
    }
    fReceiver = rc;
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

#pragma mark - getters

//___________________________________________________________________
int TChipConfig::GetParamValue(const char *Name) const
{
    if (fSettings.find (Name) != fSettings.end()) {
        return *(fSettings.find(Name)->second);
    }
    cerr << "TChipConfig::GetParamValue() - Unknown parameter `-" << Name << "`" << endl;
    return TChipConfigData::kInitValue;
}

//___________________________________________________________________
int TChipConfig::GetChipId() const
{
    if ( fChipId <= TChipConfigData::kInitValue ) {
        cout << " WARNING > TChipConfig::GetChipId() - return an uninitialized value" << endl;
    }
    return fChipId;
}

//___________________________________________________________________
int TChipConfig::GetControlInterface() const
{
    if ( fControlInterface <= TChipConfigData::kInitValue ) {
        cout << " WARNING > TChipConfig::GetControlInterface() - return an uninitialized value" << endl;
    }
    return fControlInterface;
}

//___________________________________________________________________
int TChipConfig::GetReceiver() const
{
    if ( fReceiver <= TChipConfigData::kInitValue ) {
        cout << " WARNING > TChipConfig::GetReceiver() - return an uninitialized value" << endl;
    }
    return fReceiver;
}
