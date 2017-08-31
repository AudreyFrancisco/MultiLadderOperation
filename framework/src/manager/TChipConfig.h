#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include <map>
#include <string>

namespace TChipConfigData {
    const int kInitValue = -1;
}

class TChipConfig {
    
private:
    std::map <std::string, int*> fSettings;
    int fChipId;
    int fEnabled; // variable to exclude (non-working) chip from tests, default true
    bool fEnabledSlave;
    int fReceiver;
    int fControlInterface;

#pragma mark - DACs used

    int fITHR;
    int fIDB;
    int fVCASN;
    int fVCASN2;
    int fVCLIP;
    int fVRESETD;
    int fVCASP;
    int fVPULSEL;
    int fVPULSEH;
    int fIBIAS;

#pragma mark - DACs unused

    int fVRESETP;
    //int fVTEMP; // TODO: uncomment when default value is known (see also TAlpide::BaseConfigDACs() )
    //int fIAUX2; // TODO: uncomment when default value is known (see also TAlpide::BaseConfigDACs() )
    int fIRESET;

#pragma mark - Mode Control Register settings

    bool fReadoutMode; // false = triggered, true = continuous (influences busy handling)
    bool fEnableClustering;
    bool fMatrixReadoutSpeed;
    int fSerialLinkSpeed;
    bool fEnableSkewingGlobal;
    bool fEnableSkewingStartRO;
    bool fEnableClockGating;
    bool fEnableCMUReadout;

#pragma mark - Fromu settings

    //---------------- FROMU config register 1
    
    /// default b000 enables all three MEB slices (bits 2:0)
    int  fMEBmask;

    /// strobe sequencer for continuous mode (bit 3 FROMU)
    bool fEnableInternalStrobe;

    /// controls if FROMU monitors the BUSY input and reject triggers when the BUSY is asserted (bit 4)
    bool fEnableBusyMonitoring;

    /// type of pulsing (analog or digital) of the pixels (bit 5)
    int fTestPulseMode;

    /// enable the automatic generation of an internal TRIGGER pulse after a PULSE command (bit 6)
    bool fEnableTestStrobe;

    /// enable automatic shift and rotate selected pulse line after each PULSE signal (bit 7)
    bool fEnableRotatePulseLines;

    //  all delays in unit of clock cycles (assuming a clock period of 25 ns)
    
    /// delay between external trigger command and internally generated strobe (bits 10:9)
    int fTriggerDelay;
    
    //---------------- other FROMU registers

    //  min. 1 clock cycle, max. 65536
    
    /// (value in FROMU config register 2)
    int fStrobeDuration;

    /// gap between subsequent strobes when internal sequencer mode is activated (value in FROMU config register 3)
    int fStrobeGap;

    /// delay from pulse to strobe if strobe generated internally (value in FROMU pulsing register 1)
    int fStrobeDelay;

    /// (value in FROMU pulsing register 2)
    int fPulseDuration;

#pragma mark - Buffer current settings

    int  fDclkReceiver;
    int  fDclkDriver;
    int  fMclkReceiver;
    int  fDctrlReceiver;
    int  fDctrlDriver;

#pragma mark - CMU / DMU settings

    int  fPreviousId;
    bool fInitialToken;
    bool fDisableManchester;
    bool fEnableDdr;

#pragma mark - DTU Configuration Register settings

    /// Serializer phase (bits 7:4), default 8
    int fDtuConfReg_PllPhase;
    
    /// PLL VCO delay stages control (bits 1:0), 0->3 stages, 1->4 (default),  3->5
    int fDtuConfReg_PllStages;

#pragma mark - DTU DACs Register settings

    /// PLL charge pump current setting (bits 3:0), default 8
    int fDtuDacs_PllChargePump;
    
    /// High speed line driver current setting (bits 7:4), default 15
    int fDtuDacs_HighSpeedDriver;
    
    /// Pre-emphasis driver current setting (bits 11:8), default 15
    int fDtuDacs_PreEmpDriver;
    
#pragma mark - DTU Test Register 1 settings
    
    bool fDtuTest1_TestEnable;
    bool fDtuTest1_PrbsEnable;
    bool fDtuTest1_TestSingleMode;
    int  fDtuTest1_PrbsRate;
    bool fDtuTest1_Bypass8b10b;

private:
    #pragma mark - private methods
    void InitParamMap();

public:
    
    #pragma mark - constructors/destructors
    TChipConfig();
    TChipConfig( const int chipId );
    TChipConfig( const int chipId, const int ci, const int rc );
    virtual ~TChipConfig(){}
    
    #pragma mark - setters
    bool SetParamValue(const char *Name, const char *Value);
    bool SetParamValue(const char *Name, const int Value);
    void SetChipId( const int chipId );
    void SetControlInterface( const int ci );
    void SetReceiver( const int rc );
    void SetEnable( const bool value ) { fEnabled = value; }
    void SetPreviousId( const int APreviousId )      { fPreviousId   = APreviousId; }
    void SetInitialToken( const bool AInitialToken ) { fInitialToken = AInitialToken; }
    void SetEnableDdr(const bool AEnableDdr )        { fEnableDdr    = AEnableDdr; }
    void SetDisableManchester( const bool ADisableManchester ) { fDisableManchester = ADisableManchester; }
    void SetEnableSlave( const bool value );
    void SetTestPulseMode( const int value ) { fTestPulseMode = value; }
    void SetEnableTestStrobe( const bool value ) { fEnableTestStrobe = value; }

