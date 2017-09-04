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
    fAddress( 0 ),
    fFlag( TPixFlag::kUNKNOWN )
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
        fFlag = TPixFlag::kBAD_CHIPID;
    }
    fChipId = value;
}

//___________________________________________________________________
void TPixHit::SetRegion( const unsigned int value )
{
    if ( value > MAX_REGION  ) {
        cerr << "TPixHit::SetRegion() - Warning, region > 31" << endl;
        fFlag = TPixFlag::kBAD_REGIONID;
    }
    fRegion = value;
}

//___________________________________________________________________
void TPixHit::SetDoubleColumn( const unsigned int value )
{
    if ( value > MAX_DCOL ) {
        cerr << "TPixHit::SetDoubleColumn() - Warning, double column > 511" << endl;
        fFlag = TPixFlag::kBAD_DCOLID;
    }
    fDcol = value;
}

//___________________________________________________________________
void TPixHit::SetAddress( const unsigned int value )
{
    if ( value > MAX_ADDR ) {
        cerr << "TPixHit::SetAddress() - Warning, address > 1023" << endl;
        fFlag = TPixFlag::kBAD_ADDRESS;
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
    if ( fFlag == TPixFlag::kBAD_CHIPID ) {
        cerr << "TPixHit::GetChipId() - Warning, TPixFlag::kBAD_CHIPID" << endl;
    }
    return fChipId;
}

//___________________________________________________________________
unsigned int  TPixHit::GetRegion() const
{
    if ( fRegion > MAX_REGION  ) {
        cerr << "TPixHit::GetRegion() - Warning, region > 31" << endl;
    }
    if ( fFlag == TPixFlag::kBAD_REGIONID ) {
        cerr << "TPixHit::GetRegion() - Warning, TPixFlag::kBAD_REGIONID" << endl;
    }
    return fRegion;
}

//___________________________________________________________________
unsigned int  TPixHit::GetDoubleColumn() const
{
    if ( fDcol > MAX_DCOL ) {
        cerr << "TPixHit::GetDoubleColumn() - Warning, double column > 511" << endl;
    }
    if ( fFlag == TPixFlag::kBAD_DCOLID ) {
        cerr << "TPixHit::GetDoubleColumn() - Warning, TPixFlag::kBAD_DCOLID" << endl;
    }
    return fDcol;
}

//___________________________________________________________________
unsigned int TPixHit::GetAddress() const
{
    if ( fAddress > MAX_ADDR ) {
        cerr << "TPixHit::GetAddress() - Warning, address > 1023" << endl;
    }
    if ( fFlag == TPixFlag::kBAD_ADDRESS ) {
        cerr << "TPixHit::GetAddress() - Warning, TPixFlag::kBAD_ADDRESS" << endl;
    }
    return fAddress;
}

//___________________________________________________________________
TPixFlag TPixHit::GetPixFlag() const
{
    if ( fFlag == TPixFlag::kBAD_ADDRESS ) {
        cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kBAD_ADDRESS" << endl;
    }
    if ( fFlag == TPixFlag::kBAD_DCOLID ) {
        cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kBAD_DCOLID" << endl;
    }
    if ( fFlag == TPixFlag::kBAD_REGIONID ) {
        cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kBAD_REGIONID" << endl;
    }
    if ( fFlag == TPixFlag::kBAD_CHIPID ) {
        cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kBAD_CHIPID" << endl;
    }
    if ( fFlag == TPixFlag::kSTUCK ) {
        cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kSTUCK" << endl;
    }
    return fFlag;
}
