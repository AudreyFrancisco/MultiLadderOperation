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
#include <array>

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
{ }

//___________________________________________________________________
TDeviceFifoTest::~TDeviceFifoTest()
{ }

//___________________________________________________________________
void TDeviceFifoTest::Go()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceFifoTest::Go() - not initialized ! Please use Init() first." );
    }

    // loop over all chips
    for ( unsigned int iChip = 0; iChip < fDevice->GetNChips() ; iChip++ ) {

        fCurrentChipIndex = iChip;

        // count error per chip => reset counters for each new chip
        fErrCount0 = (MAX_REGION+1)*(MAX_OFFSET+1);
        fErrCount5 = (MAX_REGION+1)*(MAX_OFFSET+1);
        fErrCountF = (MAX_REGION+1)*(MAX_OFFSET+1);

        if ( !((fDevice->GetChipConfig(iChip))->IsEnabled()) ) {
            if ( GetVerboseLevel() > kTERSE ) {
                cout << std::dec 
                     << "TDeviceFifoTest::Go() - Board " << fDevice->GetBoardIndexByChip(fCurrentChipIndex)
                     << " RCV " << fDevice->GetChipReceiverById( fDevice->GetChip(fCurrentChipIndex)->GetChipId() )
                     << " Ladder ID " << fDevice->GetLadderId()
                     << " Chip ID "<< fDevice->GetChip(fCurrentChipIndex)->GetChipId()
                     << " : disabled chip, skipped." <<  endl;
            }
            continue;
        }
        try {
            fIdx = fDevice->GetWorkingChipIndex( iChip );
        } catch ( std::exception &err ) {
            cerr << "TDeviceFifoTest::Go() - " << err.what() << endl;
            exit( EXIT_FAILURE );
        }
        
        if ( GetVerboseLevel() > kSILENT ) {
            cout << std::dec 
                 << "TDeviceFifoTest::Go() - Testing : Board " << fIdx.boardIndex
                 << " RCV " << fIdx.dataReceiver
                 << " Ladder ID " << fIdx.ladderId
                 << " Chip ID "<< fIdx.chipId
                 <<  endl;
        }
        
        // write and read the DPRAM memories of the RRU modules can only
        // be done when the chip is in configuration mode
        fDevice->GetChip(fCurrentChipIndex)->ActivateConfigMode();
        
        MemTestPerChip();
        
        if ( fErrCount0 + fErrCount5 + fErrCountF > 0 ) {
            cout << std::dec 
                 << "TDeviceFifoTest::Go() - FIFO test finished for : Board " << fIdx.boardIndex
                 << " RCV " << fIdx.dataReceiver
                 << " Ladder ID " << fIdx.ladderId
                 << " Chip ID "<< fIdx.chipId
                 <<  endl;
            cout << "TDeviceFifoTest::Go() - error counters : " << endl;
            cout << "\t pattern 0x0:      " << fErrCount0 << endl;
            cout << "\t pattern 0x555555: " << fErrCount5 << endl;
            cout << "\t pattern 0xffffff: " << fErrCountF << endl;
            cout << "(total number of tested memories: 32 * 128 = 4096)" << endl;
        } else {
            cout << std::dec 
                 << "TDeviceFifoTest::Go() - FIFO test successful for : Board " << fIdx.boardIndex
                 << " RCV " << fIdx.dataReceiver
                 << " Ladder ID " << fIdx.ladderId
                 << " Chip ID "<< fIdx.chipId
                 <<  endl;
        }
    } // end of the loop on chips    
}

