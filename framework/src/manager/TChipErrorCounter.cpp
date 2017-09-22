#include "TChipErrorCounter.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <stdexcept>

using namespace std;

//___________________________________________________________________
TChipErrorCounter::TChipErrorCounter() :
fNPrioEncoder( 0 ),
fNBadAddressIdFlag( 0 ),
fNBadColIdFlag( 0 ),
fNBadRegionIdFlag( 0 ),
fNStuckPixelFlag( 0 ),
fNDeadPixels( 0 ),
fNInefficientPixels( 0 ),
fNHotPixels( 0 ),
fFilledErrorCounters( false )
{
    fIdx.boardIndex = 0;
    fIdx.dataReceiver = 0;
    fIdx.chipId = 0;
}

//___________________________________________________________________
TChipErrorCounter::TChipErrorCounter( const common::TChipIndex aChipIndex ) :
fNPrioEncoder( 0 ),
fNBadAddressIdFlag( 0 ),
fNBadColIdFlag( 0 ),
fNBadRegionIdFlag( 0 ),
fNStuckPixelFlag( 0 ),
fNDeadPixels( 0 ),
fNInefficientPixels( 0 ),
fNHotPixels( 0 ),
fFilledErrorCounters( false )
{
    fIdx.boardIndex = aChipIndex.boardIndex;
    fIdx.dataReceiver = aChipIndex.dataReceiver;
    fIdx.chipId = aChipIndex.chipId;
}

//___________________________________________________________________
TChipErrorCounter::~TChipErrorCounter()
{
    fCorruptedHits.clear();
}

//___________________________________________________________________
void TChipErrorCounter::AddCorruptedHit( shared_ptr<TPixHit> badHit )
{
    if ( fFilledErrorCounters ) {
        cerr << "TChipErrorCounter::AddCorruptedHit() - counters filled, no more modification allowed !" << endl;
        return;
    }
    if ( badHit ) {
        fCorruptedHits.push_back( move(badHit) );
    }
}

//___________________________________________________________________
void TChipErrorCounter::AddDeadPixel( unsigned int icol, unsigned int iaddr )
{
    if ( fFilledErrorCounters ) {
        cerr << "TChipErrorCounter::AddDeadPixel() - counters filled, no more modification allowed !" << endl;
        return;
    }
    auto hit = make_shared<TPixHit>();
    hit->SetBoardIndex( fIdx.boardIndex );
    hit->SetBoardReceiver( fIdx.dataReceiver );
    hit->SetChipId( fIdx.chipId );
    hit->SetDoubleColumn( icol );
    hit->SetAddress( iaddr );
    float region = floor( ((float)icol)/((float)common::NDCOL_PER_REGION) );
    hit->SetRegion( (unsigned int)region );
    hit->SetPixFlag( TPixFlag::kDEAD );
    fCorruptedHits.push_back( move(hit) );
}

//___________________________________________________________________
void TChipErrorCounter::AddInefficientPixel( unsigned int icol, unsigned int iaddr )
{
    if ( fFilledErrorCounters ) {
        cerr << "TChipErrorCounter::AddInefficientPixel() - counters filled, no more modification allowed !" << endl;
        return;
    }
    auto hit = make_shared<TPixHit>();
    hit->SetBoardIndex( fIdx.boardIndex );
    hit->SetBoardReceiver( fIdx.dataReceiver );
    hit->SetChipId( fIdx.chipId );
    hit->SetDoubleColumn( icol );
    hit->SetAddress( iaddr );
    float region = floor( ((float)icol)/((float)common::NDCOL_PER_REGION) );
    hit->SetRegion( (unsigned int)region );
    hit->SetPixFlag( TPixFlag::kINEFFICIENT );
    fCorruptedHits.push_back( move(hit) );
}

//___________________________________________________________________
void TChipErrorCounter::AddHotPixel( unsigned int icol, unsigned int iaddr )
{
    if ( fFilledErrorCounters ) {
        cerr << "TChipErrorCounter::AddHotPixel() - counters filled, no more modification allowed !" << endl;
        return;
    }
    auto hit = make_shared<TPixHit>();
    hit->SetBoardIndex( fIdx.boardIndex );
    hit->SetBoardReceiver( fIdx.dataReceiver );
    hit->SetChipId( fIdx.chipId );
    hit->SetDoubleColumn( icol );
    hit->SetAddress( iaddr );
    float region = floor( ((float)icol)/((float)common::NDCOL_PER_REGION) );
    hit->SetRegion( (unsigned int)region );
    hit->SetPixFlag( TPixFlag::kHOT );
    fCorruptedHits.push_back( move(hit) );
}

//___________________________________________________________________
void TChipErrorCounter::FindCorruptedHits()
{
    if ( fFilledErrorCounters ) {
        return;
    }
    FindCorruptedHits( TPixFlag::kBAD_REGIONID );
    FindCorruptedHits( TPixFlag::kBAD_DCOLID );
    FindCorruptedHits( TPixFlag::kBAD_ADDRESS );
    FindCorruptedHits( TPixFlag::kSTUCK );
    FindCorruptedHits( TPixFlag::kDEAD );
    FindCorruptedHits( TPixFlag::kINEFFICIENT );
    FindCorruptedHits( TPixFlag::kHOT );
    fFilledErrorCounters = true;
}

