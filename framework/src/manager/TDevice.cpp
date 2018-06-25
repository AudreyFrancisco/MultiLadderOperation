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
    fNChips( 0 ),
    fNModules( 0 ),
    fStartChipId( 0 ),
    fBoardType( TBoardType::kBOARD_UNKNOWN ),
    fDeviceType( TDeviceType::kUNKNOWN ),
    fNickName( "" ),
    fLadderId( 0 )
{ }

//___________________________________________________________________
TDevice::~TDevice()
{
    fBoards.clear();
    fChips.clear();
    fBoardConfigs.clear();
    fChipConfigs.clear();
    fNWorkingChipsPerBoard.clear();
}

#pragma mark - setters

//___________________________________________________________________
void TDevice::SetNChips( const unsigned int number )
{
    if ( IsConfigFrozen() ) {
        cerr << "TDevice::SetNChips() - not allowed: config already created !" << endl;
        return;
    }
    fNChips = number;
}

//___________________________________________________________________
void TDevice::SetNModules( const unsigned int number )
{
    if ( IsConfigFrozen() ) {
        cerr << "TDevice::SetNModules() - not allowed: config already created !" << endl;
        return;
    }
    fNModules = number;
}

//___________________________________________________________________
void TDevice::SetStartChipId( const unsigned int Id )
{
    if ( IsConfigFrozen() ) {
        cerr << "TDevice::SetStartChipId() - not allowed: config already created !" << endl;
        return;
    }
    fStartChipId = Id;
}

//___________________________________________________________________
void TDevice::SetBoardType( const TBoardType bt )
{
    if ( IsConfigFrozen() ) {
        cerr << "TDevice::SetBoardType() - not allowed: config already created !" << endl;
        return;
    }
    fBoardType = bt;
}

//___________________________________________________________________
void TDevice::SetDeviceType( const TDeviceType dt )
{
    if ( IsConfigFrozen() ) {
        cerr << "TDevice::SetDeviceType() - not allowed: config already created !" << endl;
        return;
    }
     fDeviceType = dt;
}

//___________________________________________________________________
void TDevice::SetNickName( const std::string name )
{
    if ( IsSetupFrozen() ) {
        cerr << "TDevice::SetNickName() - not allowed: setup already done !" << endl;
        return;
    }
    fNickName = name;
}

//___________________________________________________________________
void TDevice::SetLadderId( const unsigned int number )
{
    if ( IsSetupFrozen() ) {
        cerr << "TDevice::SetLadderId() - not allowed: setup already done !" << endl;
        return;
    }
    fLadderId = number;
}

//___________________________________________________________________
void TDevice::EnableClockOutputs( const bool en )
{
    if ( !IsConfigFrozen() ) {
        cerr << "TDevice::EnableClockOutputs() - not allowed: config not created/frozen yet !" << endl;
        return;
    }
    if ( fBoards.size() == 0 ) {
        throw runtime_error( "TDevice::EnableClockOutputs() - no board available!" );
    }
    for ( int i = 0; i < (int)fBoards.size(); i++ ) {
        GetBoard(i)->EnableClockOutputs(en);
    }
}

#pragma mark - add an item to one of the vectors