//___________________________________________________________________
void TDeviceFifoTest::WriteMemPerChip()
{
    
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw domain_error( "TDeviceFifoTest::WriteMemPerChip() - Invalid chip index !" );
    }
    
    if ( !((fDevice->GetChipConfig(fCurrentChipIndex))->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << std::dec 
                 << "TDeviceFifoTest::WriteMemPerChip() - Board " << fDevice->GetBoardIndexByChip(fCurrentChipIndex)
                     << " RCV " << fDevice->GetChipReceiverById( fDevice->GetChip(fCurrentChipIndex)->GetChipId() )
                     << " Ladder ID " << fDevice->GetLadderId()
                     << " Chip ID "<< fDevice->GetChip(fCurrentChipIndex)->GetChipId()
                     << " : disabled chip, skipped." <<  endl;
        }
        return;
    }
    if ( GetVerboseLevel() > kTERSE ) {
        switch ( fBitPattern ) {
            case (int)kTEST_ALL_ZERO:
                cout << "TDeviceFifoTest::WriteMemPerChip() - pattern 0x0,      Board " << fIdx.boardIndex
                     << " RCV " << fIdx.dataReceiver
                     << " Ladder ID " << fIdx.ladderId
                     << " Chip ID "<< fIdx.chipId << endl;
                break;
            case (int)kTEST_ONE_ZERO:
                cout << "TDeviceFifoTest::WriteMemPerChip() - pattern 0x555555, Board " << fIdx.boardIndex
                     << " RCV " << fIdx.dataReceiver
                     << " Ladder ID " << fIdx.ladderId
                     << " Chip ID "<< fIdx.chipId << endl;
                break;
            case (int)kTEST_ALL_ONE:
                cout << "TDeviceFifoTest::WriteMemPerChip() - pattern 0xffffff, Board " << fIdx.boardIndex
                     << " RCV " << fIdx.dataReceiver
                     << " Ladder ID " << fIdx.ladderId
                     << " Chip ID "<< fIdx.chipId << endl;
                break;
            default:
                throw runtime_error( "TDeviceFifoTest::WriteMemPerChip() - Wrong bit pattern." );
                break;
        }
    }
    
    // boolean used to queue all write request for the MOSAIC board
    // (in order to increase the speed of the FIFO test)
    
    bool doExecute = false;
    
    // loop over all regions
    
    for ( unsigned int ireg = 0 ; ireg < TDeviceFifoTest::MAX_REGION+1 ; ireg++ ) {

        fRegion = ireg;
        
        // loop over all memories
        
        for ( unsigned int iadd = 0; iadd < TDeviceFifoTest::MAX_OFFSET+1 ; iadd++ ) {

            fOffset = iadd;

            if ( GetVerboseLevel() > kVERBOSE ) {
                cout << "\t writing board:rcv:ladder:chip:region:offset " << std::dec 
                     << fIdx.boardIndex
                     << ":" << fIdx.dataReceiver
                     << ":" << fIdx.ladderId
                     << ":" << fIdx.chipId 
                     << ":" << fRegion 
                     << ":" << fOffset << endl;
            }
            
            if (  ( fRegion == TDeviceFifoTest::MAX_REGION )
                && ( fOffset == TDeviceFifoTest::MAX_OFFSET ) ) {
                // execute all write requests in the queue at once for the MOSAIC board
                doExecute = true;
            }
            
            uint16_t LowAdd  = (uint16_t)AlpideRegister::RRU_MEB_LSB_BASE | (fRegion << 11) | fOffset;
            uint16_t HighAdd = (uint16_t)AlpideRegister::RRU_MEB_MSB_BASE | (fRegion << 11) | fOffset;
            
            uint16_t LowVal  = fBitPattern & 0xffff;
            uint16_t HighVal = (fBitPattern >> 16) & 0xff;
            
            try {
                (fDevice->GetChip(fCurrentChipIndex))->WriteRegister( LowAdd,  LowVal, doExecute );
                (fDevice->GetChip(fCurrentChipIndex))->WriteRegister( HighAdd, HighVal, doExecute );
            } catch ( exception& err ) {
                cerr << err.what() << endl;
                cerr << "TDeviceFifoTest::WriteMemPerChip() - board:rcv:ladder:chip:region:offset " << std::dec
                     << fIdx.boardIndex
                     << ":" << fIdx.dataReceiver
                     << ":" << fIdx.ladderId
                     << ":" << fIdx.chipId 
                     << ":" << fRegion 
                     << ":" << fOffset << endl;
                throw runtime_error( "TDeviceFifoTest::WriteMemPerChip() - failed." );
            }
        }
    }
}

