#include "AlpideDictionary.h"
#include "TAlpide.h"
#include "TChipConfig.h"
#include "TReadoutBoard.h"
#include <iostream>
#include <stdexcept>

using namespace std;


#pragma mark - Constructors/destructor

//___________________________________________________________________
TAlpide::TAlpide() :
    fChipId( -1 ),
    fADCBias( -1 ),
    fADCHalfLSB( false ),
    fADCSign( false )
{ }

//___________________________________________________________________
TAlpide::TAlpide( shared_ptr<TChipConfig> config ) :
    fChipId( -1 ),
    fADCBias( -1 ),
    fADCHalfLSB( false ),
    fADCSign( false )
{
    if ( !config ) {
        throw runtime_error( "TAlpide::TAlpide() - chip config. is a nullptr !" );
    }
    fConfig = config;
    fChipId = config->GetChipId();
}

//___________________________________________________________________
TAlpide::TAlpide( shared_ptr<TChipConfig> config,
                  shared_ptr<TReadoutBoard> readoutBoard ) :
    fChipId( -1 ),
    fADCBias( -1 ),
    fADCHalfLSB( false ),
    fADCSign( false )
{
    if ( !config ) {
        throw runtime_error( "TAlpide::TAlpide() - chip config. is a nullptr !" );
    }
    fConfig = config;
    fChipId = config->GetChipId();
    if ( !readoutBoard ) {
        throw runtime_error( "TAlpide::TAlpide() - readout board is a nullptr !" );
    }
    fReadoutBoard = readoutBoard;
}

//___________________________________________________________________
TAlpide::~TAlpide()
{
    fConfig.reset();
    fReadoutBoard.reset();
}

#pragma mark - setters/getters

//___________________________________________________________________
void TAlpide::SetEnable( const bool Enable )
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( spConfig ) {
        spConfig->SetEnable( Enable );
    } else {
        throw runtime_error( "TAlpide::SetEnable() - can not enable the chip config." );
    }
}

#pragma mark - basic operations with registers

//___________________________________________________________________
int TAlpide::ReadRegister( const AlpideRegister address, uint16_t &value )
{
  return ReadRegister( (uint16_t) address, value );
}


//___________________________________________________________________
int TAlpide::ReadRegister( const uint16_t address, uint16_t &value )
{
    if ( fChipId < 0 ) {
        throw domain_error( "TAlpide::ReadRegister() - undefined chip id.");
    }
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( spBoard ) {
        int err = spBoard->ReadChipRegister( address, value, fChipId );
        //if (err < 0) return err;  // readout board should have thrown an exception before
        return err;
    } else {
        throw runtime_error( "TAlpide::ReadRegister() - unuseable readout board." );
    }
}

//___________________________________________________________________
int TAlpide::WriteRegister( const AlpideRegister address,
                            uint16_t value, const bool verify)
{
  return WriteRegister( (uint16_t) address, value, verify );
}

//___________________________________________________________________
int TAlpide::WriteRegister( const uint16_t address,
                            uint16_t value, const bool verify )
{
    if ( fChipId < 0 ) {
        throw domain_error( "TAlpide::WriteRegister() - undefined chip id.");
    }
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( spBoard ) {
        int result = spBoard->WriteChipRegister( address, value, fChipId );
        if ((!verify) || (result < 0)) return result;
        uint16_t check;
        result = ReadRegister( address, check );
        if (result < 0) return result;
        if (check != value) return -1;      // raise exception (warning) readback != write value;
        return 0;  
    } else {
        throw runtime_error( "TAlpide::WriteRegister() - unuseable readout board." );
    }
}

//___________________________________________________________________
int TAlpide::ModifyRegisterBits( const AlpideRegister address,
                                 const uint8_t lowBit,
                                 const uint8_t nBits,
                                 uint16_t value,
                                 const bool verify )
{
  if ((lowBit < 0) || (lowBit > 15) || (lowBit + nBits > 15)) {
    return -1;    // raise exception illegal limits
  }
  uint16_t registerValue, mask = 0xffff; 
  ReadRegister( address, registerValue );
  
  for (int i = lowBit; i < lowBit + nBits; i++) {
    mask -= 1 << i;
  }

  registerValue &= mask;                  // set all bits that are to be overwritten to 0
  value         &= (1 << nBits) -1;       // make sure value fits into nBits
  registerValue |= value << nBits;        // or value into the foreseen spot

  return WriteRegister( address, registerValue, verify );

}

