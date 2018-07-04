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

// Example of usage : for a verbosity level 3
// ./test_multi_digitalscan 3
//

int main(int argc, char** argv) {

    TMultiDeviceOperator myOperator;
    myOperator.SetScanType( MultiDeviceScanType::kNOISE_OCC_SCAN );
    if ( argc >= 2 ) {
        const int level = atoi(argv[1]);
        myOperator.SetVerboseLevel( level );
    } else {
        myOperator.SetVerboseLevel( 2 );
    }

    char suffix[20], fName[100];
    
    time_t       t = time(0);   // get time now
    struct tm *now = localtime( & t );
    sprintf(suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
    sprintf(fName, "multinoiseScan_%s.dat", suffix);
    myOperator.SetPrefixFilename( fName );
    try {
        myOperator.AddSetup( "../config/multiboard/ConfigIBtelescope_NoiseOccScan.cfg"); // internal trigger config is given by the Master MOSAIC
        myOperator.AddSetup( "../config/multiboard/ConfigMFTladder_NoiseOccScan_ladder1_BB3.cfg"); // Slave
        myOperator.AddSetup( "../config/multiboard/ConfigMFTladder_NoiseOccScan_ladder2_BB3.cfg"); // Slave
        myOperator.AddSetup( "../config/multiboard/ConfigMFTladder_NoiseOccScan_ladder3_BB3.cfg"); // Slave

        myOperator.CloseAdmission();
        myOperator.Go();
        myOperator.Terminate();
    } catch ( exception& msg ) {
            cerr << msg.what() << endl;
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
