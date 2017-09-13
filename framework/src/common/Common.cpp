#include <iomanip>
#include <string>
#include "Common.h"

using namespace std;

//___________________________________________________________________
string common::GetFileName( common::TChipIndex aChipIndex, string suffix )
{
    string fileName = "../../data/";
    fileName+= suffix;
    fileName+= "-B";
    fileName+= std::to_string( aChipIndex.boardIndex );
    fileName+= "-Rx";
    fileName+= std::to_string( aChipIndex.dataReceiver );
    fileName+= "-chip";
    fileName+= std::to_string( aChipIndex.chipId );
    fileName+= ".dat";
    
    return fileName;
}

