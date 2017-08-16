#include "AlpideDictionary.h"
#include "TAlpide.h"
#include "TChipConfig.h"
#include "TReadoutBoard.h"
#include <bitset>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <array>

using namespace std;

const char* TAlpide::fRegName[] = {
    ( char* ) "Command Register                          ",
    ( char* ) "Mode Control register                     ",
    ( char* ) "Disable of regions 0-15                   ",
    ( char* ) "Disable of regions 16-31                  ",
    ( char* ) "FROMU Configuration Register 1            ",
    ( char* ) "FROMU Configuration Register 2            ",
    ( char* ) "FROMU Configuration Register 3            ",
    ( char* ) "FROMU Pulsing Register 1                  ",
    ( char* ) "FROMU Pulsing Register 2                  ",
    ( char* ) "FROMU Status Register 1                   ",
    ( char* ) "FROMU Status Register 2                   ",
    ( char* ) "FROMU Status Register 3                   ",
    ( char* ) "FROMU Status Register 4                   ",
    ( char* ) "FROMU Status Register 5                   ",
    ( char* ) "DAC settings for DCLK and MCLK I/O buffers",
    ( char* ) "DAC settings for CMU I/O buffers          ",
    ( char* ) "CMU and DMU Configuration Register        ",
    ( char* ) "CMU and DMU Status Register               ",
    ( char* ) "DMU Data FIFO [15:0]                      ",
    ( char* ) "DMU Data FIFO [23:16]                     ",
    ( char* ) "DTU Configuration Register                ",
    ( char* ) "DTU DACs Register                         ",
    ( char* ) "DTU PLL Lock Register 1                   ",
    ( char* ) "DTU PLL Lock Register 2                   ",
    ( char* ) "DTU Test Register 1                       ",
    ( char* ) "DTU Test Register 2                       ",
    ( char* ) "DTU Test Register 3                       ",
    ( char* ) "BUSY min width                            "
};

const char* TAlpide::fDACsRegName[] = {
    ( char* ) "VRESETP",
    ( char* ) "VRESETD",
    ( char* ) "VCASP  ",
    ( char* ) "VCASN  ",
    ( char* ) "VPULSEH",
    ( char* ) "VPULSEL",
    ( char* ) "VCASN2 ",
    ( char* ) "VCLIP  ",
    ( char* ) "VTEMP  ",
    ( char* ) "IAUX2  ",
    ( char* ) "IRESET ",
    ( char* ) "IDB    ",
    ( char* ) "IBIAS  ",
    ( char* ) "ITHR   "
};

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

//___________________________________________________________________
void TAlpide::SetVerboseLevel( const int level )
{
    if ( level > kTERSE ) {
        cout << "TAlpide::SetVerboseLevel() - " << level << " for chip id = " << DecomposeChipId() << endl;
    }
    TVerbosity::SetVerboseLevel( level );
}

#pragma mark - dump

