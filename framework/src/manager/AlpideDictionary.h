#ifndef ALPIDE_DICTIONARY_H
#define ALPIDE_DICTIONARY_H

#include <cstdint>

enum class AlpideRegister : std::uint16_t {
    // addresses of periphery control registers (alpide manual, section 3.2.2, page 36)
    COMMAND             = 0x0,
    MODE_CONTROL        = 0x1,
    DISABLE_REGION_LOW  = 0x2,
    DISABLE_REGION_HIGH = 0x3,
    
    FROMU_CONFIG1       = 0x4,
    FROMU_CONFIG2       = 0x5,
    FROMU_CONFIG3       = 0x6,
    FROMU_PULSING1      = 0x7,
    FROMU_PULSING2      = 0x8,
    FROMU_STATUS1       = 0x9,
    FROMU_STATUS2       = 0xa,
    FROMU_STATUS3       = 0xb,
    FROMU_STATUS4       = 0xc,
    FROMU_STATUS5       = 0xd,

    DACS_CLKIO_BUF      = 0xe,
    DACS_CMUIO_BUF      = 0xf,

    CMU_DMU_CONFIG      = 0x10,
    CMU_DMU_STATUS      = 0x11,
    DMU_FIFO_LOW        = 0x12,
    DMU_FIFO_HIGH       = 0x13,

    DTU_CONFIG          = 0x14,
    DTU_DACS            = 0x15,
    DTU_PLL_LOCK1       = 0x16,
    DTU_PLL_LOCK2       = 0x17,
    DTU_TEST1           = 0x18,
    DTU_TEST2           = 0x19,
    DTU_TEST3           = 0x1a,
    BUSY_MINWIDTH       = 0x1b,

    // addresses of dacs and monitoring control registers (alpide manual, section 3.2.5, page 50)
    ANALOGMON       = 0x600,
    VRESETP         = 0x601,
    VRESETD         = 0x602,
    VCASP           = 0x603,
    VCASN           = 0x604,
    VPULSEH         = 0x605,
    VPULSEL         = 0x606,
    VCASN2          = 0x607,
    VCLIP           = 0x608,
    VTEMP           = 0x609,
    IAUX2           = 0x60a,
    IRESET          = 0x60b,
    IDB             = 0x60c,
    IBIAS           = 0x60d,
    ITHR            = 0x60e,
    BYPASS_BUFFER   = 0x60f,
    ADC_CONTROL     = 0x610,
    ADC_DAC_INPUT   = 0x611,
    ADC_CALIB       = 0x612,
    ADC_AVSS        = 0x613,
    ADC_DVSS        = 0x614,
    ADC_AVDD        = 0x615,
    ADC_DVDD        = 0x616,
    ADC_VCASN       = 0x617,
    ADC_VCASP       = 0x618,
    ADC_VPULSEH     = 0x619,
    ADC_VPULSEL     = 0x61a,
    ADC_VRESETP     = 0x61b,
    ADC_VRESETD     = 0x61c,
    ADC_VCASN2      = 0x61d,
    ADC_VCLIP       = 0x61e,
    ADC_VTEMP       = 0x61f,
    ADC_ITHR        = 0x620,
    ADC_IREF        = 0x621,
    ADC_IDB         = 0x622,
    ADC_IBIAS       = 0x623,
    ADC_IAUX2       = 0x624,
    ADC_IRESET      = 0x625,
    ADC_BG2V        = 0x626,
    ADC_T2V         = 0x627,

    // addresses of test and debug control registers (alpide manual, section 3.2.6, page 54)
    SEU_ERROR_COUNT = 0x700,
    TEST_CONTROL    = 0x701,
    BMU_DEBUG       = 0x702,
    DMU_DEBUG       = 0x703,
    TRU_DEBUG       = 0x704,
    RRU_DEBUG       = 0x705,
    FROMU_DEBUG     = 0x706,
    ADC_DEBUG       = 0x707,
    // (alpide manual, section 3.2.3, page 46)
    // region register base addresses (address for region 0),
    // to be ORed with 0x0800 - 0xf800 for regions 1 - 31 or
    // with 0x80 for region broadcast
    RRU_MEB_LSB_BASE   = 0x100,    // to be ORed with 0x00 - 0x7f for the different RAM words
    RRU_MEB_MSB_BASE   = 0x200,    // same here
    DCOL_DISABLE_BASE  = 0x300,
    REGION_STATUS_BASE = 0x301,

