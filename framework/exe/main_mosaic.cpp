/**
 * \brief This executable runs a communication test {device <--> mosaic board}.
 *
 * \remark
 * The TAlpideDecoder does not seem to work properly for the moment (to be fixed in
 * subsequent versions). Only empty frames are detected, which is not the case with
 * the low level software from Giuseppe.
 *
 * \note
 * The default configuration file for this test for a MFT ladder is
 * ConfigMFTladderMOSAIC.cfg
 *
 * \warning
 * The current code can not correctly handle a number N > 1 of readout boards
 * (currently only one is written). See for e.g. TDeviceBuilder::SetDeviceParamValue().
 * For MFT, this is enough since the implemented device types (the different
 * types of MFT ladders) only need one readout board to be entirely read.
 *
 */

#include <iostream>
#include <unistd.h>
#include <cstdint>
#include <memory>
#include "TReadoutBoard.h"
#include "TReadoutBoardMOSAIC.h"
#include "TAlpide.h"
#include "TSetup.h"
#include "TDevice.h"
#include "TScanConfig.h"
#include "AlpideDictionary.h"
#include "TDeviceChipVisitor.h"
#include "TVerbosity.h"

using namespace std;

// Example of usage :
// ./test_mosaic -v 1 -c ConfigMFTladderMOSAIC.cfg
//

int main(int argc, char** argv) {
    
    TSetup mySetup;
    mySetup.DecodeCommandParameters(argc, argv);
    mySetup.ReadConfigFile();
    
    shared_ptr<TDevice> theDevice = (mySetup.GetDevice()).lock();

    const int nBoards = theDevice->GetNBoards( false );
    if ( !nBoards ) {
        cout << "No board found, exit!" << endl;
        return 1;
    }
    if ( nBoards != 1 ) {
        cout << "More than one board found, exit!" << endl;
        return 1;
    }
    const int nWorkingChips = theDevice->GetNWorkingChips();
    if ( !nWorkingChips ) {
        cout << "No working chip found, exit!" << endl;
        return 1;
    }

    shared_ptr<TScanConfig> theScanConfig = (mySetup.GetScanConfig()).lock();

    if ( !theScanConfig ) {
        cout << "No scan config found, exit!" << endl;
        return 1;
    }
    
    shared_ptr<TReadoutBoardMOSAIC> theBoard = dynamic_pointer_cast<TReadoutBoardMOSAIC>(theDevice->GetBoard( 0 ));
    if ( !theBoard ) {
        cout << "No MOSAIC board found, exit!" << endl;
        return 1;
    }
    theBoard->SetVerboseLevel( mySetup.GetVerboseLevel() );
    
    // configure chip(s)
    TDeviceChipVisitor theDeviceChipVisitor( theDevice );
    theDeviceChipVisitor.SetVerboseLevel( mySetup.GetVerboseLevel() );
    theDeviceChipVisitor.Init();
    theDeviceChipVisitor.DoActivateConfigMode();
    theDeviceChipVisitor.DoBaseConfig();
    theDeviceChipVisitor.DoActivateReadoutMode();
    theDeviceChipVisitor.DoConfigureMaskStage( theScanConfig->GetPixPerRegion(), theScanConfig->GetNMaskStages() );
    
	//--- Data Tacking
	int numberOfReadByte; // the bytes of row event
	unsigned char *theBuffer; // the buffer containing the event

    // variables that define the trigger/pulse
    bool enablePulse = true, enableTrigger = true;
    const int triggerDelay = theDevice->GetBoardConfig(0)->GetParamValue("STROBEDELAYBOARD"); // original value 160
    const int pulseDelay = theDevice->GetBoardConfig(0)->GetParamValue("PULSEDELAY"); // original value 1000
    const int nTriggers = theScanConfig->GetNInj(); // original value -1
    
	theBuffer = (unsigned char*) malloc(200 * 1024); // allocates 200 kilobytes ...

	bool isDataTakingEnd = false; // break the execution of read polling
	int returnCode = 0;
	int timeoutLimit = 10; // ten seconds

	// sets the trigger
	theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
	theBoard->SetTriggerSource( TTriggerSource::kTRIG_INT );

	theBoard->StartRun(); // Activate the data taking ...
    if ( mySetup.GetVerboseLevel() ) {
        theDeviceChipVisitor.DoDumpConfig();
    }

    theBoard->Trigger( nTriggers ); // Preset and start the trigger


    int nEvents = 0;
    int MAX_N_EVENTS = nTriggers * theDevice->GetNWorkingChips();
	while( !isDataTakingEnd ) { // while we don't receive a timeout or we don't have enough events yet
        if ( nEvents == MAX_N_EVENTS-1 ) {
            isDataTakingEnd = true;
        }
		returnCode = theBoard->ReadEventData( numberOfReadByte, theBuffer );
		if( returnCode != 0 ) { // we have some thing
            cout << "Received Event " << std::dec << nEvents << " with length " << numberOfReadByte << endl;
            if ( mySetup.GetVerboseLevel() > TVerbosity::kVERBOSE ) {
                for ( int iByte = 0; iByte < numberOfReadByte; ++iByte ) {
                    printf ("%02x ", (int) theBuffer[iByte]);
                }
                cout << endl;
            }
            if( numberOfReadByte == 0 ) isDataTakingEnd = true;
            nEvents++;
			usleep(200); // wait
		} else { // read nothing is finished ?
			if(timeoutLimit-- == 0) isDataTakingEnd = true;
			sleep(1);
		}
	}

	theBoard->StopRun(); // Stop run

    return 0;
}
