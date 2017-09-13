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
TPixHit::TPixHit( const TPixHit& obj )
{
    fBoardIndex = obj.fBoardIndex;
    fBoardReceiver = obj.fBoardReceiver;
    fChipId = obj.fChipId;
    fRegion = obj.fRegion;
    fDcol = obj.fDcol;
    fAddress = obj.fAddress;
    fFlag = obj.fFlag;
}

//___________________________________________________________________
TPixHit::TPixHit( const shared_ptr<TPixHit> obj )
{
    if ( obj ) {
        fBoardIndex = obj->GetBoardIndex();
        fBoardReceiver = obj->GetBoardReceiver();
        fChipId = obj->GetChipId();
        fRegion = obj->GetRegion();
        fDcol = obj->GetDoubleColumn();
        fAddress = obj->GetAddress();
        fFlag = obj->GetPixFlag();
    }
}

//___________________________________________________________________
TPixHit::~TPixHit()
{
    
}

//___________________________________________________________________
TPixHit& TPixHit::operator=( const TPixHit& rhs)
{
    if ( &rhs != this ) {
        fBoardIndex = rhs.fBoardIndex;
        fBoardReceiver = rhs.fBoardReceiver;
        fChipId = rhs.fChipId;
        fRegion = rhs.fRegion;
        fDcol = rhs.fDcol;
        fAddress = rhs.fAddress;
        fFlag = rhs.fFlag;
    }
    return *this;
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
    if ( value > common::MAX_REGION  ) {
        cerr << "TPixHit::SetRegion() - Warning, region > 31" << endl;
        fFlag = TPixFlag::kBAD_REGIONID;
    }
    fRegion = value;
}

//___________________________________________________________________
void TPixHit::SetDoubleColumn( const unsigned int value )
{
    unsigned int dcol = value + GetRegion() * common::NDCOL_PER_REGION;
    if ( dcol > common::MAX_DCOL ) {
        cerr << "TPixHit::SetDoubleColumn() - Warning, double column > 511" << endl;
        fFlag = TPixFlag::kBAD_DCOLID;
    }
    fDcol = dcol;
}

//___________________________________________________________________
void TPixHit::SetAddress( const unsigned int value )
{
    if ( value > common::MAX_ADDR ) {
        cerr << "TPixHit::SetAddress() - Warning, address > 1023" << endl;
        fFlag = TPixFlag::kBAD_ADDRESS;
    }
    fAddress = value;
}

#pragma mark - getters

//___________________________________________________________________
unsigned int  TPixHit::GetChipId( const bool print_warning ) const
{
    if ( print_warning ) {
        if ( fChipId == ILLEGAL_CHIP_ID ) {
            cerr << "TPixHit::GetChipId() - Warning, illegal chip id = 15" << endl;
        }
        if ( fFlag == TPixFlag::kBAD_CHIPID ) {
            cerr << "TPixHit::GetChipId() - Warning, TPixFlag::kBAD_CHIPID" << endl;
        }
    }
    return fChipId;
}

//___________________________________________________________________
unsigned int  TPixHit::GetRegion( const bool print_warning ) const
{
    if ( print_warning ) {
        if ( fRegion > common::MAX_REGION  ) {
            cerr << "TPixHit::GetRegion() - Warning, region > 31" << endl;
        }
        if ( fFlag == TPixFlag::kBAD_REGIONID ) {
            cerr << "TPixHit::GetRegion() - Warning, TPixFlag::kBAD_REGIONID" << endl;
        }
    }
    return fRegion;
}

//___________________________________________________________________
unsigned int  TPixHit::GetDoubleColumn( const bool print_warning ) const
{
    if ( print_warning ) {
        if ( fDcol > common::MAX_DCOL ) {
            cerr << "TPixHit::GetDoubleColumn() - Warning, double column > 511" << endl;
        }
        if ( fFlag == TPixFlag::kBAD_DCOLID ) {
            cerr << "TPixHit::GetDoubleColumn() - Warning, TPixFlag::kBAD_DCOLID" << endl;
        }
    }
    return fDcol;
}

//___________________________________________________________________
unsigned int TPixHit::GetAddress( const bool print_warning ) const
{
    if ( print_warning ) {
        if ( fAddress > common::MAX_ADDR ) {
            cerr << "TPixHit::GetAddress() - Warning, address > 1023" << endl;
        }
        if ( fFlag == TPixFlag::kBAD_ADDRESS ) {
            cerr << "TPixHit::GetAddress() - Warning, TPixFlag::kBAD_ADDRESS" << endl;
        }
    }
    return fAddress;
}

//___________________________________________________________________
TPixFlag TPixHit::GetPixFlag( const bool print_warning ) const
{
    if ( print_warning ) {
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
    }
    return fFlag;
}

//___________________________________________________________________
bool TPixHit::IsPixHitCorrupted( const bool print_warning ) const
{
    if ( (int)GetPixFlag( print_warning ) ) {
        return true;
    }
    return false;
}

//___________________________________________________________________
void TPixHit::DumpPixHit()
{
    cout << "\t TPixHit::DumpPixHit()" << endl;
    cout << "\t hit board.receiver/chip/region.dcol.add (flag) " << std::dec
    << GetBoardIndex() << "."
    << GetBoardReceiver() << "/"
    << GetChipId( false ) << "/"
    << GetRegion( false ) << "."
    << GetDoubleColumn( false ) << "."
    << GetAddress( false ) << " (" ;
    if ( fFlag == TPixFlag::kBAD_ADDRESS ) {
        cout << "TPixFlag::kBAD_ADDRESS" << ") " << endl;
        return;
    }
    if ( fFlag == TPixFlag::kBAD_DCOLID ) {
        cout << "TPixFlag::kBAD_DCOLID" << ") "<< endl;
        return;
    }
    if ( fFlag == TPixFlag::kBAD_REGIONID ) {
        cout << "TPixFlag::kBAD_REGIONID" << ") "<< endl;
        return;
    }
    if ( fFlag == TPixFlag::kBAD_CHIPID ) {
        cout << "TPixFlag::kBAD_CHIPID" << ") "<< endl;
        return;
    }
    if ( fFlag == TPixFlag::kSTUCK ) {
        cout << "TPixFlag::kSTUCK" << ") "<< endl;
        return;
    }
    if ( fFlag == TPixFlag::kUNKNOWN ) {
        cout << "TPixFlag::kUNKNOWN" << ") "<< endl;
        return;
    }
    cout << "TPixFlag::kOK" << ") "<< endl;
    return;
}