//___________________________________________________________________
void TAlpide::DumpConfig( const char* fileName, const bool writeFile, char* config )
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::DumpConfig() - chip config. not found!" );
    }
    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::DumpConfig() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }

    uint16_t value;
   
    if (writeFile) {
        FILE *fp = fopen(fileName, "w");
        fprintf(fp, "Chip ID %i\n", fChipId);
        // DACs
        ReadRegister( AlpideRegister::VRESETP, value );
        fprintf(fp, "VRESETP %i\n", value);
        ReadRegister( AlpideRegister::VRESETD, value );
        fprintf(fp, "VRESETD %i\n", value);
        ReadRegister( AlpideRegister::VCASP, value );
        fprintf(fp, "VCASP   %i\n", value);
        ReadRegister( AlpideRegister::VCASN, value );
        fprintf(fp, "VCASN   %i\n", value);
        ReadRegister( AlpideRegister::VPULSEH, value );
        fprintf(fp, "VPULSEH %i\n", value);
        ReadRegister( AlpideRegister::VPULSEL, value );
        fprintf(fp, "VPULSEL %i\n", value);
        ReadRegister( AlpideRegister::VCASN2, value );
        fprintf(fp, "VCASN2  %i\n", value);
        ReadRegister( AlpideRegister::VCLIP, value );
        fprintf(fp, "VCLIP   %i\n", value);
        ReadRegister( AlpideRegister::VTEMP, value );
        fprintf(fp, "VTEMP   %i\n", value);
        ReadRegister( AlpideRegister::IAUX2, value );
        fprintf(fp, "IAUX2   %i\n", value);
        ReadRegister( AlpideRegister::IRESET, value );
        fprintf(fp, "IRESET  %i\n", value);
        ReadRegister( AlpideRegister::IDB, value );
        fprintf(fp, "IDB     %i\n", value);
        ReadRegister( AlpideRegister::IBIAS, value );
        fprintf(fp, "IBIAS   %i\n", value);
        ReadRegister( AlpideRegister::ITHR, value );
        fprintf(fp, "ITHR    %i\n", value);
        
        fprintf(fp, "\n");
        // Mode control register
        ReadRegister( AlpideRegister::MODE_CONTROL, value );
        fprintf( fp, "MODE_CONTROL  %i\n", value );
        
        // FROMU config reg 1: [5]: test pulse mode; [6]: enable test strobe, etc.
        ReadRegister( AlpideRegister::FROMU_CONFIG1, value );
        fprintf( fp, "FROMU_CONFIG1  %i\n", value );
        
        // FROMU config reg 2: strobe duration
        ReadRegister( AlpideRegister::FROMU_CONFIG2, value );
        fprintf( fp, "FROMU_CONFIG2  %i\n", value );
        
        // FROMU pulsing reg 1: delay between pulse and strobe if the feature of automatic strobing is enabled
        ReadRegister( AlpideRegister::FROMU_PULSING1, value );
        fprintf( fp, "FROMU_PULSING1  %i\n", value );
        
        // FROMU pulsing reg 2: pulse duration
        ReadRegister( AlpideRegister::FROMU_PULSING2, value );
        fprintf( fp, "FROMU_PULSING2  %i\n", value );
        
        // CMU DMU config reg
        ReadRegister( AlpideRegister::CMU_DMU_CONFIG, value );
        fprintf( fp, "CMU_DMU_CONFIG  %i\n", value );
        
        fclose( fp );
    }

    config[0] = '\0';
    // DACs
    ReadRegister( AlpideRegister::VRESETP, value );
    sprintf( config, "VRESETP %i\n", value );
    ReadRegister( AlpideRegister::VRESETD, value );
    sprintf( config, "%sVRESETD %i\n", config, value );
    ReadRegister( AlpideRegister::VCASP, value );
    sprintf( config, "%sVCASP   %i\n", config, value );
    ReadRegister( AlpideRegister::VCASN, value );
    sprintf( config, "%sVCASN   %i\n", config, value );
    ReadRegister( AlpideRegister::VPULSEH, value );
    sprintf( config, "%sVPULSEH %i\n", config, value );
    ReadRegister( AlpideRegister::VPULSEL, value);
    sprintf( config, "%sVPULSEL %i\n", config, value );
    ReadRegister( AlpideRegister::VCASN2, value );
    sprintf( config, "%sVCASN2  %i\n", config, value );
    ReadRegister( AlpideRegister::VCLIP, value );
    sprintf( config, "%sVCLIP   %i\n", config, value );
    ReadRegister( AlpideRegister::VTEMP, value );
    sprintf( config, "%sVTEMP   %i\n", config, value );
    ReadRegister( AlpideRegister::IAUX2, value );
    sprintf( config, "%sIAUX2   %i\n", config, value );
    ReadRegister( AlpideRegister::IRESET, value );
    sprintf( config, "%sIRESET  %i\n", config, value );
    ReadRegister( AlpideRegister::IDB, value );
    sprintf( config, "%sIDB     %i\n", config, value );
    ReadRegister( AlpideRegister::IBIAS, value );
    sprintf( config, "%sIBIAS   %i\n", config, value );
    ReadRegister( AlpideRegister::ITHR, value );
    sprintf( config, "%sITHR    %i\n", config, value );

    sprintf( config, "%s\n", config );
    // Mode control register
    ReadRegister( AlpideRegister::MODE_CONTROL, value );
    sprintf( config, "%sMODE_CONTROL  %i\n", config, value );
  
    // FROMU config reg 1: [5]: test pulse mode; [6]: enable test strobe, etc.
    ReadRegister( AlpideRegister::FROMU_CONFIG1, value );
    sprintf( config, "%sFROMU_CONFIG1  %i\n", config, value );
  
    // FROMU config reg 2: strobe duration
    ReadRegister( AlpideRegister::FROMU_CONFIG2, value );
    sprintf( config, "%sFROMU_CONFIG2  %i\n", config, value );

    // FROMU pulsing reg 1: delay between pulse and strobe if the feature of
    // automatic strobing is enabled
    ReadRegister( AlpideRegister::FROMU_PULSING1, value );
    sprintf( config, "%sFROMU_PULSING1  %i\n", config, value );

    // FROMU pulsing reg 2: pulse duration
    ReadRegister( AlpideRegister::FROMU_PULSING2, value );
    sprintf( config, "%sFROMU_PULSING2  %i\n", config, value );

    // CMU DMU config reg
    ReadRegister( AlpideRegister::CMU_DMU_CONFIG, value );
    sprintf( config, "%sCMU_DMU_CONFIG  %i\n", config, value );

}

//___________________________________________________________________
void TAlpide::DumpConfig()
{
    array<uint16_t, (uint16_t)AlpideRegister::BUSY_MINWIDTH + 1> regs;
    regs.fill( 0 );
    
    bool doExecute = false;
    for ( uint16_t i = 0; i < regs.size(); i++ ) {
        ReadRegister( i, regs.at(i), doExecute );
    }
    
    const uint16_t nDACs = 14;
    array<uint16_t, nDACs> dacregs;
    dacregs.fill( 0 );
    
    int ii = 0;
    for ( uint16_t i = (uint16_t)AlpideRegister::VRESETP; i < (uint16_t)AlpideRegister::VRESETP + dacregs.size(); i++ ) {
        if ( i == (uint16_t)AlpideRegister::VRESETP + dacregs.size()-1 ) doExecute = true;
        ReadRegister( i, dacregs.at(ii), doExecute );
        ii++;
    }

    cout << "TAlpide::DumpConfig() - chip id = " << DecomposeChipId()  <<  endl;
    
    for ( uint i = 0; i < regs.size(); i++ ) {
        cout << fRegName[i] << " \t (hex) " << std::hex << regs.at(i) << endl;
    }
    for ( uint i = 0; i < dacregs.size(); i++ ) {
        cout << fDACsRegName[i] << " \t (dec) " << std::dec << dacregs.at(i) << endl;
    }
}


