#include "TPixHit.h"
#include <iostream>

using namespace std;

//___________________________________________________________________
TPixHit::TPixHit() :
    fBoardIndex( 0 ),
    fBoardReceiver( 0 ),
    fChipId( ILLEGAL_CHIP_ID ),
    fRegion( 0 ),
    fDcol( 0 ),
    fAddress( 0 )
{
    
}

//___________________________________________________________________
TPixHit::~TPixHit()
{
    
}

#pragma mark - setters

//___________________________________________________________________
void TPixHit::SetChipId( const unsigned int value )
{
    if ( value == ILLEGAL_CHIP_ID ) {
        cerr << "TPixHit::SetChipId() - Warning, illegal chip id = 15" << endl;
    }
    fChipId = value;
}

//___________________________________________________________________
void TPixHit::SetRegion( const unsigned int value )
{
    if ( value > MAX_REGION  ) {
        cerr << "TPixHit::SetRegion() - Warning, region > 31" << endl;
    }
    fRegion = value;
}

//___________________________________________________________________
void TPixHit::SetDoubleColumn( const unsigned int value )
{
    if ( value > MAX_DCOL ) {
        cerr << "TPixHit::SetDoubleColumn() - Warning, double column > 15" << endl;
    }
    fDcol = value;
}

//___________________________________________________________________
void TPixHit::SetAddress( const unsigned int value )
{
    if ( value > MAX_ADDR ) {
        cerr << "TPixHit::SetAddress() - Warning, address > 1023" << endl;
    }
    fAddress = value;
}

#pragma mark - getters

//___________________________________________________________________
unsigned int  TPixHit::GetChipId() const
{
    if ( fChipId == ILLEGAL_CHIP_ID ) {
        cerr << "TPixHit::GetChipId() - Warning, illegal chip id = 15" << endl;
    }
    return fChipId;
}

//___________________________________________________________________
unsigned int  TPixHit::GetRegion() const
{
    if ( fRegion > MAX_REGION  ) {
        cerr << "TPixHit::GetRegion() - Warning, region > 31" << endl;
    }
    return fRegion;
}

//___________________________________________________________________
unsigned int  TPixHit::GetDoubleColumn() const
{
    if ( fDcol > MAX_DCOL ) {
        cerr << "TPixHit::GetDoubleColumn() - Warning, double column > 15" << endl;
    }
    return fDcol;
}

//___________________________________________________________________
unsigned int TPixHit::GetAddress() const
{
    if ( fAddress > MAX_ADDR ) {
        cerr << "TPixHit::GetAddress() - Warning, address > 1023" << endl;
    }
    return fAddress;
}