#pragma mark - dump

//___________________________________________________________________
void TAlpide::DumpConfig( const char *fName, const bool writeFile, char *config )
{
  uint16_t value;
   
  if (writeFile) {
    FILE *fp = fopen(fName, "w");
    // DACs
    ReadRegister(0x601, value);
    fprintf(fp, "VRESETP %i\n", value);
    ReadRegister(0x602, value);
    fprintf(fp, "VRESETD %i\n", value);
    ReadRegister(0x603, value);
    fprintf(fp, "VCASP   %i\n", value);
    ReadRegister(0x604, value);
    fprintf(fp, "VCASN   %i\n", value);
    ReadRegister(0x605, value);
    fprintf(fp, "VPULSEH %i\n", value);
    ReadRegister(0x606, value);
    fprintf(fp, "VPULSEL %i\n", value);
    ReadRegister(0x607, value);
    fprintf(fp, "VCASN2  %i\n", value);
    ReadRegister(0x608, value);
    fprintf(fp, "VCLIP   %i\n", value);
    ReadRegister(0x609, value);
    fprintf(fp, "VTEMP   %i\n", value);
    ReadRegister(0x60a, value);
    fprintf(fp, "IAUX2   %i\n", value);
    ReadRegister(0x60b, value);
    fprintf(fp, "IRESET  %i\n", value);
    ReadRegister(0x60c, value);
    fprintf(fp, "IDB     %i\n", value);
    ReadRegister(0x60d, value);
    fprintf(fp, "IBIAS   %i\n", value);
    ReadRegister(0x60e, value);
    fprintf(fp, "ITHR    %i\n", value);

    fprintf(fp, "\n");
    // Mode control register
    ReadRegister( 0x1, value );
    fprintf( fp, "MODECONTROL  %i\n", value );
    
    // FROMU config reg 1: [5]: test pulse mode; [6]: enable test strobe, etc.
    ReadRegister( 0x4, value );
    fprintf( fp, "FROMU_CONFIG1  %i\n", value );
    
    // FROMU config reg 2: strobe duration
    ReadRegister( 0x5, value );
    fprintf( fp, "FROMU_CONFIG2  %i\n", value );

    // FROMU pulsing reg 1: delay between pulse and strobe if the feature of automatic strobing is enabled
    ReadRegister( 0x7, value );
    fprintf( fp, "FROMU_PULSING1  %i\n", value );

    // FROMU pulsing reg 2: pulse duration
    ReadRegister( 0x8, value );
    fprintf( fp, "FROMU_PULSING2  %i\n", value );

    // CMU DMU config reg
    ReadRegister( 0x10, value );
    fprintf( fp, "CMUDMU_CONFIG  %i\n", value );

    fclose( fp );
  }

  config[0] = '\0';
  // DACs
  ReadRegister( 0x601, value );
  sprintf( config, "VRESETP %i\n", value );
  ReadRegister( 0x602, value );
  sprintf( config, "%sVRESETD %i\n", config, value );
  ReadRegister( 0x603, value );
  sprintf( config, "%sVCASP   %i\n", config, value );
  ReadRegister( 0x604, value );
  sprintf( config, "%sVCASN   %i\n", config, value );
  ReadRegister( 0x605, value );
  sprintf( config, "%sVPULSEH %i\n", config, value );
  ReadRegister(0x606, value);
  sprintf( config, "%sVPULSEL %i\n", config, value );
  ReadRegister( 0x607, value );
  sprintf( config, "%sVCASN2  %i\n", config, value );
  ReadRegister( 0x608, value );
  sprintf( config, "%sVCLIP   %i\n", config, value );
  ReadRegister( 0x609, value );
  sprintf( config, "%sVTEMP   %i\n", config, value );
  ReadRegister( 0x60a, value );
  sprintf( config, "%sIAUX2   %i\n", config, value );
  ReadRegister( 0x60b, value );
  sprintf( config, "%sIRESET  %i\n", config, value );
  ReadRegister( 0x60c, value );
  sprintf( config, "%sIDB     %i\n", config, value );
  ReadRegister( 0x60d, value );
  sprintf( config, "%sIBIAS   %i\n", config, value );
  ReadRegister( 0x60e, value );
  sprintf( config, "%sITHR    %i\n", config, value );

  sprintf( config, "%s\n", config );
  // Mode control register
  ReadRegister( 0x1, value );
  sprintf( config, "%sMODECONTROL  %i\n", config, value );
  
  // FROMU config reg 1: [5]: test pulse mode; [6]: enable test strobe, etc.
  ReadRegister( 0x4, value );
  sprintf( config, "%sFROMU_CONFIG1  %i\n", config, value );
  
  // FROMU config reg 2: strobe duration
  ReadRegister( 0x5, value );
  sprintf( config, "%sFROMU_CONFIG2  %i\n", config, value );

  // FROMU pulsing reg 1: delay between pulse and strobe if the feature of automatic strobing is enabled
  ReadRegister( 0x7, value );
  sprintf( config, "%sFROMU_PULSING1  %i\n", config, value );

  // FROMU pulsing reg 2: pulse duration
  ReadRegister( 0x8, value );
  sprintf( config, "%sFROMU_PULSING2  %i\n", config, value );

  // CMU DMU config reg
  ReadRegister( 0x10, value );
  sprintf( config, "%sCMUDMU_CONFIG  %i\n", config, value );

}

