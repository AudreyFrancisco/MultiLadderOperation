/**
 * \brief This executable runs the FIFO test for all enabled chips in the device.
 * 
 * \remark
 * It is recommended to disable Manchester encoding in the Config.cfg file (CMU settings).
 * See the class TDeviceFifoTest. See below for more details on the CMU settings
 * that are appropriate for FIFO test.
 *
 * \note
 * cmuconfig = 0x60 for FIFO test (OB?)
 * i.e.
 * const int fPreviousId = 0x10;
 * const bool fInitialToken = false;
 * const bool fDisableManchester = true;
 * const bool fEnableDdr = true;
 *
 * \warning
 * The current code can not correctly handle a number N > 1 of readout boards
 * (currently only one is written). See for e.g. TDeviceBuilder::SetDeviceParamValue().
 * In particular, nothing is written (yet) to link a given number of chips to n readout
 * boards. For MFT, this is enough since the implemented device types (the different
 * types of MFT ladders) only need one readout board to be entirely read.
 *
 */

#include <iostream>
#include <unistd.h>
#include <cstdint>
#include <memory>
#include "AlpideDictionary.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TSetup.h"
#include "TDevice.h"
#include "TDeviceFifoTest.h"

using namespace std;

// Example of usage :
// ./test_fifo -v 1 -c ConfigSingleChipMOSAIC_FIFOtest.cfg
//
// If you want to see the available options, do :
// ./test_fifo -h
//
int main(int argc, char** argv) {
    
    TSetup mySetup;
    mySetup.DecodeCommandParameters(argc, argv);
    mySetup.ReadConfigFile();
    
    shared_ptr<TDevice> theDevice = (mySetup.GetDevice()).lock();
    
    const int nBoards = theDevice->GetNBoards( false );
    if ( !nBoards ) {
        cout << "No board found, exit!" << endl;
        return 0;
    }
    if ( nBoards != 1 ) {
        cout << "More than one board found, exit!" << endl;
        return 0;
    }
    
    const int nWorkingChips = theDevice->GetNWorkingChips();
    if ( !nWorkingChips ) {
        cout << "No working chip found, exit!" << endl;
        return 0;
    }

    shared_ptr<TReadoutBoard> theBoard = theDevice->GetBoard( 0 );
    shared_ptr<TReadoutBoardDAQ> myDAQBoard = dynamic_pointer_cast<TReadoutBoardDAQ>(theBoard);
  
    TDeviceFifoTest theDeviceTestor( theDevice );
    theDeviceTestor.SetVerboseLevel( mySetup.GetVerboseLevel() );
    theDeviceTestor.DoConfigureCMU();
    theDeviceTestor.Go();
 
    if ( myDAQBoard ) {
        myDAQBoard->PowerOff();
    }
    return 1;
}
