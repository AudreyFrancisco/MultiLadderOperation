#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include <map>
#include <string>

namespace ChipConfig {     // to avoid clashes with other configs (e.g. for STROBE_DELAY)
  const int  VCASN   = 50;
  const int  VCASN2  = 64;
  const int  VCLIP   = 0;
  const int  VRESETD = 147;
  const int  ITHR    = 51;
  const int  IBIAS   = 64;
  const int  VCASP   = 86;

  const bool READOUT_MODE           = false;// triggered
  const bool ENABLE_CLUSTERING      = true;
  const int  MATRIX_READOUT_SPEED   = 1;
  const int  SERIAL_LINK_SPEED      = 1200;
  const bool ENABLE_SKEWING_GLOBAL  = false;
  const bool ENABLE_SKEWING_STARTRO = false;
  const bool ENABLE_CLOCK_GATING    = false;
  const bool ENABLE_CMU_READOUT     = false;

  // timing values, to be refined
  const int  STROBE_DURATION = 80;       // 2 us
  const int  STROBE_GAP      = 4000;     // .1 ms
  const int  STROBE_DELAY    = 20;       // 500 ns
  const int  TRIGGER_DELAY   = 0;
  const int  PULSE_DURATION  = 2000;     // 50 us

  const int  DCLK_RECEIVER   = 10;
  const int  DCLK_DRIVER     = 10;
  const int  MCLK_RECEIVER   = 10;
  const int  DCTRL_RECEIVER  = 10;
  const int  DCTRL_DRIVER    = 10;

  const int  PREVIOUS_ID        = 0x10;
  const bool INITIAL_TOKEN      = true;
  const bool DISABLE_MANCHESTER = false;
  const bool ENABLE_DDR         = true;
};


class TChipConfig {
 private: 
  std::map <std::string, int*> fSettings;
  int  fChipId;
  int  fEnabled;                 // variable to exclude (non-working) chip from tests, default true
  int  fReceiver;
  int  fControlInterface;
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
  
 protected:
 public:
  TChipConfig   (int chipId, const char *fName = 0);
  void InitParamMap         (); 
  bool SetParamValue        (const char *Name, const char *Value);
  int  GetParamValue        (const char *Name) ;
  bool IsParameter          (const char *Name) {return (fSettings.count(Name) > 0);};
  int  GetChipId            () {return fChipId;};
  bool IsEnabled            () {return (fEnabled != 0);};
  void SetEnable            (bool Enabled) {fEnabled = Enabled?1:0;};

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
};


#endif   /* CHIPCONFIG_H */
