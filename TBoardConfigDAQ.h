#ifndef BOARDCONFIGDAQ_H
#define BOARDCONFIGDAQ_H

//#include "stdint.h"

#include "TBoardConfig.h"


// default values for conig parameters; less important ones are set in the TBoardConfig constructor


//---- ADC module
const int LIMIT_DIGITAL   = 300;
const int LIMIT_IO        =  50; // depreciated but leave in to write the register to some defined value
const int LIMIT_ANALOGUE  = 300; 

//---- READOUT module
const bool DATA_SAMPLING_EDGE = 1;
const bool DATA_PKTBASED_EN   = 0; // packet based readout default now!
const bool DATA_DDR_EN        = 0;
const int  DATA_PORT          = 2;
const bool HEADER_TYPE        = 1; // as of firmware version 247e0611 the header type can be defined; 0 -> full header (default); 1 -> short header 
const int  BOARD_VERSION      = 1; // as of firmware version 247e0611 the DAQboard version (v2 or v3) must be defined; 0 -> v2; 1 -> v3;  

//---- TRIGGER module
const int TRIGGER_MODE        = 2; // 2: external, 1:internal
const uint32_t STROBE_DELAY   = 10;// delay between external trigger and trigger sent to chip; when cofiguring the feature with a train of N triggers, this will be the delay between subsequent triggers
const bool BUSY_CONFIG        = 0; // as of firmware version 247e0611
const bool BUSY_OVERRIDE      = 1;

//---- RESET module
const int AUTOSHTDWN_TIME    = 10;      // time until enabling of auto shutdown
const int CLOCK_ENABLE_TIME  = 12;      // time until clock is enabled
const int SIGNAL_ENABLE_TIME = 12;      // time until signals are enabled
const int DRST_TIME          = 13;      // time until drst is deasserted

const int PULSE_STROBE_DELAY  = 10;
const int STROBE_PULSE_SEQ    =  2;     // 3: just send pulse after external trigger..



//************************************************************
// TBoardConfigDAQ: config for Cagliari DAQboard 
//************************************************************

class TBoardConfigDAQ : public TBoardConfig {
 private:
  // config related to firmware modules
  //----------------------------------------------------------
  //
  ////---- ADC module

  // ADC config reg 0
  int fCurrentLimitDigital;   // 11: 0; threshold current for digital supply 
  int fCurrentLimitIo;        // 23:12; threshold current for digital io supply
  bool fAutoShutdownEnable;   //    24; 0: disable, 1: enable
  bool fLDOEnable;            //    25; 0: disable, 1: enable; LDOAutoShutOff has no effect if this it is not set
  bool fADCEnable;            //    26; 0: disable, 1: enable; enables current/voltage sampling
  bool fADCSelfStop;          //    27; samples (pre-samples + post-samples) are sent to the user only if a latch-up event occurs
  bool fDisableTstmpReset;    //    28; if set, disables the reset of the timestamp counter with the strobe; default is 0
  bool fPktBasedROEnableADC;   //    29; enables/disables packet based readout for the ADCs; default is 0.

  // ADC config reg 1
  int fCurrentLimitAnalogue;  // 11: 0; threshold current for analogue supply 

  // ADC config reg 2
  uint32_t fAutoShutOffDelay; // 19: 0; delay with a granularity of 12.5ns
  int fADCDownSamplingFact;   // 31:20; factor for downscaling ADC sampling rate: 1.194MHz/(fADCDownSamplingFact+1)


  ////---- READOUT module
  
  // Event builder config reg

  int fMaxDiffTriggers;       //  3: 0; maximum difference between the number of triggers sent and the number of events recorded before the busy signal to the TLU is set high.
  bool fSamplingEdgeSelect;   //     4; edge of clock on which the chip event data is sampled in the FPGA event builder. 0: positive edge, 1: negative edge (for pA1 inverted..)
  bool fPktBasedROEnable;     //     5; 0: disable, 1: enable
  bool fDDREnable;            //     6; 0: disable, 1: enable 
  int fDataPortSelect;        //  8: 7; 01: serial port, 10: parallel port
  int fFPGAEmulationMode;     // 10: 9; 00: FPGA is bus master, chip is in IB or OB master mode (default)
                              //        01: the FPGA emulates an OB master, chip is slave;    !! not working with pA3 and later versions
                              //        10: the FPGA emulates an OB slave, chip is OB master; !! not working with pA3 and later versions
  bool fHeaderType;           //    11; as of firmware version 247e0611 the header type can be defined; 0 -> full header; 1 -> short header 
  int  fBoardVersion;         //    12; as of firmware version 247e0611 the DAQboard version (v2 or v3) must be defined; 0 -> v2; 1 -> v3;  
  
