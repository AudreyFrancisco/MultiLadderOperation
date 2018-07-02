#include <iomanip>
#include <iostream>
#include <string>
#include "Common.h"

using namespace std;

//___________________________________________________________________
string common::GetFileName( const common::TChipIndex aChipIndex,
                            string suffix,
                            string optional,
                            string fileExtention )
{
    string fileName = "../../data/";
    fileName+= suffix;
    fileName+= "-B";
    fileName+= std::to_string( aChipIndex.boardIndex );
    if ( common::IsMFTladder(aChipIndex) || common::IsIBhic(aChipIndex) ) {
        if ( common::IsMFTladder(aChipIndex) ) {
            fileName+= "-ladder";
        }
        if ( common::IsIBhic(aChipIndex) ) {
            fileName+= "-ibhic";
        }
        fileName+= std::to_string( aChipIndex.deviceId );
    }
    fileName+= "-Rx";
    fileName+= std::to_string( aChipIndex.dataReceiver );
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
        && (lhs.deviceId == rhs.deviceId)
        && (lhs.chipId == rhs.chipId) ) {
        equality = true;
    }
    return equality;
}

//___________________________________________________________________
bool common::IsMFTladder( common::TChipIndex aChipIndex )
{
    if ( (aChipIndex.deviceType == TDeviceType::kMFT_LADDER2) 
        || (aChipIndex.deviceType == TDeviceType::kMFT_LADDER3) 
        || (aChipIndex.deviceType == TDeviceType::kMFT_LADDER4) 
        || (aChipIndex.deviceType == TDeviceType::kMFT_LADDER5) ) return true;
    return false;
}

//___________________________________________________________________
bool common::IsIBhic( common::TChipIndex aChipIndex )
{
    if ( aChipIndex.deviceType == TDeviceType::kIBHIC ) return true;
    return false;
}

//___________________________________________________________________
void common::DumpId( const TChipIndex aChipIndex )
{
    if ( common::IsMFTladder(aChipIndex) ) cout << "[board.ladder.rcv]chip = [" ;
    else {
            if ( common::IsIBhic(aChipIndex) ) cout << "[board.ibhic.rcv]chip = [" ;
            else cout << "[board.rcv]chip = ["; 
        }
    cout << aChipIndex.boardIndex;
    if ( common::IsMFTladder(aChipIndex) || common::IsIBhic(aChipIndex) )
        cout << "." << aChipIndex.deviceId;
    cout << "." << aChipIndex.dataReceiver;
    cout << "]" << aChipIndex.chipId;
}

//___________________________________________________________________
int common::GetMapIntIndex( const common::TChipIndex idx )
{
    int int_index =  (idx.deviceId << 12)
    | (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf );
    return int_index;
}

//___________________________________________________________________
common::TChipIndex common::GetChipIndexFromMapInt( const int intIndex )
{
    common::TChipIndex index;
    index.deviceId     = (intIndex >> 12);
    index.boardIndex   = (intIndex >> 8) & 0xf;
    index.dataReceiver = (intIndex >> 4) & 0xf;
    index.chipId       =  intIndex       & 0xf;
    return index;
}