//___________________________________________________________________
void TDevice::AddBoard( std::shared_ptr<TReadoutBoard> newBoard )
{
    if ( IsSetupFrozen() ) {
        cerr << "TDevice::AddBoard() - not allowed: setup already done !" << endl;
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
    if ( IsConfigFrozen() ) {
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
    if ( IsSetupFrozen() ) {
        cerr << "TDevice::AddChip() - not allowed: setup already done !" << endl;
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
    if ( IsConfigFrozen() ) {
        cerr << "TDevice::AddChipConfig() - not allowed: config already created !" << endl;
        return;
    }
    if ( !newChipConfig ) {
        throw runtime_error( "TDevice::AddChipConfig() - a null pointer can not be added to the device!" );
    }
    fChipConfigs.push_back( move(newChipConfig) );
}

//___________________________________________________________________
void TDevice::AddNWorkingChipCounterPerBoard( const unsigned int nChips )
{
    if ( !GetNWorkingChips() ) {
        return;
    }
    if ( fNWorkingChipsPerBoard.size() >= fBoards.size() ) {
        throw runtime_error( "TDevice::AddNWorkingChipCounterPerBoard() - no more board available!" );
    }
    fNWorkingChipsPerBoard.push_back( nChips );
}

//___________________________________________________________________
void TDevice::AddWorkingChipIndex( const common::TChipIndex idx )
{
    if ( IsSetupFrozen() ) {
        cerr << "TDevice::AddChipIndex() - no allowad, setup already frozen!" << endl;
        return;
    }
    fWorkingChipIndexList.push_back( idx );
}

#pragma mark - getters

//___________________________________________________________________
shared_ptr<TReadoutBoard> TDevice::GetBoard( const unsigned int iBoard )
{
    if ( fBoards.empty() ) {
        throw runtime_error( "TDevice::GetBoard() - no board defined!" );
    }
    if ( iBoard >= fBoards.size() ) {
        cerr << "TDevice::GetBoard() - iBoard = " << iBoard << endl;
        throw out_of_range( "TDevice::GetBoard() - wrong board index!" );
    }
    shared_ptr<TReadoutBoard> myBoard = fBoards.at( iBoard );
    if ( !myBoard ) {
        cerr << "TDevice::GetBoard() - iBoard = " << iBoard << endl;
        throw runtime_error( "TDevice::GetBoard() - board does not exist!" );
    }
    return myBoard;
}

//___________________________________________________________________
shared_ptr<TBoardConfig> TDevice::GetBoardConfig( const unsigned int iBoard )
{
    if ( fBoardConfigs.empty() ) {
        throw runtime_error( "TDevice::GetBoardConfig() - no config defined!" );
    }
    if ( iBoard >= fBoardConfigs.size() ) {
        cerr << "TDevice::GetBoardConfig() - iBoard = " << iBoard << endl;
        throw out_of_range( "TDevice::GetBoardConfig() - wrong board config index!" );
    }
    shared_ptr<TBoardConfig> myBoardConfig = fBoardConfigs.at( iBoard );
    if ( !myBoardConfig ) {
        cerr << "TDevice::GetBoardConfig() - iBoard = " << iBoard << endl;
        throw runtime_error( "TDevice::GetBoardConfig() - board config does not exist!" );
    }
    return myBoardConfig;
}

//___________________________________________________________________
shared_ptr<TReadoutBoard> TDevice::GetBoardByChip( const unsigned int iChip )
{
    if ( (!IsSetupFrozen()) || fBoards.empty() || fChips.empty() ) {
        throw runtime_error( "TDevice::GetBoardByChip() - no chip or board defined!" );
    }
    if ( iChip >= fChips.size() ) {
        cerr << "TDevice::GetBoardByChip() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetBoardByChip() - wrong chip index!" );
    }
    shared_ptr<TReadoutBoard> aBoard = (fChips.at(iChip)->GetReadoutBoard()).lock();
    shared_ptr<TReadoutBoard> myBoard = nullptr;
    for ( int i = 0; i < (int)fBoards.size(); i++ ) {
        myBoard = fBoards.at(i);
        if ( myBoard == aBoard ) {
            break;
        }
    }
    if ( !myBoard ) {
        cerr << "TDevice::GetBoardByChip() - requested chip = " << iChip << endl;
        throw runtime_error( "TDevice::GetBoardByChip() - board for requested chip not found." );
    }
    return myBoard;
}

//___________________________________________________________________
unsigned int TDevice::GetBoardIndexByChip( const unsigned int iChip )
{
    if ( fBoards.empty() || fChips.empty() ) {
        throw runtime_error( "TDevice::GetBoardIndexByChip() - no chip or board defined!" );
    }
    if ( iChip >= fChips.size() ) {
        cerr << "TDevice::GetBoardIndexByChip() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetBoardIndexByChip() - wrong chip index!" );
    }
    shared_ptr<TReadoutBoard> aBoard = (fChips.at(iChip)->GetReadoutBoard()).lock();
    shared_ptr<TReadoutBoard> myBoard = nullptr;
    int index = 0;
    for ( int i = 0; i < (int)fBoards.size(); i++ ) {
        myBoard = fBoards.at(i);
        if ( myBoard == aBoard ) {
            index = i;
            break;
        }
    }
    if ( !myBoard ) {
        cerr << "TDevice::GetBoardIndexByChip() - requested chip = " << iChip << endl;
        throw runtime_error( "TDevice::GetBoardIndexByChip() - board for requested chip not found." );
    }
    return index;
}

//___________________________________________________________________
shared_ptr<TBoardConfig> TDevice::GetBoardConfigByChip( const unsigned int iChip )
{
    if ( (!IsSetupFrozen()) || fBoardConfigs.empty() || fChips.empty() ) {
        throw runtime_error( "TDevice::GetBoardConfigByChip() - no chip or board config defined!" );
    }
    if ( iChip >= fChips.size() ) {
        cerr << "TDevice::GetBoardConfigByChip() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetBoardConfigByChip() - wrong chip index!" );
    }
    shared_ptr<TBoardConfig> aBoardConfig = nullptr;
    try {
        aBoardConfig = (GetBoardByChip( iChip )->GetConfig()).lock();
    } catch ( ... ) {
        throw runtime_error( "TDevice::GetBoardConfigByChip() - failed!" );
    }
    shared_ptr<TBoardConfig> myBoardConfig = nullptr;
    for ( int i = 0; i < (int)fBoardConfigs.size(); i++ ) {
        myBoardConfig = fBoardConfigs.at(i);
        if ( myBoardConfig == aBoardConfig ) {
            break;
        }
    }
    if ( !myBoardConfig ) {
        cerr << "TDevice::GetBoardConfigByChip() - requested chip = " << iChip << endl;
        throw runtime_error( "TDevice::GetBoardConfigByChip() - board for requested chip not found." );
    }
    return myBoardConfig;
}

