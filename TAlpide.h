#ifndef ALPIDE_H
#define ALPIDE_H

#include <unistd.h>
#include <cstdint>
#include <memory>
#include "AlpideDictionary.h"

class TChipConfig;
class TReadoutBoard;

class TAlpide {
    
 private:
    int fChipId;

    // ADC calibration parameters
    int fADCBias; ///< ADC calibration parameter.
    bool fADCHalfLSB; ///< ADC calibration parameter.
    bool fADCSign; ///< ADC calibration parameter.

    std::weak_ptr<TChipConfig> fConfig;
    std::weak_ptr<TReadoutBoard> fReadoutBoard;
    
    enum DACMonIref {
        IREF_025uA = 0,
        IREF_075uA = 1,
        IREF_100uA = 2,
        IREF_125uA = 3
    };
    enum ADCMode {
        MANUAL      = 0,
        CALIBRATE   = 1,
        AUTO        = 2,
        SUPERMANUAL = 3
    };
    enum ADCComparator {
        COMP_180uA = 0,
        COMP_190uA = 1,
        COMP_296uA = 2,
        COMP_410uA = 3
    };
    enum ADCRampSpeed {
        RAMP_500ms = 0,
        RAMP_1us = 1,
        RAMP_2us = 2,
        RAMP_4us = 3
    };
    enum ADCInput {
        AVSS = 0,
        DVSS = 1,
        AVDD = 2,
        DVDD = 3,
        VBGthVolScal = 4,
        DACMONV = 5,
        DACMONI = 6,
        Bandgap = 7,
        Temperature = 8
    };

 public:
    
    #pragma mark - Constructors/destructor
    // ctor
    TAlpide();
    TAlpide( std::shared_ptr<TChipConfig> config );
    TAlpide( std::shared_ptr<TChipConfig> config,
             std::shared_ptr<TReadoutBoard> readoutBoard );
    // dtor
    virtual ~TAlpide();

    #pragma mark - setters/getters
    // setter
    void SetReadoutBoard( std::shared_ptr<TReadoutBoard> readoutBoard ) { fReadoutBoard = readoutBoard; }
    void SetEnable( bool Enable );
    // getter
    std::weak_ptr<TChipConfig> GetConfig() { return fConfig; }
    std::weak_ptr<TReadoutBoard> GetReadoutBoard() { return fReadoutBoard; }
    int GetADCBias() const { return fADCBias; }

    #pragma mark - basic operations with registers
    int  ReadRegister( const AlpideRegister address, std::uint16_t &value );
    int  ReadRegister( const std::uint16_t address, std::uint16_t &value );
    int  WriteRegister( const AlpideRegister address,
                        std::uint16_t value,
                        const bool verify = false );
    int WriteRegister( const std::uint16_t address,
                        std::uint16_t value,
                        const bool verify = false );
    int ModifyRegisterBits( const AlpideRegister address,
                            const std::uint8_t lowBit,
                            const std::uint8_t nBits, std::uint16_t value,
                            const bool verify = false );
    #pragma mark - dump
    void DumpConfig( const char *fName, const bool writeFile=true, char *Config = 0 );

    #pragma mark - operations with ADC or DAC
    /// Reads the temperature sensor by means of internal ADC.
    /**
     Returns the value in Celsius degree.
     - Note:
     
       if this was the first measure after the chip configuration phase, a calibration
       will be automatically executed.
     */
    float ReadTemperature();
    
    /// Reads the output voltage of one DAC by means of internal ADC.
    /**
     \param ADac the Index that define the DAC register
     Returns the value in Volts.
     - Note:
     
     if this was the first measure after the chip configuration phase, a calibration
     will be automatically executed.
     */
    float ReadDACVoltage( AlpideRegister ADac );
    
    /// Reads the output current of one DAC by means of internal ADC.
    /**
     \param ADac the Index that define the DAC register
     Returns the value in Micro Ampere.
     - Note:
     
     if this was the first measure after the chip configuration phase, a calibration
     will be automatically executed.
     */
    float ReadDACCurrent( AlpideRegister ADac );
    
    #pragma mark - chip configuration operations
    void Init();
    void WritePixConfReg( AlpidePixReg reg, const bool data);

    /// This method writes data to the selected pixel register in the whole matrix simultaneously.
    void WritePixRegAll( AlpidePixReg reg, const bool data);

    /// Writes data to complete row. This assumes that select bits have been cleared before.
    void WritePixRegRow( AlpidePixReg reg, const bool data, const int row);
    
    void WritePixRegSingle( AlpidePixReg reg, const bool data,
                            const int row, const int col);
    void ApplyStandardDACSettings( const float backBias );
    void ConfigureFromu( const AlpidePulseType pulseType,
                         const bool testStrobe );
    void ConfigureFromu();
    void ConfigureBuffers();
    void ConfigureCMU();

    /// return value: active row (needed for threshold scan histogramming).
    int  ConfigureMaskStage( int nPix, const int iStage );
    
    void WriteControlReg( const AlpideChipMode chipMode );
    void BaseConfigPLL();
    void BaseConfigMask();
    void BaseConfigFromu();
    void BaseConfigDACs();
    void BaseConfig();
    void PrintDebugStream();

private:
    
    #pragma mark - needed to operate with ADC or DAC
    /// Calibrate the internal ADC.
    /**
     Compute the value of the Bias.
     - Note:
     
     the calibration parameter are stored into the devoted class members.
     */
    void CalibrateADC();

    /// Set the ADC Control Register.
    /**
     \param Mode Mode of ADC measurement [0:Manual 1:Calibrate 2:Auto 3:SupoerManual]
     \param SelectInput the source specification [0:AVSS 1:DVSS 2:AVDD 3:DVDD 4:VBGthVolScal 5:DACMONV 6:DACMONI 7:Bandgap 8:Temperature]
     \param ComparatorCurrent [0:180uA 1:190uA 2:296uA 3:410uA]
     \param RampSpeed [0:500ms 1:1us 2:2us 3:4us]
     */
    uint16_t SetTheADCCtrlRegister( ADCMode Mode,
                                    ADCInput SelectInput,
                                    ADCComparator ComparatorCurrent,
                                    ADCRampSpeed RampSpeed );

    /// Sets the DAC Monitor multiplexer.
    /**
     \param ADac the Index of the DAC register
     \param IRef the IRef value [Iref =  0:0.25ua 1:0.75uA 2:1.00uA 3:1.25uA]
     */
    void SetTheDacMonitor( AlpideRegister ADac,
                            DACMonIref IRef = DACMonIref::IREF_100uA );
    
    #pragma mark - needed for chip config. operations
    /// Clear all column and row select bits.
    /**
     \param clearPulseGating If set, the pulse gating registers will also be reset
     (possibly useful at startup, but not in-between setting of mask patterns).
     */
    void ClearPixSelectBits( const bool clearPulseGating );

};


#endif  /* ALPIDE_H */