#pragma mark - basic operations with registers

//___________________________________________________________________
void TAlpide::ReadRegister( const AlpideRegister address,
                           uint16_t& value,
                           const bool doExecute,
                           const bool skipDisabledChip )
{
    if ( fChipId < 0 ) {
        throw domain_error( "TAlpide::ReadRegister() - undefined chip id.");
    }
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( !spBoard ) {
        cerr << "TAlpide::ReadRegister() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ReadRegister() - unuseable readout board." );
    }
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ReadRegister() - chip config. not found!" );
    }
    if ( skipDisabledChip && !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::ReadRegister() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }
    
    int err = -1;
    try {
        err = spBoard->ReadChipRegister( (uint16_t)address, value, (uint8_t)fChipId, doExecute );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
    }
    if ( err < 0 ) {
        cerr << "TAlpide::ReadRegister() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ReadRegister() - failed." );
    }
    return;
}

//___________________________________________________________________
void TAlpide::ReadRegister( const uint16_t address,
                           uint16_t& value,
                           const bool doExecute,
                           const bool skipDisabledChip )
{
    if ( fChipId < 0 ) {
        throw domain_error( "TAlpide::ReadRegister() - undefined chip id.");
    }
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( !spBoard ) {
        cerr << "TAlpide::ReadRegister() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ReadRegister() - unuseable readout board." );
    }
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ReadRegister() - chip config. not found!" );
    }
    if ( skipDisabledChip && !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::ReadRegister() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }

    int err = -1;
    try {
        err = spBoard->ReadChipRegister( address, value, (uint8_t)fChipId, doExecute );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
    }
    if ( err < 0 ) {
        cerr << "TAlpide::ReadRegister() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ReadRegister() - failed." );
    }
    return;
}

//___________________________________________________________________
void TAlpide::WriteRegister( const AlpideRegister address,
                            uint16_t value,
                            const bool doExecute,
                            const bool verify,
                            const bool skipDisabledChip )
{
    if ( fChipId < 0 ) {
        throw domain_error( "TAlpide::WriteRegister() - undefined chip id.");
    }
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( !spBoard ) {
        cerr << "TAlpide::WriteRegister() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::WriteRegister() - unuseable readout board." );
    }
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::WriteRegister() - chip config. not found!" );
    }
    if ( skipDisabledChip && !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::WriteRegister() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }

    int result = -1;
    try {
        result = spBoard->WriteChipRegister( (uint16_t)address, value, (uint8_t)fChipId, (doExecute || verify) ); // always execute if verify is true
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
    }
    if ( result < 0 ) {
        cerr << "TAlpide::WriteRegister() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::WriteRegister() - failed." );
    }
    if ( verify ) {
        uint16_t check;
        try {
            ReadRegister( address, check );
        } catch ( exception& msg ) {
            cerr << msg.what() << endl;
            cerr << "TAlpide::WriteRegister() - chip id = " << DecomposeChipId() << endl;
            throw runtime_error( "TAlpide::WriteRegister() - readback check failed." );
        }
        if ( check != value ) {
            cerr << "TAlpide::WriteRegister() - chip id = " << DecomposeChipId() << endl;
            cerr << "TAlpide::WriteRegister() - value = " << value << endl;
            cerr << "TAlpide::WriteRegister() - readback value = " << check << endl;
            throw runtime_error( "TAlpide::WriteRegister() - wrong readback value." );
        }
    }
    return;
}

//___________________________________________________________________
void TAlpide::WriteRegister( const uint16_t address,
                            uint16_t value,
                            const bool doExecute,
                            const bool verify,
                            const bool skipDisabledChip )
{
    if ( fChipId < 0 ) {
        throw domain_error( "TAlpide::WriteRegister() - undefined chip id.");
    }
    shared_ptr<TReadoutBoard> spBoard = fReadoutBoard.lock();
    if ( !spBoard ) {
        cerr << "TAlpide::WriteRegister() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::WriteRegister() - unuseable readout board." );
    }
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::WriteRegister() - chip config. not found!" );
    }
    if ( skipDisabledChip && !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::WriteRegister() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }
    
    int result = -1;
    try {
        result = spBoard->WriteChipRegister( address, value, (uint8_t)fChipId, (doExecute || verify) ); // always execute if verify is true
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
    }
    if ( result < 0 ) {
        cerr << "TAlpide::WriteRegister() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::WriteRegister() - failed." );
    }
    if ( verify ) {
        uint16_t check;
        try {
            ReadRegister( address, check );
        } catch ( exception& msg ) {
            cerr << msg.what() << endl;
            cerr << "TAlpide::WriteRegister() - chip id = " << DecomposeChipId() << endl;
            throw runtime_error( "TAlpide::WriteRegister() - readback check failed." );
        }
        if ( check != value ) {
            cerr << "TAlpide::WriteRegister() - chip id = " << DecomposeChipId() << endl;
            cerr << "TAlpide::WriteRegister() - value = " << value << endl;
            cerr << "TAlpide::WriteRegister() - readback value = " << check << endl;
            throw runtime_error( "TAlpide::WriteRegister() - wrong readback value." );
        }
    }

    return;
}