#pragma mark - operations with ADC or DAC

//___________________________________________________________________
float TAlpide::ReadTemperature()
{
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( !spBoard ) {
        throw runtime_error( "TAlpide::ReadTemperature() - unuseable readout board." );
    }

    uint16_t theResult = 0;
    if (fADCBias == -1) { // needs calibration
        CalibrateADC();
    }
    
    SetTheDacMonitor( AlpideRegister::ANALOGMON ); // uses the RE_ANALOGMON, in order to disable the monitoring !
    usleep(5000);
    SetTheADCCtrlRegister( ADCMode::MANUAL, ADCInput::Temperature, ADCComparator::COMP_296uA, ADCRampSpeed::RAMP_1us );
    spBoard->SendOpCode( (uint16_t)AlpideOpCode::ADCMEASURE,  (uint8_t)fChipId );
    usleep(5000); // Wait for the measurement > of 5 milli sec
    ReadRegister( AlpideRegister::ADC_AVSS, theResult );
    theResult -=  (uint16_t)fADCBias;
    float theValue =  ( ((float)theResult) * 0.1281) + 6.8; // first approximation
    return theValue;
}

//___________________________________________________________________
float TAlpide::ReadDACVoltage( AlpideRegister ADac )
{
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( !spBoard ) {
        throw runtime_error( "TAlpide::ReadDACVoltage() - unuseable readout board." );
    }

    uint16_t theResult = 0;
    if (fADCBias == -1) { // needs calibration
        CalibrateADC();
    }
    
    SetTheDacMonitor( ADac );
    usleep(5000);
    SetTheADCCtrlRegister( ADCMode::MANUAL, ADCInput::DACMONV, ADCComparator::COMP_296uA, ADCRampSpeed::RAMP_1us );
    spBoard->SendOpCode( (uint16_t)AlpideOpCode::ADCMEASURE, (uint8_t)fChipId );
    usleep(5000); // Wait for the measurement > of 5 milli sec
    ReadRegister( AlpideRegister::ADC_AVSS, theResult );
    theResult -=  (uint16_t)fADCBias;
    float theValue =  ( ((float)theResult) * 0.001644); // V scale first approximation
    return theValue;
}

//___________________________________________________________________
float TAlpide::ReadDACCurrent( AlpideRegister ADac )
{
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( !spBoard ) {
        throw runtime_error( "TAlpide::ReadDACCurrent() - unuseable readout board." );
    }

    uint16_t theResult = 0;
    if (fADCBias == -1) { // needs calibration
        CalibrateADC();
    }
    
    SetTheDacMonitor( ADac );
    usleep(5000);
    SetTheADCCtrlRegister( ADCMode::MANUAL, ADCInput::DACMONI, ADCComparator::COMP_296uA, ADCRampSpeed::RAMP_1us );
    spBoard->SendOpCode( (uint16_t)AlpideOpCode::ADCMEASURE,  (uint8_t)fChipId );
    usleep(5000); // Wait for the measurement > of 5 milli sec
    ReadRegister( AlpideRegister::ADC_AVSS, theResult );
    theResult -= (uint16_t)fADCBias;
    float theValue =  ( ((float)theResult) * 0.164); // uA scale   first approximation
    return theValue;
}

