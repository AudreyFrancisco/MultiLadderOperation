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
// ./test_fifo -v 1 -c ConfigSingleChipMOSAIC.cfg
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
