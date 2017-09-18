/**
 * \brief This executable runs the FIFO test for all enabled chips in the device.
 * 
 * \remark
 * It is recommended to disable Manchester encoding in the config file (CMU settings).
 * However, this was seen to lead to ControlInterface sync error with MOSAIC board.
 * Maybe this setting applies only in the case of the DAQ board or OB master chip?
 * See the class TDeviceFifoTest. See below for more details on the CMU settings
 * that were recommended for FIFO test (disabling Manchester encoding).
 *
 * \note
 * The default configuration file for this test for a MFT ladder is
 * ConfigMFTladder_FIFOtest.cfg
 *
 * \warning
 * The current code can not correctly handle a number N > 1 of readout boards
 * (currently only one is written). See for e.g. TDeviceBuilder::SetDeviceParamValue().
 * For MFT, this is enough since the implemented device types (the different
 * types of MFT ladders) only need one readout board to be entirely read.
 *
 */

#include <iostream>
#include <cstdlib>
#include "TSetup.h"
#include "TDevice.h"
#include "TDeviceFifoTest.h"

using namespace std;

// Example of usage :
// ./test_fifo -v 1 -c ConfigMFTladder_FIFOtest.cfg
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
        return EXIT_FAILURE;
    }
    if ( nBoards != 1 ) {
        cout << "More than one board found, exit!" << endl;
        return EXIT_FAILURE;
    }
    
    const int nWorkingChips = theDevice->GetNWorkingChips();
    if ( !nWorkingChips ) {
        cout << "No working chip found, exit!" << endl;
        return EXIT_FAILURE;
    }
  
    TDeviceFifoTest theDeviceTestor( theDevice );
    theDeviceTestor.SetVerboseLevel( mySetup.GetVerboseLevel() );
    theDeviceTestor.Init();
    theDeviceTestor.Go();
    theDeviceTestor.Terminate();
    
    return EXIT_SUCCESS;
}