#pragma mark - chip configuration operations

//___________________________________________________________________
void TAlpide::Init()
{
    ClearPixSelectBits(true);
}

//___________________________________________________________________
void TAlpide::WritePixConfReg( AlpidePixReg reg, const bool data )
{
    uint16_t pixconfig = (int) reg & 0x1;
    pixconfig         |= (data?1:0) << 1;
    WriteRegister( AlpideRegister::PIXELCONFIG, pixconfig );
}

//___________________________________________________________________
void TAlpide::WritePixRegAll( AlpidePixReg reg, const bool data )
{
    // TODO: To be checked whether this methods works or whether a loop over rows has to be implemented

    WritePixConfReg( reg, data );
    
    // set all colsel and all rowsel to 1
    WriteRegister( 0x487, 0xffff );

    ClearPixSelectBits( false );
}

//___________________________________________________________________
void TAlpide::WritePixRegRow( AlpidePixReg reg, const bool data, const int row)
{
    WritePixConfReg( reg, data);
    // set all colsel to 1 and leave all rowsel at 0
    WriteRegister( 0x483, 0xffff );
    
    // for correct region set one rowsel to 1
    int region = row / 16;
    int bit    = row % 16;
    
    int address = 0x404 | (region << 11);
    int value   = 1 << bit;
    
    WriteRegister( address, value );
    
    ClearPixSelectBits( false );
}

//___________________________________________________________________
void TAlpide::WritePixRegSingle( AlpidePixReg reg,
                                     const bool data,
                                     const int row,
                                     const int col )
{
    WritePixConfReg( reg, data );
    
    // set correct colsel bit
    int region  = col / 32;            // region that contains the corresponding col select
    int bit     = col % 16;            // bit that has to be written (0 - 15)
    int highlow = (col % 32) / 16;     // decide between col <15:0> (0) and col <31:16> (1)
    
    int address = 0x400 | (region << 11) | (1 << highlow);
    int value   = 1 << bit;
    
    WriteRegister( address, value );
    
    // set correct rowsel bit
    region = row / 16;
    bit    = row % 16;
    
    address = 0x404 | (region << 11);
    value   = 1 << bit;
    
    WriteRegister( address, value );
    
    ClearPixSelectBits( false );
}

//___________________________________________________________________
void TAlpide::ApplyStandardDACSettings( const float backBias )
{
    // TODO: pAlpide 3 settings, to be confirmed
    if ( backBias == 0 ) {
        WriteRegister( AlpideRegister::VCASN,    60 );
        WriteRegister( AlpideRegister::VCASN2,   62 );
        WriteRegister( AlpideRegister::VRESETD, 147 );
        WriteRegister( AlpideRegister::IDB,      29 );
    } else if ( backBias == 3 ) {
        WriteRegister( AlpideRegister::VCASN,   105 );
        WriteRegister( AlpideRegister::VCASN2,  117 );
        WriteRegister( AlpideRegister::VCLIP,    60 );
        WriteRegister( AlpideRegister::VRESETD, 147 );
        WriteRegister( AlpideRegister::IDB,      29 );
    } else if ( backBias == 6 ) {
        WriteRegister( AlpideRegister::VCASN,   135 );
        WriteRegister( AlpideRegister::VCASN2,  147 );
        WriteRegister( AlpideRegister::VCLIP,   100 );
        WriteRegister( AlpideRegister::VRESETD, 170 );
        WriteRegister( AlpideRegister::IDB,      29 );
    } else {
        cout << "TAlpide::ApplyStandardDACSettings() - Settings not defined for back bias " << backBias << " V. Please set manually." << endl;
    }
}