    #pragma mark - checkers
    bool IsParameter(const char *Name) { return (fSettings.count(Name) > 0); }
    bool IsEnabled() const { return (fEnabled != 0); }
    bool IsOBMaster() const { return ((fChipId % 8 == 0) && (GetModuleId() > 0)); }
    bool IsIBMode() const { return ((GetModuleId() == 0) && ((fChipId & 0xf) < 9)); }
    bool IsOBSlave() const { return ((!IsIBMode()) && (!IsOBMaster())); }
    bool HasEnabledSlave() const { return fEnabledSlave; }

    #pragma mark - getters
    int  GetParamValue(const char *Name) const;
    int  GetChipId() const;
    int  GetControlInterface() const;
    int  GetReceiver() const;
    int  GetModuleId() const                { return (fChipId & 0x70) >> 4; }
    
    bool GetReadoutMode() const             { return fReadoutMode; }
    bool GetEnableClustering() const        { return fEnableClustering; }
    bool GetMatrixReadoutSpeed() const      { return fMatrixReadoutSpeed; }
    int  GetSerialLinkSpeed() const         { return fSerialLinkSpeed; }
    bool GetEnableSkewingGlobal() const     { return fEnableSkewingGlobal; }
    bool GetEnableSkewingStartRO() const    { return fEnableSkewingStartRO; }
    bool GetEnableClockGating() const       { return fEnableClockGating; }
    bool GetEnableCMUReadout() const        { return fEnableCMUReadout; }

    int  GetPixelMEBMask() const            { return fMEBmask; }
    bool GetEnableInternalStrobe() const    { return fEnableInternalStrobe; }
    bool GetEnableBusyMonitoring() const    { return fEnableBusyMonitoring; }
    int  GetTestPulseMode() const           { return fTestPulseMode; }
    bool GetEnableTestStrobe() const        { return fEnableTestStrobe; }
    bool GetEnableRotatePulseLines() const  { return fEnableRotatePulseLines; }
    
    int  GetTriggerDelay() const            { return fTriggerDelay; }
    int  GetStrobeDuration() const          { return fStrobeDuration; }
    int  GetStrobeDelay() const             { return fStrobeDelay; }
    int  GetPulseDuration() const           { return fPulseDuration; }
    
    int  GetDclkReceiver() const            { return fDclkReceiver; }
    int  GetDclkDriver() const              { return fDclkDriver; }
    int  GetMclkReceiver() const            { return fMclkReceiver; }
    int  GetDctrlReceiver() const           { return fDctrlReceiver; }
    int  GetDctrlDriver() const             { return fDctrlDriver; }
    
    int  GetPreviousId() const              {return fPreviousId; }
    bool GetInitialToken() const            {return fInitialToken; }
    bool GetDisableManchester() const       {return fDisableManchester; }
    bool GetEnableDdr() const               {return fEnableDdr; }
    
private:
    #pragma mark - default value for the config
    static const int  ITHR;
    static const int  IDB;
    static const int  VCASN;
    static const int  VCASN2;
    static const int  VCLIP;
    static const int  VRESETD;
    static const int  VCASP;
    static const int  VPULSEL;
    static const int  VPULSEH;
    static const int  IBIAS;
    
    static const int  VRESETP;
    static const int  IRESET;

    static const bool READOUT_MODE;
    static const bool ENABLE_CLUSTERING;
    static const bool MATRIX_READOUT_SPEED;
    static const int  SERIAL_LINK_SPEED;
    static const bool ENABLE_SKEWING_GLOBAL;
    static const bool ENABLE_SKEWING_STARTRO;
    static const bool ENABLE_CLOCK_GATING;
    static const bool ENABLE_CMU_READOUT;

    static const int  PIXEL_MEB_MASK;
    static const bool ENABLE_INTERNAL_STROBE;
    static const bool ENABLE_BUSY_MONITORING;
    static const int  TEST_PULSE_MODE;
    static const bool ENABLE_TEST_STROBE;
    static const bool ENABLE_ROTATE_PULSE_LINES;

    static const int  TRIGGER_DELAY;
    static const int  STROBE_DURATION;
    static const int  STROBE_GAP;
    static const int  STROBE_DELAY;
    static const int  PULSE_DURATION;
    
    static const int  DCLK_RECEIVER;
    static const int  DCLK_DRIVER;
    static const int  MCLK_RECEIVER;
    static const int  DCTRL_RECEIVER;
    static const int  DCTRL_DRIVER;
    
    static const int  PREVIOUS_ID;
    static const bool INITIAL_TOKEN;
    static const bool DISABLE_MANCHESTER;
    static const bool ENABLE_DDR;
    
    static const int  DTU_CFG_PLL_PHASE;
    static const int  DTU_CFG_PLL_STAGES;
    
    static const int  DTU_DACS_PLL_CHARGE_PUMP;
    static const int  DTU_DACS_HIGH_SPEED_DRIVER;
    static const int  DTU_DACS_PREEMP_DRIVER;

    static const bool DTU_TEST1_TEST_ENABLE;
    static const bool DTU_TEST1_PRBS_ENABLE;
    static const bool DTU_TEST1_SINGLE_MODE;
    static const int  DTU_TEST1_PRBS_RATE;
    static const bool DTU_TEST1_BYPASS_8B10B;
    
};


#endif   /* CHIPCONFIG_H */