//___________________________________________________________________
shared_ptr<TAlpide> TDevice::GetChip( const unsigned int iChip )
{
    if ( fChips.empty() ) {
        throw runtime_error( "TDevice::GetChip() - no chip defined!" );
    }
    if ( iChip >= fChips.size() ) {
        cerr << "TDevice::GetChip() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetChip() - wrong chip index!" );
    }
    shared_ptr<TAlpide> myChip = fChips.at( iChip );
    if ( !myChip ) {
        cerr << "TDevice::GetChip() - iChip = " << iChip << endl;
        throw runtime_error( "TDevice::GetChip() - chip does not exist!" );
    }
    return myChip;
}

//___________________________________________________________________
shared_ptr<TAlpide> TDevice::GetChipById( const unsigned int chipId )
{
    if ( (!IsSetupFrozen()) || fChipConfigs.empty() ||  fChips.empty() ) {
        throw runtime_error( "TDevice::GetChipById() - no chip or chip config defined!" );
    }
    shared_ptr<TAlpide> myChip = nullptr;
    for (unsigned int i = 0; i < fChips.size(); i++) {
        shared_ptr<TChipConfig> config = (fChips.at(i)->GetConfig()).lock();
        if ((unsigned int)config->GetChipId() == chipId) {
            myChip = fChips.at(i);
            break;
        }
    }
    if ( !myChip ) {
        cerr << "TDevice::GetChipById() - requested chip id = " << chipId << endl;
        throw runtime_error( "TDevice::GetChipById() - requested chip id not found." );
    }
    return myChip;
}

//___________________________________________________________________
unsigned int TDevice::GetChipId( const unsigned int iChip ) const
{
    if ( fChips.empty() ) {
        throw runtime_error( "TDevice::GetChip() - no chip defined!" );
    }
    if ( iChip >=fChips.size() ) {
        cerr << "TDevice::GetChip() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetChip() - wrong chip index!" );
    }
    shared_ptr<TChipConfig> config = (fChips.at(iChip)->GetConfig()).lock();
    if ( !config ) {
        cerr << "TDevice::GetChipId() - iChip = " << iChip << endl;
        throw runtime_error( "TDevice::GetChipId() - config for this chip does not exist!" );
    }
    return (unsigned int)config->GetChipId();
}

//___________________________________________________________________
unsigned int TDevice::GetChipIndexById( const unsigned int chipId ) const
{
    if ( (!IsSetupFrozen()) || fChipConfigs.empty() ||  fChips.empty() ) {
        throw runtime_error( "TDevice::GetChipById() - no chip or chip config defined!" );
    }
    shared_ptr<TAlpide> myChip = nullptr;
    int index = -1;
    for (unsigned int i = 0; i < fChips.size(); i++) {
        shared_ptr<TChipConfig> config = (fChips.at(i)->GetConfig()).lock();
        if ((unsigned int)config->GetChipId() == chipId) {
            myChip = fChips.at(i);
            index = i;
            break;
        }
    }
    if ( !myChip ) {
        cerr << "TDevice::GetChipIndexById() - null TAlpide ptr at requested chip id = " << chipId << endl;
    }
    if ( index < 0 ) {
        cerr << "TDevice::GetChipIndexById() - requested chip id = " << chipId << endl;
        throw runtime_error( "TDevice::GetChipIndexById() - chip index not found for the requested chip id!" );
    }
    return (unsigned int)index;
}

//___________________________________________________________________
common::TChipIndex TDevice::GetWorkingChipIndexdByBoardReceiver( const unsigned int iBoard,
                                                                const unsigned int rcv ) const
{
    if ( !GetNWorkingChips() ) {
        throw runtime_error( "TDevice::GetChipIdByBoardReceiver() - no existing working chip!" );
    }
    bool found = false;
    for ( auto it = fWorkingChipIndexList.begin(); it != fWorkingChipIndexList.end(); ++it ) {
        if ( (iBoard == (*it).boardIndex) && (rcv == (*it).dataReceiver) ) {
            return *it;
        }
    }
    if ( !found ) {
        cerr << "TDevice::GetChipIdByBoardReceiver() - requested board receiver id = " << rcv << endl;
        throw runtime_error( "TDevice::GetChipIdByBoardReceiver() - chip id not found for the requested board receiver id!" );
    }
    auto it = fWorkingChipIndexList.begin();
    return *it;
}