  ////---- TRIGGER module
  
  // Busy configuration register
  uint32_t fBusyDuration;     // 31: 0; minimum duration of the busy generated by the trigger sequencer
    
  // Trigger configuration register
  //int fNTriggers;             // 15: 0; number of triggers sent to chip; if set to 0, continuous triggering will be performed; TODO: feature ever used? member of base class TBoardConfig
  int fTriggerMode;           // 18:16; 0: disabled, 
                              //        1: auto trigger mode: system triggers the chip automatically after a start trigger command
                              //        2: external trigger mode: system triggers the chip on external trigger after a start trigger command
  int fStrobeDuration;        // 26:19; strobe duration with 25 ns granularity; depreciated
  int fBusyConfig;            // 29:27; as of fw version 247e0611; 
                              //  BUSY_OUT = BUSY_IN or DAQ_BUSY or CHIP_BUSY_SIGNAL or CHIP_BUSY_WORD
                              //    3'b0 - All the components are enabled (default) 
                              //    Bit [29] - Setting this bit excludes CHIP_BUSY_WORD from the busy logic
                              //    Bit [28] - Setting this bit excludes CHIP_BUSY_SIGNAL from the busy logic 
                              //    Bit [27] - Setting this bit excludes BUSY_IN from the busy logic 


  // Strobe delay register
  uint32_t fStrobeDelay;      // 31: 0; delay between the external trigger and the strobe sent to chip; with 25 ns granularity

  // Busy override register
  bool fBusyOverride;         //     0; 0: TLU busy is overridden, busy to TLU is kept high; 1: TLU busy is not overridden



  ////---- CMU module
 
  // CMU config register 
  bool      fManchesterDisable;     //     0; 0: enable manchester encoding; 1: disable
  bool      fSamplingEdgeSelectCMU; //     1; edge of the FPGA clock on which the CMU data is sampled in the FPGA CMU receiver; 0: positive edge; 1: negative edge
  bool      fInvertCMUBus;          //     2; 0: bus not inverted; 1: bus inverted
  bool      fChipMaster;            //     3; 0: chip is master; 1: chip is slave

  
  ////---- RESET module

  // PULSE DRST PRST duration reg
  int fPRSTDuration;            //  7: 0; depreciated
  int fDRSTDuration;            // 15: 8; TODO: should rather be done via opcode?
  int fPULSEDuration;           // 31:16; depreciated

  // Power up sequencer delay reg
  int fAutoShutdownTime;        //  7: 0; delay with 51.2 us granularity from the LDOs ON to the start of the auto shutdown mechanism
  int fClockEnableTime;         // 15: 8; delay with 51.2 us granularity from the LDOs ON to the enable of the pALPIDEfs clock 
  int fSignalEnableTime;        // 23:16; delay with 51.2 us granularity from the LDOs ON to the enable of the pALPIDEfs input signals (except pCLK and DRST)
  int fDrstTime;                // 32:24; delay with 51.2 us granularity from the LDOs ON to the deassertion of DRST signal

  // PULSE STROBE delay sequence reg
  //int fPulseDelay;              // 15: 0; delay between strobe and pulse; member of base class TBoardConfig;
  int fStrobePulseSeq;          // 17:16; 0: pulse is generated after write access to pulse command register
                                //        1: strobe - delay - pulse; pulse is generated after a delay with respect to the strobe
                                //        2: pulse - delay - strobe; pulse is generated after a write access to pulse command register and a delay is enabled for a generated strobe; 
                                //                                   this strobe goes to the trigger sequencer module in external trigger mode
                                //        3: just pulse

  // PowerOnReset disable reg
  bool fPORDisable;              //     0; 0: enable POR; 1: disable


  ////---- ID module


  ////---- SOFTRESET module
  
  // Software reset duration register
  int fSoftResetDuration;       //  7: 0; software reset duration

  //----------------------------------------------------------



 protected:
  void InitParamMap ();
 public:
  TBoardConfigDAQ();

  //// getters for module config parameters 

