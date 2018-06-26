#ifndef ALPIDE_H
#define ALPIDE_H

#include <unistd.h>
#include <cstdint>
#include <memory>
#include <string>
#include "TVerbosity.h"
#include "AlpideDictionary.h"

class TChipConfig;
class TReadoutBoard;

class TAlpide : public TVerbosity {
    
 private:
    int fChipId;

    // ADC calibration parameters
    int fADCOffset; ///< ADC calibration parameter.
    bool fADCHalfLSB; ///< ADC calibration parameter.
    bool fADCSign; ///< ADC calibration parameter.

    std::weak_ptr<TChipConfig> fConfig;
    std::weak_ptr<TReadoutBoard> fReadoutBoard;
    
    static const char* fRegName[];
    static const char* fDACsRegName[];

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
    virtual void SetVerboseLevel( const int level );
    // getter
    std::weak_ptr<TChipConfig> GetConfig() { return fConfig; }
    std::weak_ptr<TReadoutBoard> GetReadoutBoard() { return fReadoutBoard; }
    int GetADCOffset() const { return fADCOffset; }
    int GetChipId() const { return fChipId; }

    #pragma mark - dump
    
    void DumpConfig( const char* fileName, const bool writeFile=true, char* config = 0 );
    
    void DumpConfig();

    #pragma mark - basic operations with registers
    
    void ReadRegister( const AlpideRegister address,
                      std::uint16_t& value,
                      const bool doExecute = true,
                      const bool skipDisabledChip = true );
    void ReadRegister( const std::uint16_t address,
                      std::uint16_t& value,
                      const bool doExecute = true,
                      const bool skipDisabledChip = true );

    void WriteRegister( const AlpideRegister address,
                       std::uint16_t value,
                       const bool doExecute = true,
                       const bool verify = false,
                       const bool skipDisabledChip = true );
    void WriteRegister( const std::uint16_t address,
                       std::uint16_t value,
                       const bool doExecute = true,
                       const bool verify = false,
                       const bool skipDisabledChip = true );
    
    void ModifyRegisterBits( const AlpideRegister address,
                            const std::uint8_t lowBit,
                            const std::uint8_t nBits, std::uint16_t value,
                            const bool verify = false,
                            const bool skipDisabledChip = true );
    

    #pragma mark - chip configuration operations

    void Init();
    
    void WritePixConfReg( AlpidePixConfigReg reg, const bool data);

    /// This method writes data to the selected pixel register in the whole matrix simultaneously.
    void WritePixRegAll( AlpidePixConfigReg reg, const bool data);

    /// Writes data to complete row. This assumes that select bits have been cleared before.
    void WritePixRegRow( AlpidePixConfigReg reg, const bool data, const int row);
    
    void WritePixRegSingle( AlpidePixConfigReg reg, const bool data,
                            const int row, const int col);
    void ApplyStandardDACSettings( const float backBias );
    void ConfigureBuffers();
    
    /// Put chip in configuration mode (readout disabled)
    void ActivateConfigMode();
    
    /// Put chip in readout mode
    void ActivateReadoutMode();

    /// Setting up of readout from config file - CMU part
    void ConfigureCMU();
    
    /// Setting up of DTU test register 1 (DTU_TEST1) bits
    void ConfigureDTU_TEST1();

    /// Setting up of readout from config file - FROMU part
    void ConfigureFROMU();

    /// Return value: active row (needed for threshold scan histogramming).
    int  ConfigureMaskStage( int nPix, const int iStage );
    
    /// Write the bits in the Mode Control Register
    void WriteControlReg( const AlpideChipMode chipMode );
    
    /// Configuration and start-up of the DTU
    void BaseConfigPLL();
    
    /// For analog pulse, configure VPULSE low to be at a given deltaV w.r.t. VPULSE high
    void ConfigureVPulseLow( const unsigned int deltaV );
    
    void BaseConfigMask();
    void BaseConfigDACs();
    void BaseConfig();
    void PrintDebugStream();
    
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
    uint16_t SetTheADCCtrlRegister( AlpideADCMode Mode,
                                    AlpideADCInput SelectInput,
                                    AlpideADCComparator ComparatorCurrent,
                                    AlpideADCRampSpeed RampSpeed );

    /// Sets the DAC Monitor multiplexer.
    /**
     \param ADac the Index of the DAC register
     \param IRef the IRef value [Iref =  0:0.25ua 1:0.75uA 2:1.00uA 3:1.25uA]
     */
    void SetTheDacMonitor( AlpideRegister ADac,
                           AlpideDACMonIref IRef = AlpideDACMonIref::IREF_100uA );
    
    #pragma mark - needed for chip config. operations
    
    /// Clear all column and row select bits.
    /**
     \param clearPulseGating If set, the pulse gating registers will also be reset
     (possibly useful at startup, but not in-between setting of mask patterns).
     */
    void ClearPixSelectBits( const bool clearPulseGating );
    
    #pragma mark - other
    
    /// Enable the readout of all double columns (Priority Encoders) of all regions
    void EnableDoubleColumns();
    
    std::string DecomposeChipId();

};

#endif  /* ALPIDE_H */