//___________________________________________________________________
void TAlpide::ModifyRegisterBits( const AlpideRegister address,
                                const uint8_t lowBit,
                                const uint8_t nBits,
                                uint16_t value,
                                const bool verify,
                                 const bool skipDisabledChip )
{
    
    if ( (lowBit > 15) || (lowBit + nBits > 15)) {
        throw domain_error( "TAlpide::ModifyRegisterBits() - illegal limits." );
    }
    uint16_t registerValue, mask = 0xffff;
    try {
        ReadRegister( address, registerValue, skipDisabledChip );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::ModifyRegisterBits() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ModifyRegisterBits() - readback step failed." );
    }
    
    for (int i = lowBit; i < lowBit + nBits; i++) {
        mask -= 1 << i;
    }
    
    registerValue &= mask;                // set all bits that are to be overwritten to 0
    value         &= (1 << nBits) -1;     // make sure value fits into nBits
    registerValue |= value << nBits;      // or value into the foreseen spot
    try {
        WriteRegister( address, registerValue, verify, skipDisabledChip );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::ModifyRegisterBits() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ModifyRegisterBits() - failed to overwrite bits." );
    }
    return;
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
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::Init() - chip config. not found!" );
    }
    
    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::Init() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }

    ClearPixSelectBits(true);
}

//___________________________________________________________________
void TAlpide::WritePixConfReg( AlpidePixConfigReg reg, const bool data )
{
    uint16_t pixconfig = (int) reg & 0x1;
    pixconfig         |= (data?1:0) << 1;
    WriteRegister( AlpideRegister::PIXEL_CONFIG, pixconfig );
}

//___________________________________________________________________
void TAlpide::WritePixRegAll( AlpidePixConfigReg reg, const bool data )
{
    // TODO: To be checked whether this methods works or whether a loop over rows has to be implemented

    WritePixConfReg( reg, data );
    
    // set all colsel and all rowsel to 1
    
    uint16_t address =
        (uint16_t)AlpideRegister::PIXEL_BROADCAST
        | (uint16_t)AlpideRegister::PIXEL_COLSEL1_BASE
        | (uint16_t)AlpideRegister::PIXEL_COLSEL2_BASE
        | (uint16_t)AlpideRegister::PIXEL_ROWSEL_BASE ; // address = 0x487
    
    WriteRegister( address, 0xffff ); // see alpide manual, section 3.6.2, page 70

    ClearPixSelectBits( false );
}

//___________________________________________________________________
void TAlpide::WritePixRegRow( AlpidePixConfigReg reg, const bool data, const int row)
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
void TAlpide::WritePixRegSingle( AlpidePixConfigReg reg,
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
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureCMU() - chip config. not found!" );
    }
   if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::ApplyStandardDACSettings() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }
    // TODO: pAlpide 3 settings, to be confirmed
    if ( backBias == 0 ) {
        spConfig->SetParamValue( "VCASN",    50 );
        spConfig->SetParamValue( "VCASN2",   62 );
        spConfig->SetParamValue( "VCLIP",     0 );
        spConfig->SetParamValue( "VRESETD", 117 );
        spConfig->SetParamValue( "IDB",      29 );
    } else if ( backBias == 3 ) {
        spConfig->SetParamValue( "VCASN",   105 );
        spConfig->SetParamValue( "VCASN2",  117 );
        spConfig->SetParamValue( "VCLIP",    60 );
        spConfig->SetParamValue( "VRESETD", 117 );
        spConfig->SetParamValue( "IDB",      29 );
    } else if ( backBias == 6 ) {
        spConfig->SetParamValue( "VCASN",   135 );
        spConfig->SetParamValue( "VCASN2",  147 );
        spConfig->SetParamValue( "VCLIP",   100 );
        spConfig->SetParamValue( "VRESETD", 147 );
        spConfig->SetParamValue( "IDB",      29 );
    } else {
        cerr << "TAlpide::ApplyStandardDACSettings() - back bias " << backBias << " V undefined." << endl;
        throw runtime_error( "TAlpide::ApplyStandardDACSettings() - Settings not defined for this back bias value. Please set manually." );
    }
    try {
        WriteRegister( AlpideRegister::VCASN,    spConfig->GetParamValue("VCASN") );
        WriteRegister( AlpideRegister::VCASN2,   spConfig->GetParamValue("VCASN2") );
        WriteRegister( AlpideRegister::VCLIP,    spConfig->GetParamValue("VCLIP") );
        WriteRegister( AlpideRegister::VRESETD,  spConfig->GetParamValue("VRESETD") );
        WriteRegister( AlpideRegister::IDB,      spConfig->GetParamValue("IDB") );
    } catch ( exception& msg  ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::ApplyStandardDACSettings() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ApplyStandardDACSettings() - failed." );
    }
}

