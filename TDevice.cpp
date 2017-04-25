#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include "TAlpide.h"
#include "TBoardConfig.h"
#include "TBoardConfigDAQ.h"
#include "TBoardConfigMOSAIC.h"
#include "TChipConfig.h"
#include "TDevice.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDevice::TDevice() : TVerbosity(),
    fCreatedConfig( false ),
    fInitialisedSetup( false ),
    fNWorkingChips( 0 ),
    fNChips( 0 ),
    fNModules( 0 ),
    fStartChipId( 0 ),
    fBoardType( TBoardType::kBOARD_UNKNOWN ),
    fDeviceType( TDeviceType::kUNKNOWN )
{ }

//___________________________________________________________________
TDevice::~TDevice()
{
    fBoards.clear();
    fChips.clear();
    fBoardConfigs.clear();
    fChipConfigs.clear();
}

#pragma mark - setters

//___________________________________________________________________
void TDevice::SetNChips( const int number )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::SetNChips() - not allowed: config already created !" << endl;
        return;
    }
    if ( number <= 0 ) {
        throw out_of_range( "TDevice::SetNChips() - number <= 0 not allowed !" );
    }
    fNChips = number;
}

//___________________________________________________________________
void TDevice::SetNModules( const int number )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::SetNModules() - not allowed: config already created !" << endl;
        return;
    }
    if ( number <= 0 ) {
        throw out_of_range( "TDevice::SetNModules() - number <= 0 not allowed !" );
    }
    fNModules = number;
}

//___________________________________________________________________
void TDevice::SetStartChipId( const int Id )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::SetStartChipId() - not allowed: config already created !" << endl;
        return;
    }
    if ( Id < 0 ) {
        throw out_of_range( "TDevice::SetStartChipId() - Id < 0 not allowed !" );
    }
    fStartChipId = Id;
}

//___________________________________________________________________
void TDevice::SetBoardType( const TBoardType bt )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::SetBoardType() - not allowed: config already created !" << endl;
        return;
    }
    fBoardType = bt;
}

//___________________________________________________________________
void TDevice::SetDeviceType( const TDeviceType dt )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::SetDeviceType() - not allowed: config already created !" << endl;
        return;
    }
     fDeviceType = dt;
}

#pragma mark - add an item to one of the vectors

//___________________________________________________________________
void TDevice::AddBoard( std::shared_ptr<TReadoutBoard> newBoard )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::AddBoard() - not allowed: config already created !" << endl;
        return;
    }
    if ( !newBoard ) {
        throw runtime_error( "TDevice::AddBoard() - a null pointer can not be added to the device!" );
    }
    fBoards.push_back( move(newBoard) );
}

//___________________________________________________________________
void TDevice::AddBoardConfig( std::shared_ptr<TBoardConfig> newBoardConfig )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::AddBoardConfig() - not allowed: config already created !" << endl;
        return;
    }
    if ( !newBoardConfig ) {
        throw runtime_error( "TDevice::AddBoardConfig() - a null pointer can not be added to the device!" );
    }
    fBoardConfigs.push_back( move(newBoardConfig) );
}

//___________________________________________________________________
void TDevice::AddChip( std::shared_ptr<TAlpide> newChip )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::AddChip() - not allowed: config already created !" << endl;
        return;
    }
    if ( !newChip ) {
        throw runtime_error( "TDevice::AddChip() - a null pointer can not be added to the device!" );
    }
    fChips.push_back( move(newChip) );
}

//___________________________________________________________________
void TDevice::AddChipConfig( std::shared_ptr<TChipConfig> newChipConfig )
{
    if ( fCreatedConfig ) {
        cerr << "TDevice::AddChipConfig() - not allowed: config already created !" << endl;
        return;
    }
    if ( !newChipConfig ) {
        throw runtime_error( "TDevice::AddChipConfig() - a null pointer can not be added to the device!" );
    }
    fChipConfigs.push_back( move(newChipConfig) );
}

//___________________________________________________________________
void TDevice::IncrementWorkingChipCounter()
{
    if ( fInitialisedSetup ) {
        return;
    }
    fNWorkingChips++;
}

#pragma mark - getters

//___________________________________________________________________
shared_ptr<TReadoutBoard> TDevice::GetBoard( const int iBoard)
{
    if ( (!fInitialisedSetup) || fBoards.empty() ) {
        throw runtime_error( "TDevice::GetBoard() - no board defined!" );
    }
    if ( (iBoard < 0) || (iBoard >= (int)fBoards.size()) ) {
        cerr << "TDevice::GetBoard() - iBoard = " << iBoard << endl;
        throw out_of_range( "TDevice::GetBoard() - wrong board index!" );
    }
    shared_ptr<TReadoutBoard> myBoard = nullptr;
    try {
        myBoard = fBoards.at( iBoard );
    } catch ( ... ) {
        cerr << "TDevice::GetBoard() - error, exit!" << endl;
        exit(0);
    }
    return myBoard;
}