//___________________________________________________________________
void TAlpide::ConfigureFromu( const AlpidePulseType pulseType,
                              const bool testStrobe )
{
    // MARK: for the time being use these hard coded values; if needed move to configuration
    int  mebmask          = 0;
    bool rotatePulseLines = false;
    bool internalStrobe   = false;    // strobe sequencer for continuous mode
    bool busyMonitoring   = true;
    
    
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureFromu() - chip config. not found!" );
    }
    
    uint16_t fromuconfig = 0;
    
    fromuconfig |= mebmask;
    fromuconfig |= (internalStrobe   ? 1:0)          << 3;
    fromuconfig |= (busyMonitoring   ? 1:0)          << 4;
    fromuconfig |= ((int) pulseType)                 << 5;
    fromuconfig |= (testStrobe       ? 1:0)          << 6;
    fromuconfig |= (rotatePulseLines ? 1:0)          << 7;
    fromuconfig |= (spConfig->GetTriggerDelay() & 0x7) << 8;
    
    WriteRegister( AlpideRegister::FROMU_CONFIG1,  fromuconfig );
    WriteRegister( AlpideRegister::FROMU_CONFIG2,  spConfig->GetStrobeDuration() );
    WriteRegister( AlpideRegister::FROMU_PULSING1, spConfig->GetStrobeDelay() );
    WriteRegister( AlpideRegister::FROMU_PULSING2, spConfig->GetPulseDuration() );
}

// Simpler configuration for threshold scan
//___________________________________________________________________
void TAlpide::ConfigureFromu()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureFromu() - chip config. not found!" );
    }
    // fromu config 1: digital pulsing (put to 0x20 for analogue)
    WriteRegister( AlpideRegister::FROMU_CONFIG1,  0x20 );
    // fromu config 2: strobe length
    WriteRegister( AlpideRegister::FROMU_CONFIG2,  spConfig->GetStrobeDuration() );
    // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
    WriteRegister( AlpideRegister::FROMU_PULSING1, spConfig->GetStrobeDelay() );
    // fromu pulsing 2: pulse length
    WriteRegister( AlpideRegister::FROMU_PULSING2, spConfig->GetPulseDuration() );
}

//___________________________________________________________________
void TAlpide::ConfigureBuffers()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureBuffers() - chip config. not found!" );
    }

    uint16_t clocks = 0, ctrl = 0;
    
    clocks |= (spConfig->GetDclkReceiver () & 0xf);
    clocks |= (spConfig->GetDclkDriver   () & 0xf) << 4;
    clocks |= (spConfig->GetMclkReceiver () & 0xf) << 8;
    
    ctrl   |= (spConfig->GetDctrlReceiver() & 0xf);
    ctrl   |= (spConfig->GetDctrlDriver  () & 0xf) << 4;
    
    WriteRegister( AlpideRegister::CLKIO_DACS, clocks );
    WriteRegister( AlpideRegister::CMUIO_DACS, ctrl );
}

//___________________________________________________________________
void TAlpide::ConfigureCMU()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureCMU() - chip config. not found!" );
    }
    
    uint16_t cmuconfig = 0;
    
    cmuconfig |= (spConfig->GetPreviousId() & 0xf);
    cmuconfig |= (spConfig->GetInitialToken     () ? 1:0) << 4;
    cmuconfig |= (spConfig->GetDisableManchester() ? 1:0) << 5;
    cmuconfig |= (spConfig->GetEnableDdr        () ? 1:0) << 6;
    
    WriteRegister( AlpideRegister::CMUDMU_CONFIG, cmuconfig );
}

//___________________________________________________________________
int TAlpide::ConfigureMaskStage( int nPix, const int iStage )
{
    // check that nPix is one of (1, 2, 4, 8, 16, 32)
    if ((nPix <= 0) || (nPix & (nPix - 1)) || (nPix > 32)) {
        cout << "TAlpide::ConfigureMaskStage() - Warning: bad number of pixels for mask stage (" << nPix << ", using 1 instead" << endl;
        nPix = 1;
    }
    WritePixRegAll( AlpidePixReg::MASK,   true );
    WritePixRegAll( AlpidePixReg::SELECT, false );
    
    // complete row
    if ( nPix == 32 ) {
        WritePixRegRow( AlpidePixReg::MASK,   false, iStage );
        WritePixRegRow( AlpidePixReg::SELECT, true, iStage );
        return iStage;
    } else {
        int colStep = 32 / nPix;
        for ( int icol = 0; icol < 1024; icol += colStep ) {
            WritePixRegSingle( AlpidePixReg::MASK,   false, iStage % 512, icol + iStage / 512);
            WritePixRegSingle( AlpidePixReg::SELECT, true,  iStage % 512, icol + iStage / 512);
        }
        return (iStage % 512);
    }
}