// Setting up of readout from config file- CMU part
// (alpide manual, section 3.8.3, page 75)
//___________________________________________________________________
void TAlpide::ConfigureCMU()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureCMU() - chip config. not found!" );
    }
    
    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::ConfigureCMU() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }

    uint16_t cmuconfig = 0;
    
    cmuconfig |= (spConfig->GetPreviousId() & 0xf);
    cmuconfig |= (spConfig->GetInitialToken     () ? 1:0) << 4;
    cmuconfig |= (spConfig->GetDisableManchester() ? 1:0) << 5;
    cmuconfig |= (spConfig->GetEnableDdr        () ? 1:0) << 6;
    
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TAlpide::ConfigureCMU() - chip id = " << DecomposeChipId()  <<  endl;
        cout << "TAlpide::ConfigureCMU() - value = " <<  endl;
        cout << "TAlpide::ConfigureCMU() - \t (bin) " << std::bitset<16> ( cmuconfig ) << endl;
        cout << "TAlpide::ConfigureCMU() - \t (hex) " << std::hex << cmuconfig << endl;
        cout << "TAlpide::ConfigureCMU() - \t (dec) " << std::dec << cmuconfig << endl;
    }
    try {
        WriteRegister( AlpideRegister::CMU_DMU_CONFIG, cmuconfig );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::ConfigureCMU() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ConfigureCMU() - failed." );
    }
}

// Setting up of DTU test register 1 (DTU_TEST1) bits
// (alpide manual, section 3.2.2, page 44)
//___________________________________________________________________
void TAlpide::ConfigureDTU_TEST1()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureDTU_TEST1() - chip config. not found!" );
    }
    
    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::ConfigureDTU_TEST1() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }
    
    uint16_t config = 0;
    config |= (spConfig->GetParamValue("DTU_TEST1_TEST_ENABLE") ? 1 : 0);
    config |= (spConfig->GetParamValue("DTU_TEST1_PRBS_ENABLE") ? 1 : 0) << 1;
    config |= (spConfig->GetParamValue("DTU_TEST1_SINGLE_MODE") ? 1 : 0) << 2;
    config |= (spConfig->GetParamValue("DTU_TEST1_PRBS_RATE") & 0x3 ) << 3;
    config |= (spConfig->GetParamValue("DTU_TEST1_BYPASS_8B10B") ? 1 : 0) << 5;
    
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TAlpide::ConfigureDTU_TEST1() - chip id = " << DecomposeChipId()  <<  endl;
        cout << "TAlpide::ConfigureDTU_TEST1() - value = " <<  endl;
        cout << "TAlpide::ConfigureDTU_TEST1() - \t (bin) " << std::bitset<16> ( config ) << endl;
        cout << "TAlpide::ConfigureDTU_TEST1() - \t (hex) " << std::hex << config << endl;
        cout << "TAlpide::ConfigureDTU_TEST1() - \t (dec) " << std::dec << config << endl;
    }
    try {
        WriteRegister( AlpideRegister::DTU_TEST1, config );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::ConfigureDTU_TEST1() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ConfigureDTU_TEST1() - failed." );
    }
}

// Setting up of readout - FROMU part
// (alpide manual, section 3.2.2, page 38-40)
// (alpide manual, section 3.8.3, page 76)
//___________________________________________________________________
void TAlpide::ConfigureFROMU()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureFROMU() - chip config. not found!" );
    }

    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::ConfigureFROMU() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }

    const int  mebmask          = spConfig->GetPixelMEBMask();
    const bool internalStrobe   = spConfig->GetEnableInternalStrobe();
    const bool busyMonitoring   = spConfig->GetEnableBusyMonitoring();
    const int  testPulseMode    = spConfig->GetTestPulseMode();
    const bool testStrobe       = spConfig->GetEnableTestStrobe();
    const bool rotatePulseLines = spConfig->GetEnableRotatePulseLines();
    const int  triggerDelay     = spConfig->GetTriggerDelay();
   
    uint16_t fromuconfig = 0;
    
    fromuconfig |= mebmask;
    fromuconfig |= (internalStrobe   ? 1:0) << 3;
    fromuconfig |= (busyMonitoring   ? 1:0) << 4;
    fromuconfig |= ((int) testPulseMode)    << 5;
    fromuconfig |= (testStrobe       ? 1:0) << 6;
    fromuconfig |= (rotatePulseLines ? 1:0) << 7;
    fromuconfig |= (triggerDelay & 0x7)     << 8;
    
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TAlpide::ConfigureFROMU() - chip id = " << DecomposeChipId()  <<  endl;
        cout << "TAlpide::ConfigureFROMU() - value = " <<  endl;
        cout << "TAlpide::ConfigureFROMU() - \t (bin) " << std::bitset<16> ( fromuconfig ) << endl;
        cout << "TAlpide::ConfigureFROMU() - \t (hex) " << std::hex << fromuconfig << endl;
        cout << "TAlpide::ConfigureFROMU() - \t (dec) " << std::dec << fromuconfig << endl;
    }
    try {
        WriteRegister( AlpideRegister::FROMU_CONFIG1,  fromuconfig );
        WriteRegister( AlpideRegister::FROMU_CONFIG2,  spConfig->GetStrobeDuration() );
        WriteRegister( AlpideRegister::FROMU_PULSING1, spConfig->GetStrobeDelay() );
        WriteRegister( AlpideRegister::FROMU_PULSING2, spConfig->GetPulseDuration() );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::ConfigureFROMU() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ConfigureFROMU() - failed." );
    }
}

