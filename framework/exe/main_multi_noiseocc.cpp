/**
 * \brief This executable runs the digital scan test for all enabled chips in all devices.
 * 
 * \warning
 * Be sure to have only one Master MOSAIC board in your multi-device test setup
 *
 */

#include <iostream>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include "TReadoutBoard.h"
#include "TSetup.h"
#include "TDevice.h"
#include "TScanConfig.h"
#include "TMultiDeviceOperator.h"
#include "TBoardConfig.h"

using namespace std;

// Example of usage :
// ./test_multi_digitalscan 
//

int main() {
//int main(int argc, char** argv) {

    TMultiDeviceOperator myOperator;
    myOperator.SetScanType( MultiDeviceScanType::kNOISE_OCC_SCAN );

    char suffix[20], fName[100];
    
    time_t       t = time(0);   // get time now
    struct tm *now = localtime( & t );
    sprintf(suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
    sprintf(fName, "multinoiseScan_%s.dat", suffix);
    myOperator.SetPrefixFilename( fName );
    try {
        myOperator.AddSetup( "../config/multiboard/ConfigIBtelescope_NoiseOccScan_ext.cfg");
        myOperator.AddSetup( "../config/multiboard/ConfigMFTladder_NoiseOccScan_ext_ladder1.cfg");
        myOperator.AddSetup( "../config/multiboard/ConfigMFTladder_NoiseOccScan_ext_ladder2.cfg");
        myOperator.AddSetup( "../config/multiboard/ConfigMFTladder_NoiseOccScan_ext_ladder3.cfg");
        myOperator.CloseAdmission();
        myOperator.Go();
        myOperator.Terminate();
    } catch ( exception& msg ) {
            cerr << msg.what() << endl;
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
