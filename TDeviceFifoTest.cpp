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
// be pessimistic and put all error counters to the maximum possible value
fErrCount0( (MAX_REGION+1)*(MAX_OFFSET+1) ),
fErrCount5( (MAX_REGION+1)*(MAX_OFFSET+1) ),
fErrCountF( (MAX_REGION+1)*(MAX_OFFSET+1) ),
fRegion( 0 ),
fOffset( 0 ),
fBitPattern( kTEST_ALL_ZERO )
{
    
}

//___________________________________________________________________
TDeviceFifoTest::TDeviceFifoTest( shared_ptr<TDevice> aDevice ) :
TDeviceChipVisitor( aDevice ),
fCurrentChipIndex( 0 ),
// be pessimistic and put all error counters to the maximum possible value
fErrCount0( (MAX_REGION+1)*(MAX_OFFSET+1) ),
fErrCount5( (MAX_REGION+1)*(MAX_OFFSET+1) ),
fErrCountF( (MAX_REGION+1)*(MAX_OFFSET+1) ),
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
void TDeviceFifoTest::SetVerboseLevel( const int level )
{
    if ( level > kTERSE ) {
        cout << "TDeviceFifoTest::SetVerboseLevel() - " << level << endl;
    }
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceFifoTest::Go()
{
    // loop over all chips
    for ( int iChip = 0; iChip < fDevice->GetNChips() ; iChip++ ) {
        
        fCurrentChipIndex = iChip;
        
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
                cout << "\t FIFO scan: region " << ireg ;
            }
            fRegion = ireg;
            
            // loop over all memories
            for ( int iadd = 0; iadd < TDeviceFifoTest::MAX_OFFSET ; iadd++ ) {
                if ( GetVerboseLevel() > kTERSE ) {
                    cout << " offset " << iadd << endl;
                }
                fOffset = iadd;
                if( !MemTest() ) {
                    break;
                }
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

        // count error per chip => reset counters before switching to next chip
        fErrCount0 = (MAX_REGION+1)*(MAX_OFFSET+1);
        fErrCount5 = (MAX_REGION+1)*(MAX_OFFSET+1);
        fErrCountF = (MAX_REGION+1)*(MAX_OFFSET+1);
        
    } // end of the loop on chips
}

//___________________________________________________________________
void TDeviceFifoTest::WriteMem()
{
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw domain_error( "TDeviceFifoTest::WriteMem() - invalid chip index !" );
    }
    if ( fRegion > TDeviceFifoTest::MAX_REGION ) {
        throw domain_error( "TDeviceFifoTest::WriteMem() - invalid region !" );
    }
    if ( fOffset > TDeviceFifoTest::MAX_OFFSET ) {
        throw domain_error( "TDeviceFifoTest::ReadMem() - invalid offset !" );
        return;
    }
    
    uint16_t LowAdd  = (uint16_t)AlpideRegister::RRU_MEB_LSB_BASE | (fRegion << 11) | fOffset;
    uint16_t HighAdd = (uint16_t)AlpideRegister::RRU_MEB_MSB_BASE | (fRegion << 11) | fOffset;
    
    uint16_t LowVal  = fBitPattern & 0xffff;
    uint16_t HighVal = (fBitPattern >> 16) & 0xff;
    
    try {
        (fDevice->GetChip(fCurrentChipIndex))->WriteRegister( LowAdd,  LowVal );
        (fDevice->GetChip(fCurrentChipIndex))->WriteRegister( HighAdd, HighVal );
    } catch ( exception& err ) {
        cerr << err.what() << endl;
        cerr << "TDeviceFifoTest::WriteMem() - chip:region:offset " << fCurrentChipIndex << ":" << fRegion << ":" <<  fOffset << endl;
        throw runtime_error( "TDeviceFifoTest::WriteMem() - failed." );
    }
    
}

//___________________________________________________________________
int TDeviceFifoTest::ReadMem()
{
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw domain_error( "TDeviceFifoTest::ReadMem() - invalid chip index !" );
    }
    if ( fRegion > TDeviceFifoTest::MAX_REGION ) {
        throw domain_error( "TDeviceFifoTest::ReadMem() - invalid region !" );
    }
    if ( fOffset > TDeviceFifoTest::MAX_OFFSET ) {
        throw domain_error( "TDeviceFifoTest::ReadMem() - invalid offset !" );
    }

    uint16_t LowAdd  = (uint16_t)AlpideRegister::RRU_MEB_LSB_BASE | (fRegion << 11) | fOffset;
    uint16_t HighAdd = (uint16_t)AlpideRegister::RRU_MEB_MSB_BASE | (fRegion << 11) | fOffset;
    
    uint16_t LowVal, HighVal;
    try {
        (fDevice->GetChip(fCurrentChipIndex))->ReadRegister( LowAdd, LowVal );
        (fDevice->GetChip(fCurrentChipIndex))->ReadRegister( HighAdd, HighVal );
    } catch ( exception& err ) {
        cerr << err.what() << endl;
        cerr << "TDeviceFifoTest::ReadMem() - chip:region:offset " << fCurrentChipIndex << ":" << fRegion << ":" <<  fOffset << endl;
        throw runtime_error( "TDeviceFifoTest::ReadMem() - failed." );
    }
    
    // Note to self: if you want to shorten the following lines,
    // remember that HighVal is 16 bit and (HighVal << 16) will yield 0
    // :-)
    int aValue = (HighVal & 0xff);
    aValue <<= 16;
    aValue |= LowVal;

    return aValue;
}

//___________________________________________________________________
void TDeviceFifoTest::MemReadback()
{
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw domain_error( "TDeviceFifoTest::MemReadback() - invalid chip index !" );
    }
    if ( fRegion > TDeviceFifoTest::MAX_REGION ) {
        throw domain_error( "TDeviceFifoTest::MemReadback() - invalid region !" );
    }
    if ( fOffset > TDeviceFifoTest::MAX_OFFSET ) {
        throw domain_error( "TDeviceFifoTest::MemReadback() - invalid offset !" );
    }

    // MemReadback() test fails completely when one can not properly
    // write or read the pixel control register being tested
    
    try {
        WriteMem();
    } catch ( exception& err ) {
        cerr << err.what() << endl;
        throw runtime_error( "TDeviceFifoTest::MemReadback() - failed !" );
    }

    int aValue = 1;
    try {
        aValue = ReadMem();
    } catch ( exception& err ) {
        cerr << err.what() << endl;
        throw runtime_error( "TDeviceFifoTest::MemReadback() - failed !" );
    }
    
    // MemReadback() test is working but the memory being tested has some
    // malfunction
    
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
    
    // everything is working fine, so we can decrease the counter errors
    
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
bool TDeviceFifoTest::MemTest()
{
    // If we can not write or read the memory under test for the
    // first pattern, then we don't lose time testing the next one.
    // This strategy was chosen because this seems to happen only
    // when there is some general communication problem (bad setting?)
    // between the chip and the readout board.
    
    fBitPattern = kTEST_ALL_ZERO;
    try {
        MemReadback();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0x0" << endl;
        cerr << err.what() << endl;
        return false;
    }

    fBitPattern = kTEST_ONE_ZERO;
    try {
        MemReadback();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0x555555" << endl;
        cerr << err.what() << endl;
        return false;
    }

    fBitPattern = kTEST_ALL_ONE;
    try {
        MemReadback();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0xffffff" << endl;
        cerr << err.what() << endl;
        return false;
    }
    return true;
}

