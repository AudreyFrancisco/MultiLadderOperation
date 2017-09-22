#include <iomanip>
#include <string>
#include "Common.h"

using namespace std;

//___________________________________________________________________
string common::GetFileName( common::TChipIndex aChipIndex, string suffix, string optional )
{
    string fileName = "../../data/";
    fileName+= suffix;
    fileName+= "-B";
    fileName+= std::to_string( aChipIndex.boardIndex );
    fileName+= "-Rx";
    fileName+= std::to_string( aChipIndex.dataReceiver );
    fileName+= "-chip";
    fileName+= std::to_string( aChipIndex.chipId );
    if ( !optional.empty() ) {
        fileName+= "-";
        fileName+= optional;
    }
    fileName+= ".dat";
    
    return fileName;
}

//___________________________________________________________________
bool common::SameChipIndex( common::TChipIndex lhs, common::TChipIndex rhs )
{
    bool equality = false;
    if ( (lhs.boardIndex == rhs.boardIndex)
        && (lhs.dataReceiver == rhs.dataReceiver)
        && (lhs.chipId == rhs.chipId) ) {
        equality = true;
    }
    return equality;
}