//___________________________________________________________________
void TChipErrorCounter::Dump()
{
    if ( !fFilledErrorCounters ) {
        cerr << "TChipErrorCounter::Dump() - counters empty ! Please use FindCorruptedHits() first. " << endl;
        return;
    }
    cout << endl;
    cout << "------------------------------- TChipErrorCounter::Dump() " << endl;
    cout << "Board . receiver / chip: "
         << std::dec << fIdx.boardIndex << " . "
         << fIdx.dataReceiver << " / " << fIdx.chipId << endl;
    cout << "Number of priority encoder errors: " << fNPrioEncoder << endl;
    if ( fCorruptedHits.size() ) {
        cout << "Number of hits with bad region id flag: " << fNBadRegionIdFlag << endl;
        cout << "Number of hits with bad col id flag: " << fNBadColIdFlag << endl;
        cout << "Number of hits with bad address flag: " << fNBadAddressIdFlag << endl;
        cout << "Number of hits with stuck pixel flag: " << fNStuckPixelFlag << endl;
        cout << "Number of dead pixels: " << fNDeadPixels << endl;
        cout << "Number of inefficient pixels: " << fNInefficientPixels << endl;
        cout << "Number of hot pixels: " << fNHotPixels << endl;
    }
    cout << "-------------------------------" << endl << endl;
}

//___________________________________________________________________
void TChipErrorCounter::WriteCorruptedHitsToFile( const char *fName, bool Recreate )
{
    if ( !fFilledErrorCounters ) {
        cerr << "TChipErrorCounter::WriteCorruptedHitsToFile() - counters empty ! Please use FindCorruptedHits() first. " << endl;
        return;
    }
    WriteCorruptedHitsToFile( TPixFlag::kBAD_REGIONID, fName, Recreate );
    WriteCorruptedHitsToFile( TPixFlag::kBAD_DCOLID, fName, Recreate );
    WriteCorruptedHitsToFile( TPixFlag::kBAD_ADDRESS, fName, Recreate );
    WriteCorruptedHitsToFile( TPixFlag::kSTUCK, fName, Recreate );
    WriteCorruptedHitsToFile( TPixFlag::kDEAD, fName, Recreate );
    WriteCorruptedHitsToFile( TPixFlag::kINEFFICIENT, fName, Recreate );
    WriteCorruptedHitsToFile( TPixFlag::kHOT, fName, Recreate );
}


//___________________________________________________________________
void TChipErrorCounter::FindCorruptedHits( const TPixFlag flag )
{
    if( fCorruptedHits.size() ) {
 
        if ( GetVerboseLevel() > kCHATTY) {
            cout << endl;
            cout << "------------------------------- TChipErrorCounter::FindCorruptedHits() "
            << endl;
        }
        switch ( (int)flag ) {
            case (int)TPixFlag::kUNKNOWN : // dump all hits, no matter what their flag is
                if ( GetVerboseLevel() > kCHATTY ) {
                    cout << "\t hit board.receiver / chip / region.dcol.add (flag) " ;
                    for ( unsigned int i = 0; i < fCorruptedHits.size(); i++ ) {
                        (fCorruptedHits.at(i))->DumpPixHit(false);
                    }
                }
                break;
            case (int)TPixFlag::kOK : // nothing to do, since all hits in the vector are bad
                break;
            case (int)TPixFlag::kBAD_CHIPID : // nothing to do, since a bad chip id can not be attached (and hence found) to (in) a given TChipErrorCounter
                break;
            default: // select bad hits with the requested flag
                bool first = true;
                unsigned int counter = 0;
                for ( unsigned int i = 0; i < fCorruptedHits.size(); i++ ) {
                    if ( (fCorruptedHits.at(i))->GetPixFlag() == flag ) {
                        if ( GetVerboseLevel() > kCHATTY ) {
                            (fCorruptedHits.at(i))->DumpPixHit(first);
                            first = false;
                        }
                        counter++;
                    }
                }
                if ( GetVerboseLevel() > kCHATTY ) {
                    cout << "\t Total: " << std::dec << counter << " hits with flag ";
                }
                if ( flag == TPixFlag::kBAD_ADDRESS ) {
                    if ( GetVerboseLevel() > kCHATTY ) {
                        cout << "TPixFlag::kBAD_ADDRESS" << endl;
                    }
                    fNBadAddressIdFlag = counter;
                }
                if ( flag == TPixFlag::kBAD_DCOLID ) {
                    if ( GetVerboseLevel() > kCHATTY ) {
                        cout << "TPixFlag::kBAD_DCOLID" << endl;
                    }
                    fNBadColIdFlag = counter;
                }
                if ( flag == TPixFlag::kBAD_REGIONID ) {
                    if ( GetVerboseLevel() > kCHATTY ) {
                        cout << "TPixFlag::kBAD_REGIONID" << endl;
                    }
                    fNBadRegionIdFlag = counter;
                }
                if ( flag == TPixFlag::kSTUCK ) {
                    if ( GetVerboseLevel() > kCHATTY ) {
                        cout << "TPixFlag::kSTUCK" << endl;
                    }
                    fNStuckPixelFlag = counter;
                }
                if ( flag == TPixFlag::kDEAD ) {
                    if ( GetVerboseLevel() > kCHATTY ) {
                        cout << "TPixFlag::kDEAD" << endl;
                    }
                    fNDeadPixels = counter;
                }
                if ( flag == TPixFlag::kINEFFICIENT ) {
                    if ( GetVerboseLevel() > kCHATTY ) {
                        cout << "TPixFlag::kINEFFICIENT" << endl;
                    }
                    fNInefficientPixels = counter;
                }
                if ( flag == TPixFlag::kHOT ) {
                    if ( GetVerboseLevel() > kCHATTY ) {
                        cout << "TPixFlag::kHOT" << endl;
                    }
                    fNHotPixels = counter;
                }
                break;
        }
    }
}