//___________________________________________________________________
void TAlpide::ConfigureBuffers()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureBuffers() - chip config. not found!" );
    }

    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::ConfigureBuffers() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }

    uint16_t clocks = 0, ctrl = 0;
    
    clocks |= (spConfig->GetDclkReceiver () & 0xf);
    clocks |= (spConfig->GetDclkDriver   () & 0xf) << 4;
    clocks |= (spConfig->GetMclkReceiver () & 0xf) << 8;
    
    ctrl   |= (spConfig->GetDctrlReceiver() & 0xf);
    ctrl   |= (spConfig->GetDctrlDriver  () & 0xf) << 4;
    
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TAlpide::ConfigureBuffers() - chip id = " << DecomposeChipId()  <<  endl;
        cout << "TAlpide::ConfigureBuffers() - clocks = " <<  endl;
        cout << "TAlpide::ConfigureBuffers() - \t (bin) " << std::bitset<16> ( clocks ) << endl;
        cout << "TAlpide::ConfigureBuffers() - \t (hex) " << std::hex << clocks << endl;
        cout << "TAlpide::ConfigureBuffers() - \t (dec) " << std::dec << clocks << endl;
        cout << "TAlpide::ConfigureBuffers() - ctrl = " <<  endl;
        cout << "TAlpide::ConfigureBuffers() - \t (bin) " << std::bitset<16> ( ctrl ) << endl;
        cout << "TAlpide::ConfigureBuffers() - \t (hex) " << std::hex << ctrl << endl;
        cout << "TAlpide::ConfigureBuffers() - \t (dec) " << std::dec << ctrl << endl;
    }
    
    try {
        WriteRegister( AlpideRegister::DACS_CLKIO_BUF, clocks );
        WriteRegister( AlpideRegister::DACS_CMUIO_BUF, ctrl );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::ConfigureBuffers() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::ConfigureBuffers() - failed." );
    }
}

//___________________________________________________________________
int TAlpide::ConfigureMaskStage( int nPix, const int iStage )
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::ConfigureMaskStage() - chip config. not found!" );
    }

    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::ConfigureMaskStage() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return iStage;
    }

    // check that nPix is one of (1, 2, 4, 8, 16, 32)
    if ((nPix <= 0) || (nPix & (nPix - 1)) || (nPix > 32)) {
        cout << "TAlpide::ConfigureMaskStage() - Warning: bad number of pixels for mask stage (" << nPix << ", using 1 instead" << endl;
        nPix = 1;
    }
    WritePixRegAll( AlpidePixConfigReg::MASK_ENABLE,   true );
    WritePixRegAll( AlpidePixConfigReg::PULSE_ENABLE, false );
    
    // complete row
    if ( nPix == 32 ) {
        WritePixRegRow( AlpidePixConfigReg::MASK_ENABLE,   false, iStage );
        WritePixRegRow( AlpidePixConfigReg::PULSE_ENABLE, true, iStage );
        return iStage;
    } else {
        int colStep = 32 / nPix;
        for ( int icol = 0; icol < 1024; icol += colStep ) {
            WritePixRegSingle( AlpidePixConfigReg::MASK_ENABLE,   false, iStage % 512, icol + iStage / 512);
            WritePixRegSingle( AlpidePixConfigReg::PULSE_ENABLE, true,  iStage % 512, icol + iStage / 512);
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
    
    if ( !(spConfig->IsEnabled()) || (spConfig->IsOBSlave()) ) {
        // TODO for OB: is this better than (does the OB chip have slaves? if not, no PLL config needed since it must be an OB slave chip => DTU off)
        if ( GetVerboseLevel() > kTERSE ) {
            if ( !(spConfig->IsEnabled()) ) {
                cout << "TAlpide::WriteControlReg() - chip id = "
                    << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
            }
            if ( spConfig->IsOBSlave() ) {
                cout << "TAlpide::WriteControlReg() - chip id = "
                << DecomposeChipId()  << " : chip in OB slave mode, skipped." <<  endl;
            }
        }
        return;
    }

    uint16_t controlreg = 0;
    
    if ( chipMode == AlpideChipMode::CONFIG ) {
        controlreg = 0x20; // set chip to config mode
        // this is equivalent to:
        //   const bool fEnableClustering = false;
        //   const int fMatrixReadoutSpeed = 0;
        //   const int fSerialLinkSpeed = AlpideIBSerialLinkSpeed::IB1200;
        //   const bool fEnableSkewingGlobal = false;
        //   const bool fEnableSkewingStartRO = false;
        //   const bool fEnableClockGating = false;
        //   const bool fEnableCMUReadout = false;
    } else {
        controlreg |= (uint16_t) chipMode;
        
        controlreg |= (spConfig->GetEnableClustering    () ? 1:0) << 2;
        controlreg |= (spConfig->GetMatrixReadoutSpeed  () & 0x1) << 3;
        controlreg |= (spConfig->GetSerialLinkSpeed     () & 0x3) << 4;
        controlreg |= (spConfig->GetEnableSkewingGlobal () ? 1:0) << 6;
        controlreg |= (spConfig->GetEnableSkewingStartRO() ? 1:0) << 7;
        controlreg |= (spConfig->GetEnableClockGating   () ? 1:0) << 8;
        controlreg |= (spConfig->GetEnableCMUReadout    () ? 1:0) << 9;
    }
    
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TAlpide::WriteControlReg() - chip id = "
        << DecomposeChipId() <<  endl;
        cout << "TAlpide::WriteControlReg() - value = " <<  endl;
        cout << "TAlpide::WriteControlReg() - \t (bin) " << std::bitset<16> ( controlreg ) << endl;
        cout << "TAlpide::WriteControlReg() - \t (hex) " << std::hex << controlreg << endl;
        cout << "TAlpide::WriteControlReg() - \t (dec) " << std::dec << controlreg << endl;
    }

    try {
        WriteRegister( AlpideRegister::MODE_CONTROL, controlreg );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::WriteControlReg() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::WriteControlReg() - failed." );
    }
}

