#include "TAlpideDecoder.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TChipConfig.h"
#include "TDevice.h"
#include "TDeviceMaskScan.h"
#include "TErrorCounter.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TScanConfig.h"
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <string.h>

using namespace std;

//___________________________________________________________________
TDeviceMaskScan::TDeviceMaskScan() :
TDeviceHitScan(),
fNMaskStages( 0 ),
fNPixPerRegion( 0 )
{ }

//___________________________________________________________________
TDeviceMaskScan::TDeviceMaskScan( shared_ptr<TDevice> aDevice,
                                  shared_ptr<TScanConfig> aScanConfig ) :
TDeviceHitScan( aDevice, aScanConfig ), 
fNMaskStages( 0 ),
fNPixPerRegion( 0 )
{ }

//___________________________________________________________________
TDeviceMaskScan::~TDeviceMaskScan()
{ }

//___________________________________________________________________
void TDeviceMaskScan::InitScanParameters()
{
    TDeviceHitScan::InitScanParameters();
    fNMaskStages = fScanConfig->GetNMaskStages();
    fNPixPerRegion = fScanConfig->GetPixPerRegion();
}

//___________________________________________________________________
void TDeviceMaskScan::ConfigureBoards()
{
    shared_ptr<TReadoutBoard> theBoard;
    
    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        theBoard = fDevice->GetBoard( iboard );
        if ( !theBoard ) {
            throw runtime_error( "TDeviceMaskScan::ConfigureBoard() - no readout board found." );
        }
        
        if ( fDevice->GetBoardConfig(iboard)->GetBoardType() == TBoardType::kBOARD_DAQ ) { // DAQ board
            
            // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
            // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
            const bool enablePulse = true;
            const bool enableTrigger = false;
            const int triggerDelay = 0;
            const int pulseDelay = 2 * fDevice->GetBoardConfig(iboard)->GetParamValue( "STROBEDELAYBOARD" );
            theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
            theBoard->SetTriggerSource( TTriggerSource::kTRIG_EXT );
            
        } else { // MOSAIC board
            
            const bool enablePulse = true;
            const bool enableTrigger = true;
            const int triggerDelay = fDevice->GetBoardConfig(iboard)->GetParamValue("STROBEDELAYBOARD");
            const int pulseDelay = fDevice->GetBoardConfig(iboard)->GetParamValue("PULSEDELAY");
            theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
            theBoard->SetTriggerSource( TTriggerSource::kTRIG_INT );
        }
    }
}
