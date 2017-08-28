#include "TChip.h"
#include "TReadoutBoard.h"

TReadoutBoard::TReadoutBoard() :
fBoardConfig( nullptr )
{
    
}

TReadoutBoard::TReadoutBoard (TBoardConfig *config)
{
    fBoardConfig = config;
}

TReadoutBoard::~TReadoutBoard()
{
    fChipPositions.clear();
}

int TReadoutBoard::AddChip( uint8_t chipId, int controlInterface, int receiver )
{
    if ( GetControlInterface( chipId ) > TChipData::kInitValue ) {
        throw TReadoutBoardError( "TReadoutBoard::AddChip() - duplicate chip id" );
    }
    auto newChip = make_shared<TChip>( chipId, controlInterface, receiver );
    newChip->SetEnable( true );                 // create chip positions by default enabled?
    fChipPositions.push_back( move(newChip) );
    return 0;
}


int TReadoutBoard::GetChipById( uint8_t chipId )
{
    bool found = false;
    int position = TChipData::kInitValue;
    for ( int i = 0; i < (int)fChipPositions.size(); i++ ) {
        if ( (fChipPositions.at(i))->GetChipId() == chipId ) {
            position = i;
            found = true;
            break;
        }
    }
    if ( !found ) {
        throw TReadoutBoardError( "TReadoutBoard::GetChipById() - non existing chip" );
    }
    return position;
}


int TReadoutBoard::GetControlInterface( uint8_t chipId )
{
    int chip = GetChipById( chipId );
    return (fChipPositions.at(chip))->GetControlInterface();
}


int TReadoutBoard::GetReceiver( uint8_t chipId )
{
    int chip = GetChipById( chipId );
    return (fChipPositions.at(chip))->GetReceiver();
}


void TReadoutBoard::SetControlInterface( uint8_t chipId, int controlInterface )
{
    int chip = GetChipById( chipId );
    (fChipPositions.at(chip))->SetControlInterface( controlInterface );
}


void TReadoutBoard::SetReceiver( uint8_t chipId, int receiver )
{
    int chip = GetChipById( chipId );
    (fChipPositions.at(chip))->SetReceiver( receiver );
}


void TReadoutBoard::SetChipEnable( uint8_t chipId, bool Enable )
{
    bool found = false;
    for ( int i = 0; i < (int)fChipPositions.size(); i++ ) {
        if ( (fChipPositions.at(i))->GetChipId() == chipId) {
            (fChipPositions.at(i))->SetEnable( Enable );
            found = true;
            break;
        }
    }
    if ( !found ) {
        throw TReadoutBoardError( "TReadoutBoard::SetChipEnable() - non existing chip" );
    }
}

TReadoutBoardError::TReadoutBoardError( const string& arg )
{
    msg = "ERROR > " + arg;
}



