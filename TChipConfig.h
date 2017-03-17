#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include <map>
#include <string>

class TChipConfig {
    
private:
    std::map <std::string, int*> fSettings;
    int      fChipId;
    int      fEnabled;                 // variable to exclude (non-working) chip from tests, default true
    int      fReceiver;
    int      fControlInterface;
    // DACs used
    int  fITHR;
    int  fIDB;
    int  fVCASN;
    int  fVCASN2;
    int  fVCLIP;
    int  fVRESETD;
    int  fVCASP;
    int  fVPULSEL;
    int  fVPULSEH;
    int  fIBIAS;
    // DACs unused
    int  fVRESETP;
    int  fVTEMP;
    int  fIAUX2;
    int  fIRESET;
    // Control register settings
    bool fReadoutMode;        // false = triggered, true = continuous (influences busy handling)
    bool fEnableClustering;
    int  fMatrixReadoutSpeed;
    int  fSerialLinkSpeed;
    bool fEnableSkewingGlobal;
    bool fEnableSkewingStartRO;
    bool fEnableClockGating;
    bool fEnableCMUReadout;
    // Fromu settings
    int  fStrobeDuration;
    int  fStrobeGap;          // gap between subsequent strobes in sequencer mode
    int  fStrobeDelay;        // delay from pulse to strobe if generated internally
    int  fTriggerDelay;       // delay between external trigger command and internally generated strobe
    int  fPulseDuration;
    // Buffer current settings
    int  fDclkReceiver;
    int  fDclkDriver;
    int  fMclkReceiver;
    int  fDctrlReceiver;
    int  fDctrlDriver;
    // CMU / DMU settings
    int  fPreviousId;
    bool fInitialToken;
    bool fDisableManchester;
    bool fEnableDdr;
    bool fEnabledSlave;
    
public:
    TChipConfig               (int chipId, const char *fName = 0);
    virtual ~TChipConfig(){}
    void InitParamMap         ();
    bool SetParamValue        (const char *Name, const char *Value);
    bool SetParamValue        (const char *Name, const int Value);
    int  GetParamValue        (const char *Name) ;
    bool IsParameter          (const char *Name) {return (fSettings.count(Name) > 0);};
    int  GetChipId            () {return fChipId;};
    bool IsEnabled            () {return (fEnabled != 0);};
    void SetEnable            (bool Enabled) {fEnabled = Enabled?1:0;};
    int  GetModuleId          () {return (fChipId & 0x70) >> 4;};
    bool IsOBMaster           () {return ((fChipId % 8 == 0) && (GetModuleId() > 0));};
    bool HasEnabledSlave      () const { return fEnabledSlave; }
    
    bool GetReadoutMode          () {return fReadoutMode;};
    bool GetEnableClustering     () {return fEnableClustering;};
    int  GetMatrixReadoutSpeed   () {return fMatrixReadoutSpeed;};
    int  GetSerialLinkSpeed      () {return fSerialLinkSpeed;};
    bool GetEnableSkewingGlobal  () {return fEnableSkewingGlobal;};
    bool GetEnableSkewingStartRO () {return fEnableSkewingStartRO;};
    bool GetEnableClockGating    () {return fEnableClockGating;};
    bool GetEnableCMUReadout     () {return fEnableCMUReadout;};
    
    int  GetTriggerDelay         () {return fTriggerDelay;};
    int  GetStrobeDuration       () {return fStrobeDuration;};
    int  GetStrobeDelay          () {return fStrobeDelay;};
    int  GetPulseDuration        () {return fPulseDuration;};
    
    int  GetDclkReceiver         () {return fDclkReceiver;};
    int  GetDclkDriver           () {return fDclkDriver;};
    int  GetMclkReceiver         () {return fMclkReceiver;};
    int  GetDctrlReceiver        () {return fDctrlReceiver;};
    int  GetDctrlDriver          () {return fDctrlDriver;};
    
    int  GetPreviousId           () {return fPreviousId;};
    bool GetInitialToken         () {return fInitialToken;};
    bool GetDisableManchester    () {return fDisableManchester;};
    bool GetEnableDdr            () {return fEnableDdr;};
    
    void SetPreviousId           (int APreviousId)         {fPreviousId   = APreviousId;};
    void SetInitialToken         (bool AInitialToken)      {fInitialToken = AInitialToken;};
    void SetEnableDdr            (bool AEnableDdr)         {fEnableDdr    = AEnableDdr;};
    void SetDisableManchester    (bool ADisableManchester) {fDisableManchester = ADisableManchester;};
    void SetEnableSlave( const bool value );

private:
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