//___________________________________________________________________
void TAlpide::WriteControlReg( const AlpideChipMode chipMode )
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::WriteControlReg() - chip config. not found!" );
    }

    uint16_t controlreg = 0;
    
    controlreg |= (uint16_t) chipMode;
    
    controlreg |= (spConfig->GetEnableClustering    () ? 1:0) << 2;
    controlreg |= (spConfig->GetMatrixReadoutSpeed  () & 0x1) << 3;
    controlreg |= (spConfig->GetSerialLinkSpeed     () & 0x3) << 4;
    controlreg |= (spConfig->GetEnableSkewingGlobal () ? 1:0) << 6;
    controlreg |= (spConfig->GetEnableSkewingStartRO() ? 1:0) << 7;
    controlreg |= (spConfig->GetEnableClockGating   () ? 1:0) << 8;
    controlreg |= (spConfig->GetEnableCMUReadout    () ? 1:0) << 9;
    
    WriteRegister( AlpideRegister::MODECONTROL, controlreg);
}

//___________________________________________________________________
void TAlpide::BaseConfigPLL()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::BaseConfigPLL() - chip config. not found!" );
    }

    if ( spConfig->GetParamValue("LINKSPEED") == -1 ) return; // high-speed link deactivated
    
    uint16_t Phase      = 8;  // 4bit Value, default 8
    uint16_t Stages     = 1; // 0 = 3 stages, 1 = 4,  3 = 5 (typical 4)
    uint16_t ChargePump = 8;
    uint16_t Driver     = 15;
    uint16_t Preemp     = 15;
    uint16_t Value;
    
    Value = (Stages & 0x3) | 0x4 | 0x8 | ((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x8: PLL off
    
    WriteRegister( AlpideRegister::DTU_CONFIG, Value );
    
    Value = (ChargePump & 0xf) | ((Driver & 0xf) << 4) | ((Preemp & 0xf) << 8);
    
    WriteRegister( AlpideRegister::DTU_DACS, Value );
    
    // Clear PLL off signal
    Value = (Stages & 0x3) | 0x4 | ((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x8: PLL off
    WriteRegister( AlpideRegister::DTU_CONFIG, Value );
    // Force PLL reset
    Value = (Stages & 0x3) | 0x4 | 0x100 |((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x100: Reset
    WriteRegister( AlpideRegister::DTU_CONFIG, Value );
    Value = (Stages & 0x3) | 0x4 |((Phase & 0xf) << 4);           // Reset off
    WriteRegister( AlpideRegister::DTU_CONFIG, Value );
}

//___________________________________________________________________
void TAlpide::BaseConfigMask()
{
    WritePixRegAll( AlpidePixReg::MASK,   true );
    WritePixRegAll( AlpidePixReg::SELECT, false );
}

//___________________________________________________________________
void TAlpide::BaseConfigFromu()
{
    // FIXME: not implemeted yet
    cout << "TAlpide::BaseConfigFromu() - NOT IMPLEMENTED YET" << endl;
}

//___________________________________________________________________
void TAlpide::BaseConfigDACs()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::BaseConfigDACs() - chip config. not found!" );
    }
 
    WriteRegister( AlpideRegister::VPULSEH, spConfig->GetParamValue("VPULSEH"));
    WriteRegister( AlpideRegister::VPULSEL, spConfig->GetParamValue("VPULSEL"));
    WriteRegister( AlpideRegister::VRESETD, spConfig->GetParamValue("VRESETD"));
    WriteRegister( AlpideRegister::VCASN,   spConfig->GetParamValue("VCASN"));
    WriteRegister( AlpideRegister::VCASN2,  spConfig->GetParamValue("VCASN2"));
    WriteRegister( AlpideRegister::VCLIP,   spConfig->GetParamValue("VCLIP"));
    WriteRegister( AlpideRegister::ITHR,    spConfig->GetParamValue("ITHR"));
    WriteRegister( AlpideRegister::IDB,     spConfig->GetParamValue("IDB"));
    WriteRegister( AlpideRegister::IBIAS,   spConfig->GetParamValue("IBIAS"));
    WriteRegister( AlpideRegister::VCASP,   spConfig->GetParamValue("VCASP"));
    // not used DACs..
    WriteRegister( AlpideRegister::VTEMP,   spConfig->GetParamValue("VTEMP"));
    WriteRegister( AlpideRegister::VRESETP, spConfig->GetParamValue("VRESETP"));
    WriteRegister( AlpideRegister::IRESET,  spConfig->GetParamValue("IRESET"));
    WriteRegister( AlpideRegister::IAUX2,   spConfig->GetParamValue("IAUX2"));
}

//___________________________________________________________________
void TAlpide::BaseConfig()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::BaseConfig() - chip config. not found!" );
    }

    // put all chip configurations before the start of the test here
    
    WriteRegister( AlpideRegister::MODECONTROL, 0x20 ); // set chip to config mode
    //TODO: use chip config here, the config should be written accordingly at this point!
    
    
    // CMU/DMU config: turn manchester encoding off or on etc, initial token=1, disable DDR
    int cmudmu_config = 0x10 | ((spConfig->GetDisableManchester()) ? 0x20 : 0x00);
    
    BaseConfigFromu();
    BaseConfigDACs();
    BaseConfigMask();
    BaseConfigPLL();
    
    uint16_t value;

    switch (spConfig->GetParamValue("LINKSPEED")) {
        case -1: // DTU not activated
            value = 0x21;
            break;
        case 400:
            value = 0x01;
            break;
        case 600:
            value = 0x11;
            break;
        case 1200:
            value = 0x21;
            break;
        default:
            cout << "TAlpide::BaseConfig() - Warning: invalid link speed, using 1200" << endl;
            value = 0x21;
            break;
    }
    
    WriteRegister( AlpideRegister::MODECONTROL, value ); // strobed readout mode
}


//___________________________________________________________________
void TAlpide::PrintDebugStream()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::PrintDebugStream() - chip config. not found!" );
    }

    uint16_t Value;

    cout << "TAlpide::PrintDebugStream() - start" << endl;
    cout << "Debug Stream chip id " << spConfig->GetChipId() << ": " << endl;
    
    for (int i = 0; i < 2; i++) {
        ReadRegister( AlpideRegister::BMU_DEBUG, Value );
        cout << "  BMU Debug reg word " << i << ": " << std::hex << Value << std::dec << endl;
    }
    for (int i = 0; i < 4; i++) {
        ReadRegister( AlpideRegister::DMU_DEBUG, Value );
        cout << "  DMU Debug reg word " << i << ": " << std::hex << Value << std::dec << endl;
    }
    for (int i = 0; i < 9; i++) {
        ReadRegister( AlpideRegister::FROMU_DEBUG, Value );
        cout << "  FROMU Debug reg word " << i << ": " << std::hex << Value << std::dec << endl;
    }
    cout << "TAlpide::PrintDebugStream() - end" << endl;
}