// Configuration and start-up of the DTU
// (alpide manual, section 3.8.2, page 75)
//___________________________________________________________________
void TAlpide::BaseConfigPLL()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::BaseConfigPLL() - chip config. not found!" );
    }

    if ( spConfig->GetParamValue("LINKSPEED") < 0 ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TAlpide::BaseConfigPLL() - high-speed link deactivated" << endl;
        }
        return;
    }
    if ( !(spConfig->IsEnabled()) || (spConfig->IsOBSlave()) ) {
        // TODO for OB: is this better than (does the OB chip have slaves? if not, no PLL config needed since it must be an OB slave chip => DTU off)
        if ( GetVerboseLevel() > kTERSE ) {
            if ( !(spConfig->IsEnabled()) ) {
                cout << "TAlpide::BaseConfigPLL() - chip id = "
                << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
            }
            if ( spConfig->IsOBSlave() ) {
                cout << "TAlpide::BaseConfigPLL() - chip id = "
                << DecomposeChipId()  << " : chip in OB slave mode, skipped." <<  endl;
            }
        }
        return;
    }
    
    //--- Configure the PLL

    // Write to DTU config register and DTU DACs register
    
    const uint16_t Phase      = 8; // 4bit Value, default 8
    const uint16_t Stages     = 1; // 0 -> 3 stages, 1 -> 4,  3 -> 5 (typical 4 stages)
    const uint16_t ChargePump = 8; // charge pump current (4bit value, default 8)
    const uint16_t Driver     = 15; // line driver current
    const uint16_t Preemp     = 15; // pre-emphasis driver
    uint16_t Value;
    
    Value = (Stages & 0x3) | 0x4 | 0x8 | ((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x8: PLL off
    WriteRegister( AlpideRegister::DTU_CONFIG, Value );
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TAlpide::BaseConfigPLL() - chip id = "
        << DecomposeChipId() <<  endl;
        cout << "TAlpide::BaseConfigPLL() - DTU config reg., value = " <<  endl;
        cout << "TAlpide::BaseConfigPLL() - \t (bin) " << std::bitset<16> ( Value ) << endl;
        cout << "TAlpide::BaseConfigPLL() - \t (hex) " << std::hex << Value << endl;
        cout << "TAlpide::BaseConfigPLL() - \t (dec) " << std::dec << Value << endl;
    }
    
    Value = (ChargePump & 0xf) | ((Driver & 0xf) << 4) | ((Preemp & 0xf) << 8);
    WriteRegister( AlpideRegister::DTU_DACS, Value );
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TAlpide::BaseConfigPLL() - DTU DACs reg., value = " <<  endl;
        cout << "TAlpide::BaseConfigPLL() - \t (bin) " << std::bitset<16> ( Value ) << endl;
        cout << "TAlpide::BaseConfigPLL() - \t (hex) " << std::hex << Value << endl;
        cout << "TAlpide::BaseConfigPLL() - \t (dec) " << std::dec << Value << endl;
    }
    
    //--- Clear PLL off signal bit to start the PLL up

    Value = (Stages & 0x3) | 0x4 | ((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x8: PLL off
    WriteRegister( AlpideRegister::DTU_CONFIG, Value );

    //--- Force PLL reset
    
    // first set and then clear the PLL reset bit with 2 subsequent transactions
    // (leave all the other bits of the DTU config register unchanged)

    Value = (Stages & 0x3) | 0x4 | 0x100 |((Phase & 0xf) << 4);   // 0x4: narrow bandwidth, 0x100: Reset
    WriteRegister( AlpideRegister::DTU_CONFIG, Value );
    Value = (Stages & 0x3) | 0x4 |((Phase & 0xf) << 4);           // Reset off
    WriteRegister( AlpideRegister::DTU_CONFIG, Value );
}

//___________________________________________________________________
void TAlpide::BaseConfigMask()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::BaseConfigMask() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }
    WritePixRegAll( AlpidePixConfigReg::MASK_ENABLE,   true );
    WritePixRegAll( AlpidePixConfigReg::PULSE_ENABLE, false );
}

