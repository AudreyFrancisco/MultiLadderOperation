#include "TPixHit.h"
#include <iostream>

using namespace std;

//___________________________________________________________________
TPixHit::TPixHit() : TVerbosity(),
    fBoardIndex( 0 ),
    fBoardReceiver( 0 ),
    fLadderId( 0 ),
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
    fLadderId = obj.fLadderId;
    fChipId = obj.fChipId;
    fRegion = obj.fRegion;
    fDcol = obj.fDcol;
    fAddress = obj.fAddress;
    fFlag = obj.fFlag;
    fVerboseLevel = obj.fVerboseLevel;
}

//___________________________________________________________________
TPixHit::TPixHit( const shared_ptr<TPixHit> obj )
{
    if ( obj ) {
        fBoardIndex = obj->GetBoardIndex();
        fBoardReceiver = obj->GetBoardReceiver();
        fLadderId = obj->fLadderId;
        fChipId = obj->GetChipId();
        fRegion = obj->GetRegion();
        fDcol = obj->GetDoubleColumn();
        fAddress = obj->GetAddress();
        fFlag = obj->GetPixFlag();
        fVerboseLevel = obj->GetVerboseLevel();
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
        fLadderId = rhs.fLadderId;
        fChipId = rhs.fChipId;
        fRegion = rhs.fRegion;
        fDcol = rhs.fDcol;
        fAddress = rhs.fAddress;
        fFlag = rhs.fFlag;
        fVerboseLevel = rhs.fVerboseLevel;
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
unsigned int  TPixHit::GetChipId() const
{
    if ( GetVerboseLevel() >= kINFINITE ) {
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
unsigned int  TPixHit::GetRegion() const
{
    if ( GetVerboseLevel() >= kINFINITE ) {
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
unsigned int  TPixHit::GetDoubleColumn() const
{
    if ( GetVerboseLevel() >= kINFINITE ) {
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
unsigned int TPixHit::GetAddress() const
{
    if ( GetVerboseLevel() >= kINFINITE ) {
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
TPixFlag TPixHit::GetPixFlag() const
{
    if ( GetVerboseLevel() >= kINFINITE ) {
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
        if ( fFlag == TPixFlag::kDEAD ) {
            cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kDEAD" << endl;
        }
        if ( fFlag == TPixFlag::kINEFFICIENT ) {
            cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kINEFFICIENT" << endl;
        }
        if ( fFlag == TPixFlag::kHOT ) {
            cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kHOT" << endl;
        }
        if ( fFlag == TPixFlag::kUNKNOWN ) {
            cerr << "TPixHit::GetPixFlag() - Warning, TPixFlag::kUNKNOWN" << endl;
        }
    }
    return fFlag;
}

//___________________________________________________________________
bool TPixHit::IsPixHitCorrupted() const
{
    if ( (int)GetPixFlag() ) {
        return true;
    }
    return false;
}

//___________________________________________________________________
unsigned int TPixHit::GetColumn() const
{
    if ( (fFlag == TPixFlag::kBAD_REGIONID)
        || (fFlag == TPixFlag::kBAD_ADDRESS)
        || (fFlag == TPixFlag::kBAD_DCOLID) ) {
        cerr << "TPixHit::GetColumn() - Warning, return value probably meaningless" << endl;
    }
    unsigned int column = fDcol * 2;
    int leftRight = ((fAddress % 4) < 2 ? 1:0); // Left or right column within the double column
    
    column += leftRight;
    
    return column;
}

//___________________________________________________________________
unsigned int TPixHit::GetRow() const
{
    if ( fFlag == TPixFlag::kBAD_ADDRESS ) {
        cerr << "TPixHit::GetRow() - Warning, return value probably meaningless" << endl;
    }
    unsigned int row = fAddress / 2;         // This is OK for the top-right and the bottom-left pixel within a group of 4
    if ((fAddress % 4) == 3) row -= 1;      // adjust the top-left pixel
    if ((fAddress % 4) == 0) row += 1;      // adjust the bottom-right pixel
    return row;
}


//___________________________________________________________________
void TPixHit::DumpPixHit( const bool with_reminder )
{
    int buffer_verbosity = GetVerboseLevel();
    SetVerboseLevel( kSILENT );
    if ( with_reminder ) {
        cout << "\t TPixHit::DumpPixHit()" << endl;
        if ( fLadderId ) {
            cout << "\t board.receiver.ladder / chip / region.dcol.add (flag) \n" ;
        } else {
            cout << "\t board.receiver / chip / region.dcol.add (flag) \n" ;
        }
    }
    cout << std::dec << "\t"
    << GetBoardIndex() << ".";
    if ( fLadderId ) {
        cout << GetBoardReceiver() << "."
             << GetLadderId() << " / ";
    } else {
        cout << GetBoardReceiver() << " / " ;
    }
    cout << GetChipId() << " / "
         << GetRegion() << "."
         << GetDoubleColumn() << "."
         << GetAddress() << " (" ;
    if ( with_reminder ) {
        if ( fFlag == TPixFlag::kBAD_ADDRESS ) {
            cout << "TPixFlag::kBAD_ADDRESS" << ") " << endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        if ( fFlag == TPixFlag::kBAD_DCOLID ) {
            cout << "TPixFlag::kBAD_DCOLID" << ") "<< endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        if ( fFlag == TPixFlag::kBAD_REGIONID ) {
            cout << "TPixFlag::kBAD_REGIONID" << ") "<< endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        if ( fFlag == TPixFlag::kBAD_CHIPID ) {
            cout << "TPixFlag::kBAD_CHIPID" << ") "<< endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        if ( fFlag == TPixFlag::kSTUCK ) {
            cout << "TPixFlag::kSTUCK" << ") "<< endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        if ( fFlag == TPixFlag::kDEAD ) {
            cout << "TPixFlag::kDEAD" << ") "<< endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        if ( fFlag == TPixFlag::kINEFFICIENT ) {
            cout << "TPixFlag::kINEFFICIENT" << ") "<< endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        if ( fFlag == TPixFlag::kHOT ) {
            cout << "TPixFlag::kHOT" << ") "<< endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        if ( fFlag == TPixFlag::kUNKNOWN ) {
            cout << "TPixFlag::kUNKNOWN" << ") "<< endl;
            SetVerboseLevel( buffer_verbosity );
            return;
        }
        cout << "TPixFlag::kOK" << ") "<< endl;
    } else {
        cout <<  (int)GetPixFlag() << ") "<< endl;
    }
    SetVerboseLevel( buffer_verbosity );
    return;
}

