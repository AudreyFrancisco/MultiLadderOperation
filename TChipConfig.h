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
    int fVTEMP;
    int fIAUX2;
    int fIRESET;
    #pragma mark - Control register settings
    bool fReadoutMode; // false = triggered, true = continuous (influences busy handling)
    bool fEnableClustering;
    int fMatrixReadoutSpeed;
    int fSerialLinkSpeed;
    bool fEnableSkewingGlobal;
    bool fEnableSkewingStartRO;
    bool fEnableClockGating;
    bool fEnableCMUReadout;
    #pragma mark - Fromu settings
    int fStrobeDuration;
    int fStrobeGap;    // gap between subsequent strobes in sequencer mode
    int fStrobeDelay;  // delay from pulse to strobe if generated internally
    int fTriggerDelay; // delay between external trigger command and internally generated strobe
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
    bool fEnabledSlave;

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

    #pragma mark - checkers
    bool IsParameter(const char *Name) { return (fSettings.count(Name) > 0); }
    bool IsEnabled() const { return (fEnabled != 0); }
    bool IsOBMaster() const { return ((fChipId % 8 == 0) && (GetModuleId() > 0)); }
    bool HasEnabledSlave() const { return fEnabledSlave; }

    #pragma mark - getters
    int  GetParamValue(const char *Name);
    int  GetChipId() const;
    int  GetControlInterface() const;
    int  GetReceiver() const;
    int  GetModuleId() const                { return (fChipId & 0x70) >> 4; }
    
    bool GetReadoutMode() const             { return fReadoutMode; }
    bool GetEnableClustering() const        { return fEnableClustering; }
    int  GetMatrixReadoutSpeed() const      { return fMatrixReadoutSpeed; }
    int  GetSerialLinkSpeed() const         { return fSerialLinkSpeed; }
    bool GetEnableSkewingGlobal() const     { return fEnableSkewingGlobal; }
    bool GetEnableSkewingStartRO() const    { return fEnableSkewingStartRO; }
    bool GetEnableClockGating() const       { return fEnableClockGating; }
    bool GetEnableCMUReadout() const        { return fEnableCMUReadout; }
    
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
    static const int  VCASN;
    static const int  VCASN2;
    static const int  VCLIP;
    static const int  VRESETD;
    static const int  ITHR;
    static const int  IBIAS;
    static const int  VCASP;
    
    static const bool READOUT_MODE;
    static const bool ENABLE_CLUSTERING;
    static const int  MATRIX_READOUT_SPEED;
    static const int  SERIAL_LINK_SPEED;
    static const bool ENABLE_SKEWING_GLOBAL;
    static const bool ENABLE_SKEWING_STARTRO;
    static const bool ENABLE_CLOCK_GATING;
    static const bool ENABLE_CMU_READOUT;
    
    static const int  STROBE_DURATION;
    static const int  STROBE_GAP;
    static const int  STROBE_DELAY;
    static const int  TRIGGER_DELAY;
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
    
};


#endif   /* CHIPCONFIG_H */
