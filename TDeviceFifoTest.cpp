#include "AlpideDictionary.h"
#include "TAlpide.h"
#include "TChipConfig.h"
#include "TDevice.h"
#include "TDeviceFifoTest.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include <stdexcept>
#include <iostream>
#include <bitset>

using namespace std;

//___________________________________________________________________
TDeviceFifoTest::TDeviceFifoTest() : TDeviceChipVisitor(),
fCurrentChipIndex( 0 ),
fErrCount0( 0 ),
fErrCount5( 0 ),
fErrCountF( 0 ),
fRegion( 0 ),
fOffset( 0 ),
fBitPattern( kTEST_ALL_ZERO )
{
    
}

//___________________________________________________________________
TDeviceFifoTest::TDeviceFifoTest( shared_ptr<TDevice> aDevice ) :
TDeviceChipVisitor( aDevice ),
fCurrentChipIndex( 0 ),
fErrCount0( 0 ),
fErrCount5( 0 ),
fErrCountF( 0 ),
fRegion( 0 ),
fOffset( 0 ),
fBitPattern( kTEST_ALL_ZERO )
{
    
}

//___________________________________________________________________
TDeviceFifoTest::~TDeviceFifoTest()
{
    
}

//___________________________________________________________________
void TDeviceFifoTest::DoConfigureCMU()
{
    if ( !fDevice ) {
        throw runtime_error( "TDeviceFifoTest::DoConfigureCMU() - can not use a null pointer !" );
    }
    if ( fDevice->GetNChips() == 0 ) {
        throw runtime_error( "TDeviceFifoTest::DoConfigureCMU() - no chip found !" );
    }

    shared_ptr<TReadoutBoard> myBoard = fDevice->GetBoard(0);
    if ( !myBoard ) {
        throw runtime_error( "TDeviceFifoTest::DoConfigureCMU() - no readout board found!" );
    }
    myBoard->SendOpCode( (uint16_t)AlpideOpCode::GRST );
    myBoard->SendOpCode( (uint16_t)AlpideOpCode::PRST );

    for (int i = 0; i < fDevice->GetNChips(); i ++) {
        fDevice->GetChip(i)->WriteControlReg( AlpideChipMode::CONFIG );
        fDevice->GetChip(i)->ConfigureCMU();
    }

    myBoard->SendOpCode( (uint16_t)AlpideOpCode::RORST );
}

//___________________________________________________________________
void TDeviceFifoTest::Go()
{
    // loop over all chips
    for ( int iChip = 0; iChip < fDevice->GetNChips() ; iChip++ ) {
        
        fCurrentChipIndex = iChip;
        
        // count error per chip => reset counters
        fErrCount0 = 0;
        fErrCount5 = 0;
        fErrCountF = 0;
        
        if ( !((fDevice->GetChipConfig(iChip))->IsEnabled()) ) {
            if ( GetVerboseLevel() > kTERSE ) {
                cout << "TDeviceFifoTest::Go() - chip # "
                << fCurrentChipIndex << " : disabled chip, skipped." <<  endl;
            }
            return;
        }
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceFifoTest::Go() - testing chip # "
                 << fCurrentChipIndex <<  endl;
        }
        
        // loop over all regions
        for ( int ireg = 0 ; ireg < TDeviceFifoTest::MAX_REGION ; ireg++ ) {
            if ( GetVerboseLevel() > kTERSE ) {
                cout << "\t FIFO scan: region " << ireg << endl;
            }

            // loop over all memories
            for ( int iadd = 0; iadd < TDeviceFifoTest::MAX_OFFSET ; iadd++ ) {
                MemTest();
            } // end of loop on addresses

        } // end of the loop on region
        
        if ( fErrCount0 + fErrCount5 + fErrCountF > 0 ) {
            cout << "TDeviceFifoTest::Go() - FIFO test finished for chip # "
            << fCurrentChipIndex << endl;
            cout << "TDeviceFifoTest::Go() - error counters : " << endl;
            cout << "\t pattern 0x0:      " << fErrCount0 << endl;
            cout << "\t pattern 0x555555: " << fErrCount5 << endl;
            cout << "\t pattern 0xffffff: " << fErrCountF << endl;
            cout << "(total number of tested memories: 32 * 128 = 4096)" << endl;
        } else {
            cout << "TDeviceFifoTest::Go() - FIFO test successful for chip # "
            << fCurrentChipIndex << endl;
        }

    } // end of the loop on chips
}

