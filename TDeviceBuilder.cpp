#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include "TDevice.h"
#include "TDeviceBuilder.h"
#include "TChipConfig.h"
#include "TBoardConfig.h"
#include "TBoardConfigMOSAIC.h"
#include "TAlpide.h"


using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilder::TDeviceBuilder() : TVerbosity(),
    fCurrentDevice( nullptr )
{
}

//___________________________________________________________________
TDeviceBuilder::~TDeviceBuilder()
{ }

#pragma mark - Propagate verbosity down to the TDevice

//___________________________________________________________________
void TDeviceBuilder::SetVerboseLevel( const int level )
{
    fVerboseLevel = level;
    if ( fCurrentDevice ) {
        fCurrentDevice->SetVerboseLevel( level );
    }
}

#pragma mark - Device creation and initialisation

//___________________________________________________________________
void TDeviceBuilder::CreateDevice()
{
    if ( !fCurrentDevice ) {
        fCurrentDevice = make_shared<TDevice>();
    }
}

//___________________________________________________________________
void TDeviceBuilder::SetDeviceType( const TDeviceType dt )
{
    if ( !fCurrentDevice ) {
        throw runtime_error( "TDeviceBuilder::SetDeviceType() - no device defined!" );
    }
    if ( fCurrentDevice->IsConfigFrozen() || fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    fCurrentDevice->SetDeviceType( dt );
}

//___________________________________________________________________
void TDeviceBuilder::SetDeviceParamValue( const char *Name, const char *Value, int Chip )
{
    if ( !fCurrentDevice ) {
        throw runtime_error( "TDeviceBuilder::SetDeviceParamValue() - no device defined!" );
    }

    int Start, ChipStop, BoardStop;
    if (Chip == -1) {
        Start     = 0;
        ChipStop  = fCurrentDevice->GetChipConfigsVectorSize();
        BoardStop = fCurrentDevice->GetNBoards();
    }
    else {
        Start     = Chip;
        ChipStop  = Chip+1;
        BoardStop = Chip+1;
    }
    
    // Todo: correctly handle the number of readout boards
    // currently only one is written
    // Note: having a config file with parameters for the mosaic board, but a setup with a DAQ board
    // (or vice versa) will issue unknown-parameter warnings...
    if ( (fCurrentDevice->GetChipConfig(0))->IsParameter(Name) ) {
        for (int i = Start; i < ChipStop; i++) {
            (fCurrentDevice->GetChipConfig(i))->SetParamValue( Name, Value );
        }
    }
    else if ( (fCurrentDevice->GetBoardConfig(0))->IsParameter(Name) ) {
        for (int i = Start; i < BoardStop; i++) {
            (fCurrentDevice->GetBoardConfig(i))->SetParamValue( Name, Value );
        }
    }
    else if ( (!strcmp(Name, "ADDRESS"))
             && ((fCurrentDevice->GetBoardConfig(0))->GetBoardType() == TBoardType::kBOARD_MOSAIC) ) {
        for (int i = Start; i < BoardStop; i++) {
            shared_ptr<TBoardConfigMOSAIC> boardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC>(fCurrentDevice->GetBoardConfig(i));
            boardConfig->SetIPaddress( Value );
        }
    } else {
        cout << "TDeviceBuilder::SetDeviceParamValue() - Warning: Unknown parameter "
             << Name << endl;
    }
}

#pragma mark - protected methods

// Try to communicate with all chips, disable chips that are not answering
//___________________________________________________________________
void TDeviceBuilder::CheckControlInterface()
{
    uint16_t WriteValue = 10;
    uint16_t Value;
    if ( fVerboseLevel ) {
        cout << endl
        << "TDeviceBuilder::CheckControlInterface() - Before starting actual test:"
        << endl
        << "TDeviceBuilder::CheckControlInterface() - Checking the control interfaces of all chips by doing a single register readback test"
        << endl;
    }
    
    if ( !fCurrentDevice->IsConfigFrozen() ) {
        throw runtime_error( "TDeviceBuilder::CheckControlInterface() - no config defined!" );
    }
    if ( fCurrentDevice->IsSetupFrozen() ) {
        cerr << "TDeviceBuilder::CheckControlInterface() - setup already initialised!" << endl;
        return;
    }

    if ( !fCurrentDevice->GetNChips() ) {
        throw runtime_error( "TDeviceBuilder::CheckControlInterface() - no chip defined!" );
    }

    for ( int i = 0; i < fCurrentDevice->GetNChips(); i++ ) {
        
        if ( !(fCurrentDevice->GetChipConfig(i)->IsEnabled()) ) continue;
        
        fCurrentDevice->GetChip(i)->WriteRegister( 0x60d, WriteValue );
        try {
            fCurrentDevice->GetChip(i)->ReadRegister( 0x60d, Value );
            if ( WriteValue == Value ) {
                if ( fVerboseLevel ) {
                    cout << "TDeviceBuilder::CheckControlInterface() -  Chip ID "
                         << fCurrentDevice->GetChipConfig(i)->GetChipId()
                         << ", readback correct." << endl;
                }
                fCurrentDevice->IncrementWorkingChipCounter();
            } else {
                cerr << "TDeviceBuilder::CheckControlInterface() - Chip ID "
                     << fCurrentDevice->GetChipConfig(i)->GetChipId()
                     << ", wrong readback value (" << Value << " instead of " << WriteValue << "), disabling." << endl;
                fCurrentDevice->GetChipConfig(i)->SetEnable(false);
            }
        } catch (std::exception &e) {
            cerr << "TDeviceBuilder::CheckControlInterface() - Chip ID "
                 << fCurrentDevice->GetChipConfig(i)->GetChipId() << ", not answering, disabling." << endl;
            fCurrentDevice->GetChipConfig(i)->SetEnable(false);
        }
    }
    cout << "TDeviceBuilder::CheckControlInterface() - Found "
         << fCurrentDevice->GetNWorkingChips() << " working chips." << endl << endl;
    
    if ( fCurrentDevice->GetNWorkingChips() == 0 ) {
        throw runtime_error( "TDeviceBuilder::CheckControlInterface() - no working chip found!" );
    }
}

