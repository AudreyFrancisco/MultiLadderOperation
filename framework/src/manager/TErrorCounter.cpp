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
        fChipList.push_back( aScanHisto->GetChipIndex(i) );
    }
    for ( unsigned int i = 0; i < fChipList.size() ; i++ ) {
        AddChipErrorCounter( fChipList.at(i) );
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
    if ( !fChipList.size() ) {
        throw runtime_error( "TErrorCounter::AddDeadPixel() - no chip in the list ! Please use Init() first." );
    }
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).AddDeadPixel( idx, icol, iaddr );
    } catch ( exception& msg ) {
        cerr << "TErrorCounter::AddDeadPixel() - " << msg.what() << endl;
    }
}

//___________________________________________________________________
void TErrorCounter::AddAlmostDeadPixel( common::TChipIndex idx,
                                       unsigned int icol, unsigned int iaddr )
{
    if ( !fChipList.size() ) {
        throw runtime_error( "TErrorCounter::AddDeadPixel() - no chip in the list ! Please use Init() first." );
    }
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).AddAlmostDeadPixel( idx, icol, iaddr );
    } catch ( exception& msg ) {
        cerr << "TErrorCounter::AddAlmostDeadPixel() - " << msg.what() << endl;
    }
}

//___________________________________________________________________
void TErrorCounter::IncrementNPrioEncoder( std::shared_ptr<TPixHit> badHit, const unsigned int value )
{
    if ( !fChipList.size() ) {
        throw runtime_error( "TErrorCounter::AddDeadPixel() - no chip in the list ! Please use Init() first." );
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

    for ( unsigned int i = 0; i < fChipList.size() ; i++ ) {
        common::TChipIndex idx = fChipList.at(i);
        (fCounterCollection.at( GetMapIntIndex(idx) )).DumpCorruptedHits();
    }
    for ( unsigned int i = 0; i < fChipList.size() ; i++ ) {
        common::TChipIndex idx = fChipList.at(i);
        (fCounterCollection.at( GetMapIntIndex(idx) )).Dump( idx );
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
void TErrorCounter::AddChipErrorCounter( common::TChipIndex idx )
{
    int int_index = GetMapIntIndex( idx );
    TChipErrorCounter chipCounter;
    fCounterCollection.insert( std::pair<int, TChipErrorCounter>(int_index, chipCounter));
}

//___________________________________________________________________
int TErrorCounter::GetMapIntIndex( common::TChipIndex idx ) const
{
    int int_index = (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf );
    return int_index;
}