//___________________________________________________________________
common::TChipIndex TDevice::GetWorkingChipIndex( const unsigned iChip ) const
{
    if ( !GetNWorkingChips() ) {
        throw runtime_error( "TDevice::GetChipIdByBoardReceiver() - no existing working chip!" );
    }
    if ( iChip >= GetNWorkingChips() ) {
        cerr << "TDevice::GetWorkingChipIndex() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetWorkingChipIndex() - wrong chip index!" );
    }
    return fWorkingChipIndexList.at( iChip );
}

//___________________________________________________________________
shared_ptr<TChipConfig> TDevice::GetChipConfig( const unsigned int iChip )
{
    if ( fChipConfigs.empty()  ) {
        throw runtime_error( "TDevice::GetChipConfig() - no config defined!" );
    }
    if ( iChip >= fChipConfigs.size() ) {
        cerr << "TDevice::GetChipConfig() - iChip = " << iChip << endl;
        throw out_of_range( "TDevice::GetChipConfig() - wrong chip config index!" );
    }
    shared_ptr<TChipConfig> myChipConfig = fChipConfigs.at( iChip );
    if ( !myChipConfig ) {
        cerr << "TDevice::GetChipConfig() - iChip = " << iChip << endl;
        throw runtime_error( "TDevice::GetChipConfig() - chip config does not exist!" );
    }
    return myChipConfig;
}

//___________________________________________________________________
shared_ptr<TChipConfig> TDevice::GetChipConfigById( const unsigned int chipId )
{
    if ( fChipConfigs.empty() ) {
        throw runtime_error( "TDevice::GetChipConfigById() - no config defined!" );
    }
    shared_ptr<TChipConfig> config = nullptr;
    for (unsigned int i = 0; i < fChipConfigs.size(); i++) {
        if ((unsigned int)(fChipConfigs.at(i)->GetChipId()) == chipId) {
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
unsigned int TDevice::GetNChips() const
{
    if ( fCreatedConfig ) {
        return fChipConfigs.size();
    } else {
        return fNChips;
    }
}

//___________________________________________________________________
unsigned int TDevice::GetChipConfigsVectorSize() const
{
    return fChipConfigs.size();
}

//___________________________________________________________________
unsigned int TDevice::GetNBoards( const bool useBoardConfigVector ) const
{
    if ( useBoardConfigVector ) {
        return fBoardConfigs.size();
    } else {
        return fBoards.size();
    }
}

//___________________________________________________________________
unsigned int TDevice::GetStartChipId()
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
    if ( (GetDeviceType() == TDeviceType::kMFT_LADDER5)
        || (GetDeviceType() == TDeviceType::kMFT_LADDER4)
        || (GetDeviceType() == TDeviceType::kMFT_LADDER3)
        || (GetDeviceType() == TDeviceType::kMFT_LADDER2) ) {
        return true;
    } else {
        return false;
    }
}

//___________________________________________________________________
unsigned int TDevice::GetNWorkingChipsPerBoard( const unsigned int iBoard ) const
{
    if ( fBoards.empty() ) {
        throw runtime_error( "TDevice::GetNWorkingChipsPerBoard() - no existing board!" );
    }
    if ( (iBoard >= fBoards.size())
        || (iBoard >= fNWorkingChipsPerBoard.size()) ) {
        cerr << "TDevice::GetNWorkingChipsPerBoard() - iBoard = " << iBoard << endl;
        throw out_of_range( "TDevice::GetNWorkingChipsPerBoard() - wrong board config index!" );
    }
    return fNWorkingChipsPerBoard.at( iBoard );
}

//___________________________________________________________________
bool TDevice::IsValidChipIndex( const common::TChipIndex idx ) const
{
    if ( !GetNWorkingChips() ) {
        throw runtime_error( "TDevice::IsValidChipIndex() - no existing working chip!" );
    }
    for ( auto it = fWorkingChipIndexList.begin(); it != fWorkingChipIndexList.end(); ++it ) {
        if ( common::SameChipIndex( idx, *it ) ) {
            return true;
        }
    }
    return false;
}

//___________________________________________________________________
bool TDevice::IsValidChipId( const unsigned int chipId ) const
{
    if ( !GetNWorkingChips() ) {
        throw runtime_error( "TDevice::IsValidChipId() - no existing working chip!" );
    }
    for ( auto it = fWorkingChipIndexList.begin(); it != fWorkingChipIndexList.end(); ++it ) {
        unsigned int legitimateChipId = (*it).chipId;
        if ( chipId == legitimateChipId ) {
            return true;
        }
    }
    return false;
}

//___________________________________________________________________
int TDevice::GetChipReceiverById( const unsigned int chipId )
{
    int rcv = GetChipConfigById( chipId )->GetReceiver();
    return rcv;
}