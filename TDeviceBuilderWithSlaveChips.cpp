#include <iostream>
#include <stdlib.h>
#include "TDevice.h"
#include "TDeviceBuilderWithSlaveChips.h"
#include "TChipConfig.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderWithSlaveChips::TDeviceBuilderWithSlaveChips() : TDeviceBuilder()
{ }

//___________________________________________________________________
TDeviceBuilder::~TDeviceBuilder()
{ }

#pragma mark - other protected methods

//___________________________________________________________________
void TDeviceBuilderWithSlaveChips::EnableSlave( const int mychip )
{
    bool toggle = false;
    shared_ptr<TChipConfig> mychipConfig = fCurrentDevice->GetChipConfig( mychip );
    int mychipID = mychipConfig->GetChipId();
    if ( mychipConfig->IsOBMaster() ) {
        for ( int i = mychipID + 1; i <= mychipID + 6; i++ ) {
            if ( fCurrentDevice->GetChipConfig(i)->IsEnabled() ) {
                toggle = true;
                break;
            }
        }
    }
    mychipConfig->SetEnableSlave( toggle );
}

// Make the daisy chain for OB readout, based on enabled chips
// i.e. to be called after CheckControlInterface
//___________________________________________________________________
void TDeviceBuilderWithSlaveChips::MakeDaisyChain()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }

    //   TConfig* config, std::vector <TAlpide *> * chips
    int firstLow[8], firstHigh[8], lastLow[8], lastHigh[8];
    
    for (int imod = 0; imod < 8; imod ++) {
        firstLow  [imod] = 0x77;
        firstHigh [imod] = 0x7f;
        lastLow   [imod] = 0x0;
        lastHigh  [imod] = 0x8;
    }
    
    // find the first and last enabled chip in each row
    for (int i = 0; i < fCurrentDevice->GetNChips(); i++) {
        if (!(fCurrentDevice->GetChipConfig(i))->IsEnabled()) continue;
        int chipId   = (fCurrentDevice->GetChipConfig(i))->GetChipId();
        int modId    = (chipId & 0x70) >> 4;
        
        if ( (chipId & 0x8) && (chipId < firstHigh [modId])) firstHigh [modId] = chipId;
        if (!(chipId & 0x8) && (chipId < firstLow  [modId])) firstLow  [modId] = chipId;
        
        if ( (chipId & 0x8) && (chipId > lastHigh [modId])) lastHigh [modId] = chipId;
        if (!(chipId & 0x8) && (chipId > lastLow  [modId])) lastLow  [modId] = chipId;
    }
    
    for (int i = 0; i < fCurrentDevice->GetNChips(); i++) {
        if (!(fCurrentDevice->GetChipConfig(i))->IsEnabled()) continue;
        int chipId   = (fCurrentDevice->GetChipConfig(i))->GetChipId();
        int modId    = (chipId & 0x70) >> 4;
        int previous = -1;
        
        // first chip in row gets token and previous chip is last chip in row (for each module)
        // (first and last can be same chip)
        if (chipId == firstLow [modId]) {
            (fCurrentDevice->GetChipConfig(i))->SetInitialToken(true);
            (fCurrentDevice->GetChipConfig(i))->SetPreviousId(lastLow [modId]);
        }
        else if (chipId == firstHigh [modId]) {
            (fCurrentDevice->GetChipConfig(i))->SetInitialToken(true);
            (fCurrentDevice->GetChipConfig(i))->SetPreviousId(lastHigh [modId]);
        }
        // chip is enabled, but not first in row; no token, search previous chip
        // search range: first chip in row on same module .. chip -1
        else if (chipId & 0x8) {
            (fCurrentDevice->GetChipConfig(i))->SetInitialToken(false);
            for (int iprev = chipId - 1; (iprev >= firstHigh [modId]) && (previous == -1); iprev--) {
                if ((fCurrentDevice->GetChipConfigById(iprev))->IsEnabled()) {
                    previous = iprev;
                }
            }
            (fCurrentDevice->GetChipConfig(i))->SetPreviousId(previous);
        }
        else if (!(chipId & 0x8)) {
            (fCurrentDevice->GetChipConfig(i))->SetInitialToken(false);
            for (int iprev = chipId - 1; (iprev >= firstLow [modId]) && (previous == -1); iprev--) {
                if ((fCurrentDevice->GetChipConfigById(iprev))->IsEnabled()) {
                    previous = iprev;
                }
            }
            (fCurrentDevice->GetChipConfig(i))->SetPreviousId(previous);
        }
        
        cout << "TDeviceBuilderWithSlaveChips::MakeDaisyChain()  - Chip Id " << chipId
        << ", token = " << (bool)(fCurrentDevice->GetChipConfig(i))->GetInitialToken()
        << ", previous = " << (fCurrentDevice->GetChipConfig(i))->GetPreviousId() << endl;
    }
}
