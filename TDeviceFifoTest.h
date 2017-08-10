#ifndef DEVICE_FIFO_TEST_H
#define DEVICE_FIFO_TEST_H

/**
 * \class TDeviceFifoTest
 *
 * \brief This class runs the FIFO test for all enabled chips in the device.
 *
 * \author Andry Rakotozafindrabe
 *
 * \remark
 * It is recommended to disable Manchester encoding in the config file (CMU settings).
 *
 * \note
 * cmuconfig = 0x60 for FIFO test (OB?)
 * i.e.
 * const int fPreviousId = 0x10;
 * const bool fInitialToken = false;
 * const bool fDisableManchester = true;
 * const bool fEnableDdr = true;
 *
 * \note 
 * Most of the code was moved from the original main_fifo.cpp written by ITS team.
 * More safety checks, better initialization, improved code readability and 
 * reset of error counters for each new chip were added in this new class w.r.t. the 
 * original code.
 */

#include <memory>
#include "TDeviceChipVisitor.h"

class TDeviceFifoTest : public TDeviceChipVisitor {
    
    /// index of the current chip being tested
    int fCurrentChipIndex;
    
    /// counter to keep tracks of the failure for the FIFO test pattern 0x0 per chip
    int fErrCount0;

    /// counter to keep tracks of the failure for the FIFO test pattern 0x555555 per chip
    int fErrCount5;
    
    /// counter to keep tracks of the failure for the FIFO test pattern 0xffffff per chip
    int fErrCountF;
    
    /// index of the current region of the chip whose pixel memories are being tested
    int fRegion;
    
    /// index of the current pixel control register being tested
    int fOffset;
    
    /// bit pattern currently used for the memory test and written to the pixel register
    int fBitPattern;
    
public:
    
    /// constructor
    TDeviceFifoTest();

    /// constructor with a TDevice specified
    TDeviceFifoTest( std::shared_ptr<TDevice> aDevice );
    
    /// destructor
    virtual ~TDeviceFifoTest();

    /// configure all chips for FIFO test
    virtual void DoConfigureCMU();
    
    /// decorate the base class SetVerboseLevel() method
    virtual void SetVerboseLevel( const int level );
    
    /// run the FIFO test on all chips of the device
    void Go();
    
private:
    
    /// write to a given pixel control register
    void WriteMem();
    
    /// read a given pixel control register
    int  ReadMem();
    
    /// perform write pattern + readback in a given pixel control register
    void MemReadback();
    
    /// perform MemReadback() for all patterns in a given pixel control register
    bool MemTest();
    
    enum MemoryPattern {
        kTEST_ALL_ZERO = 0x0,
        kTEST_ONE_ZERO = 0x555555,
        kTEST_ALL_ONE = 0xffffff
    };
    
    static const int MAX_REGION = 31;
    static const int MAX_OFFSET = 127;
};


#endif
