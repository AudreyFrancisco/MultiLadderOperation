#include "TErrorCounter.h"
#include "THisto.h"
#include <iostream>

using namespace std;

//___________________________________________________________________
TErrorCounter::TErrorCounter() :
fNTimeout( 0 ),
fN8b10b( 0 ),
fNCorruptEvent( 0 )
{
    
}

//___________________________________________________________________
TErrorCounter::~TErrorCounter()
{
    fBadChipIdHits.clear();
}

//___________________________________________________________________
void TErrorCounter::Init( shared_ptr<TScanHisto> aScanHisto )
{
    if ( !aScanHisto ) {
        throw runtime_error( "TErrorCounter::CreateCounterCollection() - can not use a null pointer !" );
    }
    for ( unsigned int i = 0; i < aScanHisto->GetChipListSize(); i++ ) {
        common::TChipIndex aChipIndex = aScanHisto->GetChipIndex(i) ;
        AddChipErrorCounter( aChipIndex );
    }
}

//___________________________________________________________________
void TErrorCounter::AddCorruptedHit( shared_ptr<TPixHit> badHit )
{
    if ( badHit ) {
        
        if ( badHit->GetPixFlag() != TPixFlag::kBAD_CHIPID ) {
            common::TChipIndex idx;
            idx.boardIndex    = badHit->GetBoardIndex();
            idx.dataReceiver  = badHit->GetBoardReceiver();
            idx.chipId        = badHit->GetChipId();
            try {
                (fCounterCollection.at( GetMapIntIndex(idx) )).AddCorruptedHit( badHit );
            } catch ( exception& msg ) {
                cerr << "TErrorCounter::AddCorruptedHit() - " << msg.what() << endl;
            }
        } else {
            fBadChipIdHits.push_back( move(badHit) );
        }
    }
}

//___________________________________________________________________
void TErrorCounter::AddDeadPixel( common::TChipIndex idx,
                                 unsigned int icol, unsigned int iaddr )
{
    if ( !fCounterCollection.size() ) {
        throw runtime_error( "TErrorCounter::AddDeadPixel() - no chip in the list ! Please use Init() first." );
    }
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).AddDeadPixel( icol, iaddr );
    } catch ( exception& msg ) {
        cerr << "TErrorCounter::AddDeadPixel() - " << msg.what() << endl;
    }
}

//___________________________________________________________________
void TErrorCounter::AddInefficientPixel( common::TChipIndex idx,
                                       unsigned int icol, unsigned int iaddr )
{
    if ( !fCounterCollection.size() ) {
        throw runtime_error( "TErrorCounter::AddInefficientPixel() - no chip in the list ! Please use Init() first." );
    }
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).AddInefficientPixel( icol, iaddr );
    } catch ( exception& msg ) {
        cerr << "TErrorCounter::AddInefficientPixel() - " << msg.what() << endl;
    }
}

//___________________________________________________________________
void TErrorCounter::AddHotPixel( common::TChipIndex idx,
                                 unsigned int icol, unsigned int iaddr )
{
    if ( !fCounterCollection.size() ) {
        throw runtime_error( "TErrorCounter::AddHotPixel() - no chip in the list ! Please use Init() first." );
    }
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).AddHotPixel( icol, iaddr );
    } catch ( exception& msg ) {
        cerr << "TErrorCounter::AddHotPixel() - " << msg.what() << endl;
    }
}

//___________________________________________________________________
void TErrorCounter::SetVerboseLevel( const int level )
{
    for ( std::map<int, TChipErrorCounter>::iterator it = fCounterCollection.begin(); it != fCounterCollection.end(); ++it ) {
        ((*it).second).SetVerboseLevel( level );
    }
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
void TErrorCounter::IncrementNPrioEncoder( std::shared_ptr<TPixHit> badHit, const unsigned int value )
{
    if ( !fCounterCollection.size() ) {
        throw runtime_error( "TErrorCounter::IncrementNPrioEncoder() - no chip in the list ! Please use Init() first." );
    }
    common::TChipIndex idx;
    idx.boardIndex    = badHit->GetBoardIndex();
    idx.dataReceiver  = badHit->GetBoardReceiver();
    idx.chipId        = badHit->GetChipId();
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).IncrementNPrioEncoder( value );
    } catch ( exception& msg ) {
        cerr << "TErrorCounter::IncrementNPrioEncoder() - " << msg.what() << endl;
    }
}

//___________________________________________________________________
void TErrorCounter::Dump()
{
    for ( std::map<int, TChipErrorCounter>::iterator it = fCounterCollection.begin(); it != fCounterCollection.end(); ++it ) {
        ((*it).second).Dump();
    }

    cout << endl;
    cout << "------------------------------- TErrorCounter::Dump() " << endl;
    cout << "Number of corrupted events: " << std::dec << fNCorruptEvent << endl;
    cout << "Number of timeout: " << fNTimeout << endl;
    cout << "Number of 8b10b encoder errors: " << fN8b10b << endl;
    cout << "Number of hits with bad chip id: " << fBadChipIdHits.size() << endl;
    cout << "-------------------------------" << endl << endl;
}

//___________________________________________________________________
void TErrorCounter::FindCorruptedHits()
{
    for ( std::map<int, TChipErrorCounter>::iterator it = fCounterCollection.begin(); it != fCounterCollection.end(); ++it ) {
        ((*it).second).FindCorruptedHits();
    }
}

//___________________________________________________________________
void TErrorCounter::WriteCorruptedHitsToFile( const char *fName, bool Recreate )
{
    for ( std::map<int, TChipErrorCounter>::iterator it = fCounterCollection.begin(); it != fCounterCollection.end(); ++it ) {
        ((*it).second).WriteCorruptedHitsToFile( fName, Recreate );
    }
}

//___________________________________________________________________
void TErrorCounter::AddChipErrorCounter( common::TChipIndex idx )
{
    int int_index = GetMapIntIndex( idx );
    TChipErrorCounter chipCounter( idx );
    fCounterCollection.insert( std::pair<int, TChipErrorCounter>(int_index, chipCounter));
}

//___________________________________________________________________
int TErrorCounter::GetMapIntIndex( common::TChipIndex idx ) const
{
    int int_index = (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf );
    return int_index;
}