//___________________________________________________________________
void TChipErrorCounter::WriteCorruptedHitsToFile( const TPixFlag flag,
                                                  const char *fName,
                                                  bool Recreate )
{
    // excluded flags
    
    if ( (flag ==  TPixFlag::kOK)
        || (flag == TPixFlag::kBAD_CHIPID)
        || (flag == TPixFlag::kUNKNOWN) ) {
        cerr << "TChipErrorCounter::WriteCorruptedHitsToFile() - wrong flag !" << endl;
        return;
    }
    
    // file will only be written if there is any bad hit corresponding to the flag
    
    if ( !fCorruptedHits.size() ) {
        cout << "TChipErrorCounter::WriteCorruptedHitsToFile() - no bad hits => no file will be written !" << endl;
        return;
    }
    if ( (flag == TPixFlag::kBAD_ADDRESS) && !fNBadAddressIdFlag ) {
        return;
    }
    if ( (flag == TPixFlag::kBAD_DCOLID) && !fNBadColIdFlag ) {
        return;
    }
    if ( (flag == TPixFlag::kBAD_REGIONID) && !fNBadRegionIdFlag ) {
        return;
    }
    if ( (flag == TPixFlag::kSTUCK) && !fNStuckPixelFlag ) {
        return;
    }
    if ( (flag == TPixFlag::kDEAD) && !fNDeadPixels ) {
        return;
    }
    if ( (flag == TPixFlag::kINEFFICIENT) && !fNInefficientPixels ) {
        return;
    }
    if ( (flag == TPixFlag::kHOT) && !fNHotPixels ) {
        return;
    }
    
    char  fNameChip[100];
    FILE *fp;
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    string suffix( fNameTemp );
    stringstream flag_name; flag_name << "-Error" << (int)flag;
    
    string filename = common::GetFileName( fIdx, suffix, flag_name.str() );
    if ( GetVerboseLevel() > kSILENT ) {
        cout << "TChipErrorCounter::WriteCorruptedHitsToFile() - Chip ID = "<< fIdx.chipId << endl;
    }
    strcpy( fNameChip, filename.c_str() );
    
    if ( GetVerboseLevel() > kSILENT ) {
        cout << "TChipErrorCounter::WriteCorruptedHitsToFile() - Writing bad hits " ;
        if ( flag == TPixFlag::kBAD_ADDRESS ) {
            cout << "TPixFlag::kBAD_ADDRESS";
        }
        if ( flag == TPixFlag::kBAD_DCOLID ) {
            cout << "TPixFlag::kBAD_DCOLID";
        }
        if ( flag == TPixFlag::kBAD_REGIONID ) {
            cout << "TPixFlag::kBAD_REGIONID";
        }
        if ( flag == TPixFlag::kSTUCK ) {
            cout << "TPixFlag::kSTUCK";
        }
        if ( flag == TPixFlag::kDEAD ) {
            cout << "TPixFlag::kDEAD";
        }
        if ( flag == TPixFlag::kINEFFICIENT ) {
            cout << "TPixFlag::kINEFFICIENT";
        }
        if ( flag == TPixFlag::kHOT ) {
            cout << "TPixFlag::kHOT";
        }
        cout << " to file " << fNameChip << endl;
    }
    if ( Recreate ) fp = fopen(fNameChip, "w");
    else            fp = fopen(fNameChip, "a");
    if ( !fp ) {
        throw runtime_error( "TChipErrorCounter::WriteCorruptedHitsToFile() - output file not found." );
    }
    const int nhit = 1;
    for ( unsigned int i = 0; i < fCorruptedHits.size(); i++ ) {
        if ( (fCorruptedHits.at(i))->GetPixFlag() == flag ) {
            fprintf(fp, "%d %d %d\n",
                    (fCorruptedHits.at(i))->GetDoubleColumn(),
                    (fCorruptedHits.at(i))->GetAddress(), nhit);
        }
    }
    if (fp) fclose (fp);
}


