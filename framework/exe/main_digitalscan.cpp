/**
 * \brief This executable runs the digital scan test for all enabled chips in the device.
 *
 * \remark
 * Missing: list of dead pixels.
 *
 * \note
 * The default configuration file for this test for a MFT ladder is 
 * ConfigMFTladder_DigitalScan.cfg
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
#include "TReadoutBoard.h"
#include "TSetup.h"
#include "TDevice.h"
#include "TScanConfig.h"
#include "TDeviceDigitalScan.h"
#include "TBoardConfig.h"

using namespace std;

// Example of usage :
// ./test_digital -v 1 -c ConfigMFTladder_DigitalScan.cfg
//
// If you want to see the available options, do :
// ./test_digital -h
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
    
    shared_ptr<TScanConfig> theScanConfig = (mySetup.GetScanConfig()).lock();

    if ( !theScanConfig ) {
        cout << "No scan config found, exit!" << endl;
        return EXIT_FAILURE;
    }
    
    TDeviceDigitalScan theDeviceTestor( theDevice, theScanConfig );
    theDeviceTestor.SetVerboseLevel( mySetup.GetVerboseLevel() );
    theDeviceTestor.Init();
    
    sleep(1);
    char Suffix[20], fName[100];
    
    time_t       t = time(0);   // get time now
    struct tm *now = localtime( & t );
    sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

    theDeviceTestor.Go(); // run the digital scan
    theDeviceTestor.Terminate();
    
    sprintf(fName, "digitalScan_%s.dat", Suffix);
    const bool Recreate = true;
    theDeviceTestor.WriteDataToFile( fName, Recreate );

    return EXIT_SUCCESS;
}