//___________________________________________________________________
void TDeviceFifoTest::ReadMemPerChip()
{
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw domain_error( "TDeviceFifoTest::ReadMemPerChip()  - Invalid chip index !" );
    }
    
    if ( !((fDevice->GetChipConfig(fCurrentChipIndex))->IsEnabled()) ) {
        if ( GetVerboseLevel() > kTERSE ) {
          cout << std::dec 
                 << "TDeviceFifoTest::ReadMemPerChip() - Board " << fDevice->GetBoardIndexByChip(fCurrentChipIndex)
                     << " RCV " << fDevice->GetChipReceiverById( fDevice->GetChip(fCurrentChipIndex)->GetChipId() )
                     << " Ladder ID " << fDevice->GetLadderId()
                     << " Chip ID "<< fDevice->GetChip(fCurrentChipIndex)->GetChipId()
                     << " : disabled chip, skipped." <<  endl;      
        }
        return;
    }
    if ( GetVerboseLevel() > kTERSE ) {
        switch ( fBitPattern ) {
            case (int)kTEST_ALL_ZERO:
               cout << "TDeviceFifoTest::ReadMemPerChip()  - pattern 0x0,      Board " << fIdx.boardIndex
                     << " RCV " << fIdx.dataReceiver
                     << " Ladder ID " << fIdx.ladderId
                     << " Chip ID "<< fIdx.chipId << endl;
                break;
            case (int)kTEST_ONE_ZERO:
                cout << "TDeviceFifoTest::ReadMemPerChip()  - pattern 0x555555, Board " << fIdx.boardIndex
                     << " RCV " << fIdx.dataReceiver
                     << " Ladder ID " << fIdx.ladderId
                     << " Chip ID "<< fIdx.chipId << endl;
                break;
            case (int)kTEST_ALL_ONE:
                cout << "TDeviceFifoTest::ReadMemPerChip()  - pattern 0xffffff, Board " << fIdx.boardIndex
                     << " RCV " << fIdx.dataReceiver
                     << " Ladder ID " << fIdx.ladderId
                     << " Chip ID "<< fIdx.chipId << endl;
                break;
            default:
                throw runtime_error( "TDeviceFifoTest::ReadMemPerChip()  - wrong bit pattern." );
                break;
        }
    }
    
    // boolean used to queue all read request for the MOSAIC board
    // (in order to increase the speed of the FIFO test)
    
    bool doExecute = false;
    
    // arrays used to store what was read in the various memory locations
    // (needed since we queue the read requests for the MOSAIC board)
    
    array<uint16_t, (MAX_REGION+1)*(MAX_OFFSET+1)> LowVal;
    LowVal.fill(0);

    array<uint16_t, (MAX_REGION+1)*(MAX_OFFSET+1)> HighVal;
    HighVal.fill(0);
    
    // queue read requests and execute them at once
    
    unsigned int index = 0;
    // loop over all regions
    for ( unsigned int ireg = 0 ; ireg < TDeviceFifoTest::MAX_REGION+1 ; ireg++ ) {
        
        fRegion = ireg;
        
        // loop over all memories
        for ( unsigned int iadd = 0; iadd < TDeviceFifoTest::MAX_OFFSET+1 ; iadd++ ) {
            
            fOffset = iadd;

            if ( GetVerboseLevel() > kVERBOSE ) {
                cout << "\t reading board:rcv:ladder:chip:region:offset " << std::dec 
                     << fIdx.boardIndex
                     << ":" << fIdx.dataReceiver
                     << ":" << fIdx.ladderId
                     << ":" << fIdx.chipId 
                     << ":" << fRegion 
                     << ":" << fOffset << endl;
            }
            
            if (  ( fRegion == TDeviceFifoTest::MAX_REGION )
                && ( fOffset == TDeviceFifoTest::MAX_OFFSET ) ) {
                // execute all read requests in the queue for the MOSAIC board
                doExecute = true;
            }

            uint16_t LowAdd  = (uint16_t)AlpideRegister::RRU_MEB_LSB_BASE | (fRegion << 11) | fOffset;
            uint16_t HighAdd = (uint16_t)AlpideRegister::RRU_MEB_MSB_BASE | (fRegion << 11) | fOffset;
            
            //uint16_t LowVal, HighVal;
            try {
                (fDevice->GetChip(fCurrentChipIndex))->ReadRegister( LowAdd, LowVal.at(index), doExecute );
                (fDevice->GetChip(fCurrentChipIndex))->ReadRegister( HighAdd, HighVal.at(index), doExecute );
            } catch ( exception& err ) {
                cerr << err.what() << endl;
                cerr << "TDeviceFifoTest::ReadMemPerChip() - board:rcv:ladder:chip:region:offset " << std::dec
                     << fIdx.boardIndex
                     << ":" << fIdx.dataReceiver
                     << ":" << fIdx.ladderId
                     << ":" << fIdx.chipId 
                     << ":" << fRegion 
                     << ":" << fOffset << endl;
                throw runtime_error( "TDeviceFifoTest::ReadMemPerChip() - failed." );
            }
            
            index++;
        }
        // end loop over all memories
    } // end loop over all regions
    
    // compare read back values to written values
    
    index = 0;
    for ( unsigned int ireg = 0 ; ireg < TDeviceFifoTest::MAX_REGION+1 ; ireg++ ) {
        for ( unsigned int iadd = 0; iadd < TDeviceFifoTest::MAX_OFFSET+1 ; iadd++ ) {
            
            // Note to self: if you want to shorten the following lines,
            // remember that HighVal is 16 bit and (HighVal << 16) will yield 0
            // :-)
            int aValue = (HighVal.at(index) & 0xff);
            aValue <<= 16;
            aValue |= LowVal.at(index);
            
            // Readback process worked but the tested memory location may had
            // some malfunction
            
            bool success = true;
            if ( aValue != fBitPattern ) {
                if ( GetVerboseLevel() > kSILENT ) {
                    cout << "TDeviceFifoTest::ReadMemPerChip() - Error in mem board:rcv:ladder:chip:region:offset "
                     << std::dec
                     << fIdx.boardIndex
                     << ":" << fIdx.dataReceiver
                     << ":" << fIdx.ladderId
                     << ":" << fIdx.chipId 
                     << ":" << fRegion 
                     << ":" << fOffset
                    << " : wrote " << std::hex << fBitPattern
                    << " , read " << std::hex << aValue << endl;
                }
                success = false;
            }
            
            // everything is working fine, so we can decrease the counter errors
            // for this chip
            
            if ( success ) {
                switch ( fBitPattern ) {
                    case (int)kTEST_ALL_ZERO:
                        fErrCount0--;
                        break;
                    case (int)kTEST_ONE_ZERO:
                        fErrCount5--;
                        break;
                    case (int)kTEST_ALL_ONE:
                        fErrCountF--;
                        break;
                    default:
                        break;
                }
            }
            index++;
        }// end loop over all memories
    } // end loop over all regions
}

