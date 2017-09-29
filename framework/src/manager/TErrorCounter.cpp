#include "TErrorCounter.h"
#include "THisto.h"

#include <iostream>

using namespace std;

//___________________________________________________________________
TErrorCounter::TErrorCounter() :
fNTimeout( 0 ),
fNCorruptEvent( 0 )
{
    
}

//___________________________________________________________________
TErrorCounter::~TErrorCounter()
{
    fBadChipIdHits.clear();
}

//___________________________________________________________________
void TErrorCounter::Init( shared_ptr<TScanHisto> aScanHisto,
                         const unsigned int nInjections )
{
    if ( !aScanHisto ) {
        throw runtime_error( "TErrorCounter::CreateCounterCollection() - can not use a null pointer !" );
    }
    for ( unsigned int i = 0; i < aScanHisto->GetChipListSize(); i++ ) {
        common::TChipIndex aChipIndex = aScanHisto->GetChipIndex(i) ;
        AddChipErrorCounter( aChipIndex, nInjections );
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
            idx.ladderId      = badHit->GetLadderId();
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
void TErrorCounter::AddDeadPixel( const common::TChipIndex idx,
                                 const unsigned int icol, const unsigned int iaddr )
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
void TErrorCounter::AddInefficientPixel( const common::TChipIndex idx,
                                        const unsigned int icol, const unsigned int iaddr,
                                        const double nhits )
{
    if ( !fCounterCollection.size() ) {
        throw runtime_error( "TErrorCounter::AddInefficientPixel() - no chip in the list ! Please use Init() first." );
    }
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).AddInefficientPixel( icol, iaddr, nhits );
    } catch ( exception& msg ) {
        cerr << "TErrorCounter::AddInefficientPixel() - " << msg.what() << endl;
    }
}

//___________________________________________________________________
void TErrorCounter::AddHotPixel( const common::TChipIndex idx,
                                const unsigned int icol, const unsigned int iaddr,
                                const double nhits )
{
    if ( !fCounterCollection.size() ) {
        throw runtime_error( "TErrorCounter::AddHotPixel() - no chip in the list ! Please use Init() first." );
    }
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).AddHotPixel( icol, iaddr, nhits );
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
void TErrorCounter::IncrementNPrioEncoder( std::shared_ptr<TPixHit> badHit,
                                           const unsigned int value )
{
    if ( !fCounterCollection.size() ) {
        throw runtime_error( "TErrorCounter::IncrementNPrioEncoder() - no chip in the list ! Please use Init() first." );
    }
    common::TChipIndex idx;
    idx.boardIndex    = badHit->GetBoardIndex();
    idx.dataReceiver  = badHit->GetBoardReceiver();
    idx.ladderId      = badHit->GetLadderId();
    idx.chipId        = badHit->GetChipId();
    try {
        (fCounterCollection.at( GetMapIntIndex(idx) )).IncrementNPrioEncoder( value );
    } catch ( exception& msg ) {
        cerr << "TErrorCounter::IncrementNPrioEncoder() - " << msg.what() << endl;
    }
}

//___________________________________________________________________
void TErrorCounter::IncrementN8b10b( const unsigned int boardReceiver,
                                     const unsigned int value )
{
    for ( std::map<int, TChipErrorCounter>::iterator it = fCounterCollection.begin(); it != fCounterCollection.end(); ++it ) {
        ((*it).second).IncrementN8b10b( boardReceiver, value );
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
void TErrorCounter::WriteCorruptedHitsToFile( const char *fName, const bool Recreate )
{
    for ( std::map<int, TChipErrorCounter>::iterator it = fCounterCollection.begin(); it != fCounterCollection.end(); ++it ) {
        ((*it).second).WriteCorruptedHitsToFile( fName, Recreate );
    }
}

//___________________________________________________________________
void TErrorCounter::DrawAndSaveToFile( const char *fName )
{
    for ( std::map<int, TChipErrorCounter>::iterator it = fCounterCollection.begin(); it != fCounterCollection.end(); ++it ) {
        ((*it).second).DrawAndSaveToFile( fName );
    }
}

//___________________________________________________________________
void TErrorCounter::AddChipErrorCounter( const common::TChipIndex idx,
                                        const unsigned int nInjections )
{
    int int_index = GetMapIntIndex( idx );

    TChipErrorCounter chipCounter( idx, nInjections  );
    fCounterCollection.insert( std::pair<int, TChipErrorCounter>(int_index, chipCounter) );
}

//___________________________________________________________________
int TErrorCounter::GetMapIntIndex( const common::TChipIndex idx ) const
{
    int int_index =  (idx.ladderId << 12)
        | (idx.boardIndex << 8) | (idx.dataReceiver << 4) | (idx.chipId & 0xf );
    return int_index;
}
