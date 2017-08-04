/* ---------------
 * Example of MOSAIC use
 *
 ----------------- */

#include <iostream>
#include <unistd.h>
#include <cstdint>
#include <memory>
#include "TReadoutBoard.h"
#include "TReadoutBoardMOSAIC.h"
#include "TAlpide.h"
#include "TSetup.h"
#include "TDevice.h"
#include "AlpideDictionary.h"
#include "TDeviceChipVisitor.h"

using namespace std;

// Example of usage :
// ./test_mosaic -v 1 -c ConfigSingleChipMOSAIC.cfg
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
    
    // global reset chips
    theBoard->SendOpCode( (std::uint16_t)AlpideOpCode::GRST );

    // pixel matrix reset
    theBoard->SendOpCode( (std::uint16_t)AlpideOpCode::PRST );
    
    // configure chip(s)
    TDeviceChipVisitor theDeviceChipVisitor( theDevice );
    theDeviceChipVisitor.SetVerboseLevel( mySetup.GetVerboseLevel() );
    theDeviceChipVisitor.DoBaseConfig();
    
    // readout reset
    theBoard->SendOpCode( (std::uint16_t)AlpideOpCode::RORST );
    
	//--- Data Tacking
	int numberOfReadByte; // the bytes of row event
	unsigned char *theBuffer; // the buffer containing the event

	int enablePulse = -1, enableTrigger = -1, triggerDelay = -1, pulseDelay = -1, nTriggers = -1; // variables that define the trigger/pulse

	theBuffer = (unsigned char*) malloc(200 * 1024); // allocates 200 kilobytes ...

	bool isDataTackingEnd = false; // break the execution of read polling
	int returnCode = 0;
	int timeoutLimit = 10; // ten seconds

	// sets the trigger
	theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
	theBoard->SetTriggerSource( TTriggerSource::kTRIG_INT );

	(dynamic_pointer_cast<TReadoutBoardMOSAIC>(theBoard))->StartRun(); // Activate the data taking ...

	theBoard->Trigger( nTriggers ); // Preset end start the trigger

	while( !isDataTackingEnd ) { // while we don't receive a timeout
		returnCode = theBoard->ReadEventData( numberOfReadByte, theBuffer );
		if( returnCode != 0 ) { // we have some thing
			std::cout << "Read an event !  Dimension :" << numberOfReadByte << std::endl;   // Consume the buffer ...
			usleep(20000); // wait
		} else { // read nothing is finished ?
			if(timeoutLimit-- == 0) isDataTackingEnd = true;
			sleep(1);
		}
	}

	(dynamic_pointer_cast<TReadoutBoardMOSAIC>(theBoard))->StopRun(); // Stop run
    return 1;
}
