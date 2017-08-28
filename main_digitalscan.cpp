/**
 * \brief This executable runs the digital scan test for all enabled chips in the device.
 *
 * \remark
 *
 * \note
 *
 * \warning
 * The current code can not correctly handle a number N > 1 of readout boards
 * (currently only one is written). See for e.g. TDeviceBuilder::SetDeviceParamValue().
 * For MFT, this is enough since the implemented device types (the different
 * types of MFT ladders) only need one readout board to be entirely read.
 *
 */

#include <iostream>
#include "AlpideDictionary.h"
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
    shared_ptr<TScanConfig> theScanConfig = (mySetup.GetScanConfig()).lock();
    
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
    
    TDeviceDigitalScan theDeviceTestor( theDevice, theScanConfig );
    theDeviceTestor.SetVerboseLevel( mySetup.GetVerboseLevel() );
    theDeviceTestor.Init();
    theDeviceTestor.DoBaseConfig();
    if ( mySetup.GetVerboseLevel() ) {
        theDeviceTestor.DoDumpConfig();
    }
    
    sleep(1);
    char Suffix[20], fName[100];
    
    time_t       t = time(0);   // get time now
    struct tm *now = localtime( & t );
    sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

    if ( theDevice->GetBoardConfig(0)->GetBoardType() == TBoardType::kBOARD_DAQ ) { // DAQ board
        
        // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
        // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
        const bool enablePulse = true;
        const bool enableTrigger = false;
        const int triggerDelay = 0;
        const int pulseDelay = 2 * theDevice->GetBoardConfig(0)->GetParamValue( "STROBEDELAYBOARD" );
        theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
        theBoard->SetTriggerSource( TTriggerSource::kTRIG_EXT );
        
    } else { // MOSAIC board

        const bool enablePulse = true;
        const bool enableTrigger = true;
        const int triggerDelay = theDevice->GetBoardConfig(0)->GetParamValue("STROBEDELAYBOARD");
        const int pulseDelay = theDevice->GetBoardConfig(0)->GetParamValue("PULSEDELAY");
        theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
        theBoard->SetTriggerSource( TTriggerSource::kTRIG_INT );
    }

    // run the digital scan
    theDeviceTestor.Go();
    
    sprintf(fName, "Data/digitalScan_%s.dat", Suffix);
    const bool Recreate = true;
    theDeviceTestor.WriteDataToFile( fName, Recreate );

    return 0;
}
