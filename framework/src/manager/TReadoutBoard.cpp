#include <exception>
#include <stdexcept>
#include "TChipConfig.h"
#include "TBoardConfig.h"
#include "TReadoutBoard.h"

using namespace std;

//___________________________________________________________________
TReadoutBoard::TReadoutBoard()
{ }

//___________________________________________________________________
TReadoutBoard::TReadoutBoard( shared_ptr<TBoardConfig> config )
{
    if ( !config ) {
        throw runtime_error( "TReadoutBoard::TReadoutBoard() - board config. is a nullptr !" );
    }
}

//___________________________________________________________________
TReadoutBoard::~TReadoutBoard()
{
    fChipPositions.clear();
}

//___________________________________________________________________
void TReadoutBoard::AddChipConfig( shared_ptr<TChipConfig> newChipConfig )
{
    if ( !newChipConfig ) {
        throw invalid_argument( "TReadoutBoard::AddChipConfig() - null pointer");
    }
    for ( unsigned int ii = 0; ii < fChipPositions.size() ; ii++ ) {
        shared_ptr<TChipConfig> sp_chip = (fChipPositions.at(ii)).lock();
        if ( newChipConfig->GetChipId() == sp_chip->GetChipId() ) {
            throw invalid_argument( "TReadoutBoard::AddChipConfig() - duplicate chip id" );
        }
    }
    fChipPositions.push_back( newChipConfig );
}

//___________________________________________________________________
int TReadoutBoard::GetControlInterface( const uint8_t chipId ) const
{
    int chip = GetChipById( chipId );
    shared_ptr<TChipConfig> sp_chip = (fChipPositions.at(chip)).lock();
    return sp_chip->GetControlInterface();
}


//___________________________________________________________________
int TReadoutBoard::GetChipById( const uint8_t chipId ) const
{
    bool found = false;
    int position = TChipConfigData::kInitValue;
    for ( unsigned int i = 0; i < fChipPositions.size(); i++ ) {
        shared_ptr<TChipConfig> sp_chip = (fChipPositions.at(i)).lock();
        if ( sp_chip->GetChipId() == chipId ) {
            position = i;
            found = true;
            break;
        }
    }
    if ( !found ) {
        throw invalid_argument( "TReadoutBoard::GetChipById() - non existing chip" );
    }
    return position;
}

//___________________________________________________________________
int TReadoutBoard::GetReceiver( const uint8_t chipId ) const
{
    int chip = GetChipById( chipId );
    shared_ptr<TChipConfig> sp_chip = (fChipPositions.at(chip)).lock();
    return sp_chip->GetReceiver();
}


