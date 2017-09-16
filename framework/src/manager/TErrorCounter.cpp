#include "TErrorCounter.h"
#include <iostream>

using namespace std;

//___________________________________________________________________
TErrorCounter::TErrorCounter() :
fN8b10b( 0 ),
fNCorruptEvent( 0 ),
fNPrioEncoder( 0 ),
fNTimeout( 0 )
{
    
}

//___________________________________________________________________
TErrorCounter::~TErrorCounter()
{
    fCorruptedHits.clear();
}

//___________________________________________________________________
void TErrorCounter::AddCorruptedHit( shared_ptr<TPixHit> badHit )
{
    if ( badHit ) {
        fCorruptedHits.push_back( move(badHit) );
    }
}

//___________________________________________________________________
void TErrorCounter::Dump()
{
    cout << endl;
    cout << "------------------------------- TErrorCounter::Dump() " << endl;
    cout << "Number of 8b10b decoder errors: " << std::dec << fN8b10b << endl;
    cout << "Number of corrupted events: " << fNCorruptEvent << endl;
    cout << "Number of priority encoder errors: " << fNPrioEncoder << endl;
    cout << "Number of timeout: " << fNTimeout << endl;
    cout << "-------------------------------" << endl;
}

//___________________________________________________________________
void TErrorCounter::DumpCorruptedHits( const TPixFlag flag )
{
    if( fCorruptedHits.size() ) {
        cout << endl;
        cout << "------------------------------- TErrorCounter::DumpCorruptedHits() "
        << endl;
    }
    switch ( (int)flag ) {
        case (int)TPixFlag::kUNKNOWN : // dump all hits, no matter what their flag is
            cout << "\t hit board.receiver / chip / region.dcol.add (flag) " ;
            for ( unsigned int i = 0; i < fCorruptedHits.size(); i++ ) {
                (fCorruptedHits.at(i))->DumpPixHit(false);
            }
            break;
        case (int)TPixFlag::kOK : // nothing to display, since all hits in the vector are bad
            break;
        default: // dump bad hits with the requested flag
            bool first = true;
            unsigned int counter = 0;
            for ( unsigned int i = 0; i < fCorruptedHits.size(); i++ ) {
                if ( (fCorruptedHits.at(i))->GetPixFlag() == flag ) {
                    (fCorruptedHits.at(i))->DumpPixHit(first);
                    first = false;
                    counter++;
                }
            }
            cout << "\t Total: " << std::dec << counter << " hits with flag ";
            if ( flag == TPixFlag::kBAD_ADDRESS ) {
                cout << "TPixFlag::kBAD_ADDRESS" << endl;
            }
            if ( flag == TPixFlag::kBAD_DCOLID ) {
                cout << "TPixFlag::kBAD_DCOLID" << endl;
            }
            if ( flag == TPixFlag::kBAD_REGIONID ) {
                cout << "TPixFlag::kBAD_REGIONID" << endl;
            }
            if ( flag == TPixFlag::kBAD_CHIPID ) {
                cout << "TPixFlag::kBAD_CHIPID" << endl;
            }
            if ( flag == TPixFlag::kSTUCK ) {
                cout << "TPixFlag::kSTUCK" << endl;
            }
            break;
    }
    if( fCorruptedHits.size() ) {
        cout << "------------------------------- " << endl;
    }
}