//___________________________________________________________________
void TDeviceFifoTest::WriteMem()
{
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw runtime_error( "TDeviceFifoTest::WriteMem() - invalid chip index !" );
    }
    if ( fRegion > TDeviceFifoTest::MAX_REGION ) {
        cout << "TDeviceFifoTest::WriteMem() - invalid region " << fRegion << endl;
        return;
    }
    if ( fOffset > TDeviceFifoTest::MAX_OFFSET ) {
        cout << "TDeviceFifoTest::WriteMem() - invalid offset " << fOffset << endl;
        return;
    }
    
    uint16_t LowAdd  = (uint16_t)AlpideRegister::RRU_MEB_LSB_BASE | (fRegion << 11) | fOffset;
    uint16_t HighAdd = (uint16_t)AlpideRegister::RRU_MEB_MSB_BASE | (fRegion << 11) | fOffset;
    
    uint16_t LowVal  = fBitPattern & 0xffff;
    uint16_t HighVal = (fBitPattern >> 16) & 0xff;
    
    try {
        (fDevice->GetChip(fCurrentChipIndex))->WriteRegister( LowAdd,  LowVal );
        (fDevice->GetChip(fCurrentChipIndex))->WriteRegister( HighAdd, HighVal );
    } catch ( ... ) {
        cout << "TDeviceFifoTest::WriteMem() - chip # " << fCurrentChipIndex << " , cannot write chip register." << endl;
        throw runtime_error( "TDeviceFifoTest::WriteMem() - failed." );
    }
    
    // be pessimistic:
    // assume each successful WriteMem() will lead to a failed MemReadback()
    if ( fBitPattern == kTEST_ALL_ZERO ) {
        fErrCount0++;
    }
    if ( fBitPattern == kTEST_ONE_ZERO ) {
        fErrCount5++;
    }
    if ( fBitPattern == kTEST_ALL_ONE ) {
        fErrCountF++;
    }
}

//___________________________________________________________________
int TDeviceFifoTest::ReadMem()
{
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw runtime_error( "TDeviceFifoTest::ReadMem() - invalid chip index !" );
    }
    int aValue = 1;
    if ( fRegion > TDeviceFifoTest::MAX_REGION ) {
        cout << "TDeviceFifoTest::ReadMem() - invalid region " << fRegion << endl;
        return aValue;
    }
    if ( fOffset > TDeviceFifoTest::MAX_OFFSET ) {
        cout << "TDeviceFifoTest::ReadMem() - invalid offset " << fOffset << endl;
        return aValue;
    }

    uint16_t LowAdd  = (uint16_t)AlpideRegister::RRU_MEB_LSB_BASE | (fRegion << 11) | fOffset;
    uint16_t HighAdd = (uint16_t)AlpideRegister::RRU_MEB_MSB_BASE | (fRegion << 11) | fOffset;
    
    uint16_t LowVal, HighVal;
    try {
        (fDevice->GetChip(fCurrentChipIndex))->ReadRegister( LowAdd, LowVal );
        (fDevice->GetChip(fCurrentChipIndex))->ReadRegister( HighAdd, HighVal );
    } catch ( ... ) {
        cout << "TDeviceFifoTest::ReadMem() - cannot read chip register. Chip # " << fCurrentChipIndex << "skipped." << endl;
        return aValue;
    }
    
    // Note to self: if you want to shorten the following lines,
    // remember that HighVal is 16 bit and (HighVal << 16) will yield 0
    // :-)
    aValue = (HighVal & 0xff);
    aValue <<= 16;
    aValue |= LowVal;

    return aValue;
}

//___________________________________________________________________
void TDeviceFifoTest::MemReadback()
{
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw runtime_error( "TDeviceFifoTest::MemReadback() - invalid chip index !" );
    }
    if ( fRegion > TDeviceFifoTest::MAX_REGION ) {
        cout << "TDeviceFifoTest::MemReadback() - invalid region " << fRegion << endl;
        return;
    }
    if ( fOffset > TDeviceFifoTest::MAX_OFFSET ) {
        cout << "TDeviceFifoTest::MemReadback() - invalid offset " << fOffset << endl;
        return;
    }

    try {
        WriteMem();
    } catch ( ... ) {
        // skip ReadMem() for that chip that had problems for the WriteMem();
        return;
    }
    int aValue = 1;
    aValue = ReadMem();
    
    if ( aValue != fBitPattern ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceFifoTest::MemReadback() - Error in mem "
            << fRegion
            << "/0x" << std::hex << fOffset
            << ": wrote " << fBitPattern
            << ", read: " << aValue << std::dec << endl;
        }
        return;
    }
    if ( fBitPattern == kTEST_ALL_ZERO ) {
        fErrCount0--;
    }
    if ( fBitPattern == kTEST_ONE_ZERO ) {
        fErrCount5--;
    }
    if ( fBitPattern == kTEST_ALL_ONE ) {
        fErrCountF--;
    }
}

//___________________________________________________________________
void TDeviceFifoTest::MemTest()
{
    fBitPattern = kTEST_ALL_ZERO;
    MemReadback();

    fBitPattern = kTEST_ONE_ZERO;
    MemReadback();

    fBitPattern = kTEST_ALL_ONE;
    MemReadback();
}

