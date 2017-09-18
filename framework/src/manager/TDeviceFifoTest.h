#ifndef DEVICE_FIFO_TEST_H
#define DEVICE_FIFO_TEST_H

/**
 * \class TDeviceFifoTest
 *
 * \brief This class runs the FIFO test for all enabled chips in the device.
 *
 * \author Andry Rakotozafindrabe
 *
 * For all enabled chips in the device, write various patterns of zero or one in all
 * Region Readout Unit (RRU) DPRAM memories. Then read them, and check if the readback
 * pattern match the written pattern. In that case, the FIFO test is successful. If not,
 * error counters per pattern per chip are displayed.
 *
 * \remark
 * It was recommended to disable Manchester encoding in the config file (CMU settings).
 * However, this lead to ControlInterface sync error with MOSAIC board reading IB chip.
 * Maybe this setting applies only in the case of the DAQ board or OB master chip?
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
 * More safety checks, better initialization, improved code readability,
 * reset of error counters for each new chip and a significant speed up of the 
 * FIFO scan for the MOSAIC board were added in this new class w.r.t. the original code.
 */

#include <unistd.h>
#include <cstdint>
#include <memory>
#include "TDeviceChipVisitor.h"

class TDeviceFifoTest : public TDeviceChipVisitor {
    
    /// index of the current chip being tested
    unsigned int fCurrentChipIndex;
    
    /// counter to keep tracks of the failure for the FIFO test pattern 0x0 per chip
    unsigned int fErrCount0;

    /// counter to keep tracks of the failure for the FIFO test pattern 0x555555 per chip
    unsigned int fErrCount5;
    
    /// counter to keep tracks of the failure for the FIFO test pattern 0xffffff per chip
    unsigned int fErrCountF;
    
    /// index of the current region of the chip whose DPRAM memories are being tested
    unsigned int fRegion;
    
    /// index of the current DPRAM memory location being tested
    unsigned int fOffset;
    
    /// bit pattern currently used for the memory test
    int fBitPattern;
    
public:
    
    /// constructor
    TDeviceFifoTest();

    /// constructor with a TDevice specified
    TDeviceFifoTest( std::shared_ptr<TDevice> aDevice );
    
    /// destructor
    virtual ~TDeviceFifoTest();
        
    /// terminate
    virtual void Terminate();
    
    /// run the FIFO test on all chips of the device
    void Go();
    
private:
    
    /// write to all DPRAM of the current chip
    void WriteMemPerChip();
    
    /// read all DPRAM of the current chip
    void ReadMemPerChip();
    
    /// perform write pattern + readback of all DPRAM of the current chip
    void MemTestPerChip();
    
    /// the various bit patterns to be used for the memory test
    enum MemoryPattern {
        kTEST_ALL_ZERO = 0x0,
        kTEST_ONE_ZERO = 0x555555,
        kTEST_ALL_ONE = 0xffffff
    };
    
    /// id of the last region of the chip
    static const unsigned int MAX_REGION = 31;  // [0 .. 31] 32 regions
    
    /// id of the last DPRAM of a given region of the chip
    static const unsigned int MAX_OFFSET = 127; // [0 .. 127] 128 DPRAM per region
    
protected:
    
    /// configure readout boards
    virtual void ConfigureBoards();
    
    /// configure chips
    virtual void ConfigureChips();
};


#endif