#pragma mark - needed to operate with ADC or DAC

//___________________________________________________________________
void TAlpide::CalibrateADC()
{
    uint16_t theVal2,theVal1;
    //	bool isAVoltDAC, isACurrDAC, isATemperature, isAVoltageBuffered;
    //	int theSelInput;
    
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( !spBoard ) {
        throw runtime_error( "TAlpide::CalibrateADC() - unuseable readout board." );
    }
    // Calibration Phase 1
    fADCHalfLSB = false;
    fADCSign = false;
    SetTheADCCtrlRegister( ADCMode::CALIBRATE , ADCInput::AVSS, ADCComparator::COMP_296uA, ADCRampSpeed::RAMP_1us );
    spBoard->SendOpCode ( (uint16_t)AlpideOpCode::ADCMEASURE, (uint8_t)fChipId );
    usleep(4000); // > of 5 milli sec
    ReadRegister( AlpideRegister::ADC_CALIB, theVal1 );
    fADCSign = true;
    SetTheADCCtrlRegister( ADCMode::CALIBRATE , ADCInput::AVSS, ADCComparator::COMP_296uA, ADCRampSpeed::RAMP_1us );
    spBoard->SendOpCode( (uint16_t)AlpideOpCode::ADCMEASURE, (uint8_t)fChipId );
    usleep(4000); // > of 5 milli sec
    ReadRegister( AlpideRegister::ADC_CALIB, theVal2 );
    fADCSign =  (theVal1 > theVal2) ? false : true;
    
    // Calibration Phase 2
    fADCHalfLSB = false;
    SetTheADCCtrlRegister( ADCMode::CALIBRATE , ADCInput::AVSS, ADCComparator::COMP_296uA, ADCRampSpeed::RAMP_1us );
    spBoard->SendOpCode ( (uint16_t)AlpideOpCode::ADCMEASURE, (uint8_t)fChipId );
    usleep(4000); // > of 5 milli sec
    ReadRegister( AlpideRegister::ADC_CALIB, theVal1 );
    fADCHalfLSB = true;
    SetTheADCCtrlRegister( ADCMode::CALIBRATE , ADCInput::AVSS, ADCComparator::COMP_296uA, ADCRampSpeed::RAMP_1us );
    spBoard->SendOpCode( (uint16_t)AlpideOpCode::ADCMEASURE, (uint8_t)fChipId );
    usleep(4000); // > of 5 milli sec
    ReadRegister( AlpideRegister::ADC_CALIB, theVal2 );
    fADCHalfLSB =  (theVal1 > theVal2) ? false : true;
    
    // Detect the Bias
    SetTheADCCtrlRegister( ADCMode::CALIBRATE , ADCInput::AVSS, ADCComparator::COMP_296uA, ADCRampSpeed::RAMP_1us );
    spBoard->SendOpCode( (uint16_t)AlpideOpCode::ADCMEASURE, (uint8_t)fChipId );
    usleep(4000); // > of 5 milli sec
    ReadRegister( AlpideRegister::ADC_CALIB,theVal1 );
    fADCBias = theVal1;
}

