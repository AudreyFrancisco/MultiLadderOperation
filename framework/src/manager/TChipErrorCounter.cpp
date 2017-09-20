#include "TChipErrorCounter.h"
#include <iostream>
#include <cmath>

using namespace std;

//___________________________________________________________________
TChipErrorCounter::TChipErrorCounter() :
fNPrioEncoder( 0 ),
fNBadAddressIdFlag( 0 ),
fNBadColIdFlag( 0 ),
fNBadRegionIdFlag( 0 ),
fNStuckPixelFlag( 0 ),
fNDeadPixels( 0 ),
fNAlmostDeadPixels( 0 )
{
    
}

//___________________________________________________________________
TChipErrorCounter::~TChipErrorCounter()
{
    fCorruptedHits.clear();
}

//___________________________________________________________________
void TChipErrorCounter::AddCorruptedHit( shared_ptr<TPixHit> badHit )
{
    if ( badHit ) {
        fCorruptedHits.push_back( move(badHit) );
    }
}

//___________________________________________________________________
void TChipErrorCounter::AddDeadPixel( common::TChipIndex idx,
                                 unsigned int icol, unsigned int iaddr )
{
    auto hit = make_shared<TPixHit>();
    hit->SetBoardIndex( idx.boardIndex );
    hit->SetBoardReceiver( idx.dataReceiver );
    hit->SetChipId( idx.chipId );
    hit->SetDoubleColumn( icol );
    hit->SetAddress( iaddr );
    float region = floor( ((float)icol)/((float)common::NDCOL_PER_REGION) );
    hit->SetRegion( (unsigned int)region );
    hit->SetPixFlag( TPixFlag::kDEAD );
    fCorruptedHits.push_back( move(hit) );
}

//___________________________________________________________________
void TChipErrorCounter::AddAlmostDeadPixel( common::TChipIndex idx,
                                       unsigned int icol, unsigned int iaddr )
{
    auto hit = make_shared<TPixHit>();
    hit->SetBoardIndex( idx.boardIndex );
    hit->SetBoardReceiver( idx.dataReceiver );
    hit->SetChipId( idx.chipId );
    hit->SetDoubleColumn( icol );
    hit->SetAddress( iaddr );
    float region = floor( ((float)icol)/((float)common::NDCOL_PER_REGION) );
    hit->SetRegion( (unsigned int)region );
    hit->SetPixFlag( TPixFlag::kALMOST_DEAD );
    fCorruptedHits.push_back( move(hit) );
}

//___________________________________________________________________
void TChipErrorCounter::DumpCorruptedHits()
{
    DumpCorruptedHits( TPixFlag::kBAD_REGIONID );
    DumpCorruptedHits( TPixFlag::kBAD_DCOLID );
    DumpCorruptedHits( TPixFlag::kBAD_ADDRESS );
    DumpCorruptedHits( TPixFlag::kSTUCK );
    DumpCorruptedHits( TPixFlag::kDEAD );
    DumpCorruptedHits( TPixFlag::kALMOST_DEAD );
}

//___________________________________________________________________
void TChipErrorCounter::Dump( common::TChipIndex idx )
{
    cout << endl;
    cout << "------------------------------- TChipErrorCounter::Dump() " << endl;
    cout << "Board . receiver / chip: "
         << std::dec << idx.boardIndex << " . "
         << idx.dataReceiver << " / " << idx.chipId << endl;
    cout << "Number of priority encoder errors: " << fNPrioEncoder << endl;
    if ( fCorruptedHits.size() ) {
        cout << "Number of hits with bad region id flag: " << fNBadRegionIdFlag << endl;
        cout << "Number of hits with bad col id flag: " << fNBadColIdFlag << endl;
        cout << "Number of hits with bad address flag: " << fNBadAddressIdFlag << endl;
        cout << "Number of hits with stuck pixel flag: " << fNStuckPixelFlag << endl;
        cout << "Number of dead pixels: " << fNDeadPixels << endl;
        cout << "Number of almost dead pixels: " << fNAlmostDeadPixels << endl;
    }
    cout << "-------------------------------" << endl << endl;
}


//___________________________________________________________________
void TChipErrorCounter::DumpCorruptedHits( const TPixFlag flag )
{
    if( fCorruptedHits.size() ) {
        cout << endl;
        cout << "------------------------------- TChipErrorCounter::DumpCorruptedHits() "
        << endl;
        
        switch ( (int)flag ) {
            case (int)TPixFlag::kUNKNOWN : // dump all hits, no matter what their flag is
                cout << "\t hit board.receiver / chip / region.dcol.add (flag) " ;
                for ( unsigned int i = 0; i < fCorruptedHits.size(); i++ ) {
                    (fCorruptedHits.at(i))->DumpPixHit(false);
                }
                break;
            case (int)TPixFlag::kOK : // nothing to display, since all hits in the vector are bad
                break;
            case (int)TPixFlag::kBAD_CHIPID : // nothing to display, since a bad chip id can not be attached (and hence found) to (in) a given TChipErrorCounter
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
                    fNBadAddressIdFlag = counter;
                }
                if ( flag == TPixFlag::kBAD_DCOLID ) {
                    cout << "TPixFlag::kBAD_DCOLID" << endl;
                    fNBadColIdFlag = counter;
                }
                if ( flag == TPixFlag::kBAD_REGIONID ) {
                    cout << "TPixFlag::kBAD_REGIONID" << endl;
                    fNBadRegionIdFlag = counter;
                }
                if ( flag == TPixFlag::kSTUCK ) {
                    cout << "TPixFlag::kSTUCK" << endl;
                    fNStuckPixelFlag = counter;
                }
                if ( flag == TPixFlag::kDEAD ) {
                    cout << "TPixFlag::kDEAD" << endl;
                    fNDeadPixels = counter;
                }
                if ( flag == TPixFlag::kALMOST_DEAD ) {
                    cout << "TPixFlag::kALMOST_DEAD" << endl;
                    fNAlmostDeadPixels = counter;
                }
                break;
        }
    }
}