//___________________________________________________________________
void TAlpide::BaseConfigDACs()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::BaseConfigDACs() - chip config. not found!" );
    }
    
    if ( !(spConfig->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TAlpide::BaseConfigDACs() - chip id = "
            << DecomposeChipId()  << " : disabled chip, skipped." <<  endl;
        }
        return;
    }
    
    if ( GetVerboseLevel() > kVERBOSE ) {
        cout << "TAlpide::BaseConfigDACs() - chip id = "
        << DecomposeChipId() <<  endl;
    }
 
    try {
        if ( GetVerboseLevel() > kVERBOSE ) {
            cout << "TAlpide::BaseConfigDACs() - \t ITHR = "
                 <<  spConfig->GetParamValue("ITHR") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t IDB = "
                 <<  spConfig->GetParamValue("IDB") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t VCASN = "
                 <<  spConfig->GetParamValue("VCASN") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t VCASN2 = "
                 <<  spConfig->GetParamValue("VCASN2") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t VCLIP = "
                 <<  spConfig->GetParamValue("VCLIP") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t VRESETD = "
                 <<  spConfig->GetParamValue("VRESETD") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t VCASP = "
                 <<  spConfig->GetParamValue("VCASP") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t VPULSEL = "
                 <<  spConfig->GetParamValue("VPULSEL") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t VPULSEH = "
                 <<  spConfig->GetParamValue("VPULSEH") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t IBIAS = "
                 <<  spConfig->GetParamValue("IBIAS") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t VRESETP = "
                 <<  spConfig->GetParamValue("VRESETP") << endl;
            cout << "TAlpide::BaseConfigDACs() - \t IRESET = "
                 <<  spConfig->GetParamValue("IRESET") << endl;
        }
        WriteRegister( AlpideRegister::ITHR,    spConfig->GetParamValue("ITHR"));
        WriteRegister( AlpideRegister::IDB,     spConfig->GetParamValue("IDB"));
        WriteRegister( AlpideRegister::VCASN,   spConfig->GetParamValue("VCASN"));
        WriteRegister( AlpideRegister::VCASN2,  spConfig->GetParamValue("VCASN2"));
        WriteRegister( AlpideRegister::VCLIP,   spConfig->GetParamValue("VCLIP"));
        WriteRegister( AlpideRegister::VRESETD, spConfig->GetParamValue("VRESETD"));
        WriteRegister( AlpideRegister::VCASP,   spConfig->GetParamValue("VCASP"));
        WriteRegister( AlpideRegister::VPULSEL, spConfig->GetParamValue("VPULSEL"));
        WriteRegister( AlpideRegister::VPULSEH, spConfig->GetParamValue("VPULSEH"));
        WriteRegister( AlpideRegister::IBIAS,   spConfig->GetParamValue("IBIAS"));
        // not used DACs..
        WriteRegister( AlpideRegister::VRESETP, spConfig->GetParamValue("VRESETP"));
        //WriteRegister( AlpideRegister::VTEMP,   spConfig->GetParamValue("VTEMP")); // TODO: uncomment when default value is known (see also TChipConfig class)
        WriteRegister( AlpideRegister::IRESET,  spConfig->GetParamValue("IRESET"));
        //WriteRegister( AlpideRegister::IAUX2,   spConfig->GetParamValue("IAUX2")); // TODO: uncomment when default value is known (see also TChipConfig class)
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::BaseConfigDACs() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::BaseConfigDACs() - failed." );
    }
}

// Propagate the chip configuration to the chip registers
// All chip configuration comes from TChipConfig, hence from the config file
//___________________________________________________________________
void TAlpide::BaseConfig()
{
    shared_ptr<TChipConfig> spConfig = fConfig.lock();
    if ( !spConfig ) {
        throw runtime_error( "TAlpide::BaseConfig() - chip config. not found!" );
    }

    try {
        WriteControlReg( AlpideChipMode::CONFIG ); // set chip to config mode
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        cerr << "TAlpide::BaseConfig() - chip id = " << DecomposeChipId() << endl;
        throw runtime_error( "TAlpide::BaseConfig() - chip can not be set into config mode." );
    }
    
    ConfigureCMU();
    ConfigureFROMU();
    BaseConfigDACs();
    BaseConfigMask();
    BaseConfigPLL();
    
    if ( spConfig->GetReadoutMode() ) {
        WriteControlReg( AlpideChipMode::CONTINUOUS );
    } else {
        WriteControlReg( AlpideChipMode::TRIGGERED ); // strobed readout mode
    }
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
    cout << "Debug Stream chip id " << DecomposeChipId() << ": " << endl;
    
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
void TAlpide::ClearPixSelectBits( const bool clearPulseGating )
{
    uint16_t address =
        (uint16_t)AlpideRegister::PIXEL_BROADCAST
        | (uint16_t)AlpideRegister::PIXEL_COLSEL1_BASE
        | (uint16_t)AlpideRegister::PIXEL_COLSEL2_BASE
        | (uint16_t)AlpideRegister::PIXEL_ROWSEL_BASE ; // address = 0x487

    if ( clearPulseGating ) {
        address |= (uint16_t)AlpideRegister::PIXEL_PULSESEL_BASE; // address = 0x48f
    }
    
    WriteRegister( address, 0 );
}

#pragma mark - other
//___________________________________________________________________
string TAlpide::DecomposeChipId()
{
    const int moduleID = (fChipId & 0x70) >> 4;
    const int chipPos = (fChipId & 0xf);
    stringstream sschipid;
    sschipid << std::bitset<3> ( moduleID ) << "_" << std::bitset<4> ( chipPos ) ;
    string label = sschipid.str();
    return label;
}