//___________________________________________________________________
void TDeviceFifoTest::MemTestPerChip()
{
    
    if ( fCurrentChipIndex >= fDevice->GetNChips() ) {
        throw domain_error( "TDeviceFifoTest::MemTestPerChip() - invalid chip index !" );
    }

    // If we can not write or read the memory location under test for
    // the first pattern, then we don't lose time testing the next one.
    
    fBitPattern = kTEST_ALL_ZERO;
    
    try {
        WriteMemPerChip();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0x0 failed" << endl;
        cerr << err.what() << endl;
        return;
    }
    try {
        ReadMemPerChip();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0x0 failed" << endl;
        cerr << err.what() << endl;
        return;
    }

    fBitPattern = kTEST_ONE_ZERO;

    try {
        WriteMemPerChip();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0x555555 failed" << endl;
        cerr << err.what() << endl;
        return;
    }
    try {
        ReadMemPerChip();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0x555555 failed" << endl;
        cerr << err.what() << endl;
        return;
    }

    fBitPattern = kTEST_ALL_ONE;

    try {
        WriteMemPerChip();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0xffffff failed" << endl;
        cerr << err.what() << endl;
        return;
    }
    try {
        ReadMemPerChip();
    } catch ( exception& err ) {
        cerr << "TDeviceFifoTest::MemTest() - pattern 0xffffff failed" << endl;
        cerr << err.what() << endl;
        return;
    }
}

//___________________________________________________________________
void TDeviceFifoTest::ConfigureBoards()
{
    // nothing to do to be able to run a FIFO test
}

//___________________________________________________________________
void TDeviceFifoTest::ConfigureChips()
{
    DoActivateConfigMode();
    DoBaseConfig();
}

//___________________________________________________________________
void TDeviceFifoTest::StartReadout()
{
    // nothing to do to be able to run a FIFO test
}

//___________________________________________________________________
void TDeviceFifoTest::StopReadout()
{
    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        shared_ptr<TReadoutBoardDAQ> myDAQBoard = dynamic_pointer_cast<TReadoutBoardDAQ>(fDevice->GetBoard( iboard ));
        
        if ( myDAQBoard ) {
            myDAQBoard->PowerOff();
        }
    }
}

