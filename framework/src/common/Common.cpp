#include <iomanip>
#include <string>
#include "Common.h"

using namespace std;

//___________________________________________________________________
string common::GetFileName( common::TChipIndex aChipIndex,
                            string suffix,
                            string optional,
                            string fileExtention )
{
    string fileName = "../../data/";
    fileName+= suffix;
    fileName+= "-B";
    fileName+= std::to_string( aChipIndex.boardIndex );
    fileName+= "-Rx";
    fileName+= std::to_string( aChipIndex.dataReceiver );
    if ( aChipIndex.ladderId ) {
        fileName+= "-hic";
        fileName+= std::to_string( aChipIndex.ladderId );
    }
    fileName+= "-chip";
    fileName+= std::to_string( aChipIndex.chipId );
    if ( !optional.empty() ) {
        fileName+= "-";
        fileName+= optional;
    }
    fileName+= fileExtention;
    
    return fileName;
}

//___________________________________________________________________
bool common::SameChipIndex( const common::TChipIndex lhs, const common::TChipIndex rhs )
{
    bool equality = false;
    if ( (lhs.boardIndex == rhs.boardIndex)
        && (lhs.dataReceiver == rhs.dataReceiver)
        && (lhs.ladderId == rhs.ladderId)
        && (lhs.chipId == rhs.chipId) ) {
        equality = true;
    }
    return equality;
}

//___________________________________________________________________
int common::GetMapIntIndex( const common::TChipIndex idx )
{
    int int_index =  (idx.ladderId << 12)
    | (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf );
    return int_index;
}

//___________________________________________________________________
common::TChipIndex common::GetChipIndexFromMapInt( const int intIndex )
{
    common::TChipIndex index;
    index.ladderId     = (intIndex >> 12);
    index.boardIndex   = (intIndex >> 8) & 0xf;
    index.dataReceiver = (intIndex >> 4) & 0xf;
    index.chipId       =  intIndex       & 0xf;
    return index;
}