//___________________________________________________________________
shared_ptr<TBoardConfig> TDevice::GetBoardConfig( const int iBoard)
{
    if ( (!fCreatedConfig) || fBoardConfigs.empty() ) {
        throw runtime_error( "TDevice::GetBoardConfig() - no config defined!" );
    }
    if ( (iBoard < 0) || (iBoard >= (int)fBoardConfigs.size()) ) {
        cerr << "TDevice::GetBoardConfig() - iBoard = " << iBoard << endl;
        throw out_of_range( "TDevice::GetBoardConfig() - wrong board config index!" );
    }
    shared_ptr<TBoardConfig> myBoardConfig = nullptr;
    try {
        myBoardConfig = fBoardConfigs.at( iBoard );
    } catch ( ... ) {
        cerr << "TDevice::GetBoardConfig() - error, exit!" << endl;
        exit(0);
    }
    return myBoardConfig;
}

//___________________________________________________________________
shared_ptr<TAlpide> TDevice::GetChip( const int iChip)
{
    if ( (!fInitialisedSetup) || fChips.empty() ) {
        throw runtime_error( "TDevice::GetChip() - no chip defined!" );
    }
    if ( (iChip < 0) || (iChip >= (int)fChips.size()) ) {
        cerr << "TDevice::GetChip() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetChip() - wrong chip index!" );
    }
    shared_ptr<TAlpide> myChip = nullptr;
    try {
        myChip = fChips.at( iChip );
    } catch ( ... ) {
        cerr << "TDevice::GetChip() - error, exit!" << endl;
        exit(0);
    }
    return myChip;
}

//___________________________________________________________________
shared_ptr<TChipConfig> TDevice::GetChipConfig( const int iChip )
{
    if ( (!fCreatedConfig) || fChipConfigs.empty()  ) {
        throw runtime_error( "TDevice::GetChipConfig() - no config defined!" );
    }
    if ( (iChip < 0) || (iChip >= (int)fChipConfigs.size()) ) {
        cerr << "TDevice::GetChipConfig() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetChipConfig() - wrong chip config index!" );
    }
    shared_ptr<TChipConfig> myChipConfig = nullptr;
    try {
        myChipConfig = fChipConfigs.at( iChip );
    } catch ( ... ) {
        cerr << "TDevice::GetChipConfig() - error, exit!" << endl;
        exit(0);
    }
    return myChipConfig;
}

//___________________________________________________________________
shared_ptr<TChipConfig> TDevice::GetChipConfigById( const int chipId )
{
    if ( (!fCreatedConfig) || fChipConfigs.empty() ) {
        throw runtime_error( "TDevice::GetChipConfigById() - no config defined!" );
    }
    shared_ptr<TChipConfig> config = nullptr;
    for (int i = 0; i < (int)fChipConfigs.size(); i++) {
        if (fChipConfigs.at(i)->GetChipId() == chipId) {
            config = fChipConfigs.at(i);
            break;
        }
    }
    if ( !config ) {
        cerr << "TDevice::GetChipConfigById() - requested chip id = " << chipId << endl;
        throw runtime_error( "TDevice::GetChipConfigById() - requested chip id not found." );
    }
    return config;
}

//___________________________________________________________________
int TDevice::GetNChips() const
{
    if ( fCreatedConfig ) {
        return (int)fChipConfigs.size();
    } else {
        return fNChips;
    }
}

//___________________________________________________________________
int TDevice::GetChipConfigsVectorSize() const
{
    return (int)fChipConfigs.size();
}

//___________________________________________________________________
int TDevice::GetNBoards( const bool useBoardConfigVector ) const
{
    if ( useBoardConfigVector ) {
        return (int)fBoardConfigs.size();
    } else {
        return (int)fBoards.size();
    }
}

//___________________________________________________________________
int TDevice::GetStartChipId()
{
    if ( fCreatedConfig ) {
        return GetChipConfig(0)->GetChipId();
    } else {
        return fStartChipId;
    }
}

//___________________________________________________________________
bool TDevice::IsMFTLadder() const
{
    if ( !fCreatedConfig ) {
        throw runtime_error( "TDevice::IsMFTLadder() - device not created yet." );
    }
    if ( (GetDeviceType() == TDeviceType::kMFT_LADDER5)
        || (GetDeviceType() == TDeviceType::kMFT_LADDER4)
        || (GetDeviceType() == TDeviceType::kMFT_LADDER3)
        || (GetDeviceType() == TDeviceType::kMFT_LADDER2) ) {
        return true;
    } else {
        return false;
    }
}