  // ADC Module
  int GetCurrentLimitDigital()  {return fCurrentLimitDigital;};
  int GetCurrentLimitIo()       {return fCurrentLimitIo;};
  int GetCurrentLimitAnalogue() {return fCurrentLimitDigital;};
  bool GetAutoShutdownEnable()  {return fAutoShutdownEnable;};
  bool GetLDOEnable()           {return fLDOEnable;};
  bool GetADCEnable()           {return fADCEnable;};
  bool GetADCSelfStop()         {return fADCSelfStop;};
  bool GetDisableTstmpReset()   {return fDisableTstmpReset;};
  bool GetPktBasedROEnableADC() {return fPktBasedROEnableADC;};
  uint32_t GetAutoShutOffDelay(){return fAutoShutOffDelay;};
  int GetADCDownSamplingFact()  {return fADCDownSamplingFact;};

  // READOUT Module
  int GetMaxDiffTriggers()      {return fMaxDiffTriggers;};
  bool GetSamplingEdgeSelect()  {return fSamplingEdgeSelect;};
  bool GetPktBasedROEnable()    {return fPktBasedROEnable;}; 
  bool GetDDREnable()           {return fDDREnable;};
  int GetDataPortSelect()       {return fDataPortSelect;};
  int GetFPGAEmulationMode()    {return fFPGAEmulationMode;};  
  bool GetHeaderType()          {return fHeaderType;};  

  // TRIGGER Module
  uint32_t GetBusyDuration()    {return fBusyDuration;};
  //int GetNTriggers()            {return fNTriggers;}; defined in base class TBoardConfig
  int GetTriggerMode()          {return fTriggerMode;};
  int GetStrobeDuration()       {return fStrobeDuration;};
  uint32_t GetStrobeDelay()     {return fStrobeDelay;};
  int GetBusyConfig()           {return fBusyConfig;}; 
  bool GetBusyOverride()        {return fBusyOverride;};

  // CMU Module
  bool GetManchesterDisable()    {return fManchesterDisable;};
  bool GetSamplingEdgeSelectCMU(){return fSamplingEdgeSelectCMU;};
  bool GetInvertCMUBus()         {return fInvertCMUBus;};
  bool GetChipMaster()           {return fChipMaster;};

  // RESET Module
  int GetPRSTDuration()         {return fPRSTDuration;};
  int GetDRSTDuration()         {return fDRSTDuration;};
  int GetPULSEDuration()        {return fPULSEDuration;};

  //int GetPulseDelay()           {return fPulseDelay;}; defined in base class TBoardConfig
  int GetStrobePulseSeq()       {return fStrobePulseSeq;};
  int GetPORDisable()           {return fPORDisable;};


  int GetAutoShutdownTime() {return fAutoShutdownTime;};
  int GetClockEnableTime()  {return fClockEnableTime;};
  int GetSignalEnableTime() {return fSignalEnableTime;};
  int GetDrstTime()         {return fDrstTime;};

  
  // SOFTRESET Module
  int GetSoftResetDuration()     {return fSoftResetDuration;};
   


  //// setters for module config parameters
  
  // ADC Module
  void SetAutoShutdownEnable(bool enable)  {fAutoShutdownEnable = enable;};
  void SetLDOEnable(bool enable)           {fLDOEnable          = enable;};

  // READOUT Module 
  void SetDataPortSelect(int dataPort)     {fDataPortSelect      = dataPort;};
  void SetPktBasedROEnable(bool enable)    {fPktBasedROEnable    = enable;};

  // TRIGGER Module
  // void SetNTriggers   (int nTriggers)      {fNTriggers          = nTriggers;}; // defined in base class TBoardConfig
  void SetTriggerMode (int triggerMode)    {fTriggerMode        = triggerMode;};
  void SetStrobeDelay (uint32_t delay)     {fStrobeDelay        = delay;};
  void SetBusyOverride(bool busyOverride)  {fBusyOverride       = busyOverride;};
  
  // CMU Module 

  // RESET Module
  void SetDrstTime(int duration)           {fDrstTime           = duration;};
  void SetClockEnableTime(int duration)    {fClockEnableTime    = duration;};
  void SetSignalEnableTime(int duration)   {fSignalEnableTime   = duration;};
  void SetAutoShutdownTime(int duration)   {fAutoShutdownTime   = duration;};
  void SetPORDisable(bool disable)         {fPORDisable         = disable;};

  void SetStrobePulseSeq(int strobePulseSeq) {fStrobePulseSeq   = strobePulseSeq;};

  // ID Module

  // SOFTRESET Module
  void SetSoftResetDuration(int duration)  {fSoftResetDuration  = duration;};

 
};

//************************************************************

#endif   /* BOARDCONFIGDAQ_H */
