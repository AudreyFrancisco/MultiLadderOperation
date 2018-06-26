/**
 * \brief This executable runs the digital scan test for all enabled chips in the device.
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
#include <string>
#include "TReadoutBoard.h"
#include "TSetup.h"
#include "TDevice.h"
#include "TScanConfig.h"
#include "TDeviceDigitalScan.h"
#include "TBoardConfig.h"

using namespace std;

// Example of usage : if 25 is the id of the tested ladder
// ./test_digitalscan -v 1 -c ../config/ConfigMFTladder_DigitalScan.cfg -l 25
//
// If you want to see the available options, do :
// ./test_digitalscan -h
//

int main(int argc, char** argv) {
    
    TSetup mySetup;
    mySetup.DecodeCommandParameters(argc, argv);
    mySetup.ReadConfigFile();
    
    shared_ptr<TDevice> theDevice = mySetup.GetDevice();
    
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
    
    shared_ptr<TScanConfig> theScanConfig = mySetup.GetScanConfig();

    if ( !theScanConfig ) {
        cout << "No scan config found, exit!" << endl;
        return EXIT_FAILURE;
    }
    
    TDeviceDigitalScan theDeviceTestor( theDevice, theScanConfig );
    theDeviceTestor.Init();
    theDeviceTestor.SetVerboseLevel( mySetup.GetVerboseLevel() );
    theDeviceTestor.SetRescueBadChipId( true );
    
    sleep(1);
    char chipName[20], suffix[20], fName[100];
    
    time_t       t = time(0);   // get time now
    struct tm *now = localtime( & t );
    sprintf(suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

    theDeviceTestor.Go(); // run the digital scan
    theDeviceTestor.Terminate();
    
    const bool Recreate = true;

    if ( !(theDevice->GetNickName()).empty() ) {
        sprintf( chipName, "%s", (theDevice->GetNickName()).c_str() );
        sprintf(fName, "digitalScan_%s_%s.dat", chipName, suffix);
    } else {
        sprintf(fName, "digitalScan_%s.dat", suffix);
    }
    theDeviceTestor.WriteDataToFile( fName, Recreate );
    theDeviceTestor.WriteCorruptedHitsToFile( fName, Recreate );
    theDeviceTestor.DrawAndSaveToFile( fName );
    
    return EXIT_SUCCESS;
}