    // global pixel configuration register (alpide manual, page 49)
    PIXEL_CONFIG        = 0x500,

    // used to address the pixel control registers (alpide manual, section 3.2.4, page 48)
    PIXEL_BROADCAST     = 0x080,
    PIXEL_COLSEL1_BASE  = 0x401,
    PIXEL_COLSEL2_BASE  = 0x402,
    PIXEL_ROWSEL_BASE   = 0x404,
    PIXEL_PULSESEL_BASE = 0x408
};

// command codes recognized by the Command Register
enum class AlpideOpCode : std::uint16_t {
    TRIGGER1     = 0xb1,   // trigger command
    TRIGGER2     = 0x55,   // trigger command
    TRIGGER3     = 0xc9,   // trigger command
    TRIGGER4     = 0x2d,   // trigger command
    GRST         = 0xd2,   // chip global reset
    PRST         = 0xe4,   // pixel matrix reset
    PULSE        = 0x78,   // pixel matrix pulse
    BCRST        = 0x36,   // bunch counter reset
    DEBUG        = 0xaa,   // store snapshot into debug registers
    RORST        = 0x63,   // readout (RRU/TRU/DMU) reset
    WROP         = 0x9c,   // start unicast or multicast write
    RDOP         = 0x4e,   // start unicast read
    CMU_CLEARERR = 0xff00, // clear CMU error flags
    FIFOTEST     = 0xff01, // starts region memory test
    LOADOBDEFCFG = 0xff02, // loads default config for the OB module
    XOFF         = 0xff10, // stops sending data off-chip
    XON          = 0xff11, // resume data sending
    ADCMEASURE   = 0xff20  // start ADC measure
};

// PIXCNFG_REGSEL (bit 0 in the global Pixel Configuration Register)
// (alpide manual, section 3.2.4, page 48)
// (see also alpide manual, section 3.6, page 66)
enum class AlpidePixConfigReg {
    MASK_ENABLE   = 0x0,
    PULSE_ENABLE  = 0x1
};

enum class AlpideTestPulseMode { // previously named AlpidePulseType
    DIGITAL  = 0,
    ANALOGUE = 1
};

// Bits 5:4 of the Mode Control Register
enum class AlpideIBSerialLinkSpeed {
    IB400   = 0,
    IB600   = 1,
    IB1200  = 2
};    
    
// Bits 1:0 of the Mode Control Register
enum class AlpideChipMode {
    CONFIG      = 0, // (readout disabled)
    TRIGGERED   = 1, // (readout enabled)
    CONTINUOUS  = 2  // (readout enabled)
};
    
// Bits 4:3 of the DTU Test Register 1
enum class AlpidePrbs7Mode {
    PRBS1200   = 0, // 2b'00
    PRBS400    = 1, // 2b'01
    PRBS600    = 2, // 2b'10
    PRBSRST    = 3 // 2b'11 (synchronous reset of the PRBS-7 pattern generator)
};

enum class AlpideDACMonIref {
    IREF_025uA = 0,
    IREF_075uA = 1,
    IREF_100uA = 2,
    IREF_125uA = 3
};
enum class AlpideADCMode {
    MANUAL      = 0,
    CALIBRATE   = 1,
    AUTO        = 2,
    SUPERMANUAL = 3
};
enum class AlpideADCComparator {
    COMP_180uA = 0,
    COMP_190uA = 1,
    COMP_296uA = 2,
    COMP_410uA = 3
};
enum class AlpideADCRampSpeed {
    RAMP_500ms = 0,
    RAMP_1us = 1,
    RAMP_2us = 2,
    RAMP_4us = 3
};
enum class AlpideADCInput {
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

#endif