//___________________________________________________________________
uint16_t TAlpide::SetTheADCCtrlRegister( ADCMode Mode,
										ADCInput SelectInput,
										ADCComparator ComparatorCurrent,
										ADCRampSpeed RampSpeed )
{
	uint16_t Data;
	Data = Mode | (SelectInput<<2) | (ComparatorCurrent<<6) | (fADCSign<<8) | (RampSpeed<<9) | (fADCHalfLSB<<11);
	WriteRegister( AlpideRegister::ADC_CONTROL, Data );
	return Data;
}

//___________________________________________________________________
void TAlpide::SetTheDacMonitor( AlpideRegister ADac, DACMonIref IRef )
{
	int VDAC, IDAC;
	uint16_t Value;
	switch (ADac) {
        case AlpideRegister::VRESETP:
            VDAC = 4;
            IDAC = 0;
            break;
        case AlpideRegister::VRESETD:
            VDAC = 5;
            IDAC = 0;
            break;
        case AlpideRegister::VCASP:
            VDAC = 1;
            IDAC = 0;
            break;
        case AlpideRegister::VCASN:
            VDAC = 0;
            IDAC = 0;
            break;
        case AlpideRegister::VPULSEH:
            VDAC = 2;
            IDAC = 0;
            break;
        case AlpideRegister::VPULSEL:
            VDAC = 3;
            IDAC = 0;
            break;
        case AlpideRegister::VCASN2:
            VDAC = 6;
            IDAC = 0;
            break;
        case AlpideRegister::VCLIP:
            VDAC = 7;
            IDAC = 0;
            break;
        case AlpideRegister::VTEMP:
            VDAC = 8;
            IDAC = 0;
            break;
        case AlpideRegister::IAUX2:
            IDAC = 1;
            VDAC = 0;
            break;
        case AlpideRegister::IRESET:
            IDAC = 0;
            VDAC = 0;
            break;
        case AlpideRegister::IDB:
            IDAC = 3;
            VDAC = 0;
            break;
        case AlpideRegister::IBIAS:
            IDAC = 2;
            VDAC = 0;
            break;
        case AlpideRegister::ITHR:
            IDAC = 5;
            VDAC = 0;
            break;
        default:
            VDAC = 0;
            IDAC = 0;
            break;
    }

	Value = VDAC & 0xf;
	Value |= (IDAC & 0x7) << 4;
	Value |= (IRef & 0x3) << 9;

	WriteRegister( AlpideRegister::ANALOGMON, Value );
	return;
}

#pragma mark - needed for chip config. operations

//___________________________________________________________________
void TAlpide::ClearPixSelectBits( const bool clearPulseGating)
{
    if ( clearPulseGating )
        WriteRegister( 0x48f, 0 );
    else
        WriteRegister( 0x487, 0 );
}




