#include "TAlpideDecoder.h"
#include "TPixHit.h"
#include "THisto.h"
#include "TErrorCounter.h"
#include <stdint.h>
#include <iostream>
#include <string>
#include <bitset>
#include <stdexcept>

using namespace std;

//___________________________________________________________________
TAlpideDecoder::TAlpideDecoder() : TVerbosity(),
    fNewEvent( false ),
    fBunchCounter( 0 ),
    fFlags( 0 ),
    fChipId( -1 ),
    fRegion( -1 ),
    fBoardIndex( 0 ),
    fBoardReceiver( 0 ),
    fLadderId( 0 ),
    fDataType( TDataType::kUNKNOWN ),
    fScanHisto( nullptr ),
    fErrorCounter( nullptr )
{
    
}

//___________________________________________________________________
TAlpideDecoder::TAlpideDecoder( shared_ptr<TScanHisto> aScanHisto,
                                shared_ptr<TErrorCounter> anErrorCounter ) :
    TVerbosity(),
    fNewEvent( false ),
    fBunchCounter( 0 ),
    fFlags( 0 ),
    fChipId( -1 ),
    fRegion( -1 ),
    fBoardIndex( 0 ),
    fBoardReceiver( 0 ),
    fLadderId( 0 ),
    fDataType( TDataType::kUNKNOWN ),
    fScanHisto( nullptr ),
    fErrorCounter( nullptr )
{
    try {
        SetScanHisto( aScanHisto );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        exit(0);
    }
    try {
        SetErrorCounter( anErrorCounter );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        exit(0);
    }
}

//___________________________________________________________________
TAlpideDecoder::~TAlpideDecoder()
{
    fHits.clear();
}

//___________________________________________________________________
void TAlpideDecoder::SetScanHisto( shared_ptr<TScanHisto> aScanHisto )
{
    if ( !aScanHisto ) {
        throw runtime_error( "TAlpideDecoder::SetScanHisto() - can not use a null pointer !" );
    }
    fScanHisto = aScanHisto;
}

//___________________________________________________________________
void TAlpideDecoder::SetErrorCounter( shared_ptr<TErrorCounter> anErrorCounter )
{
    if ( !anErrorCounter ) {
        throw runtime_error( "TAlpideDecoder::SetErrorCounter() - can not use a null pointer !" );
    }
    fErrorCounter = anErrorCounter;
}

//___________________________________________________________________
void TAlpideDecoder::WriteDataToFile( const char *fName, bool Recreate )
{
    if ( !fScanHisto ) {
        throw runtime_error( "TAlpideDecoder::WriteDataToFile() - scan histo is a null pointer !" );
    }
    
    char  fNameChip[100];
    FILE *fp;
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    string suffix( fNameTemp );
    
    for ( unsigned int ichip = 0; ichip < fScanHisto->GetChipListSize(); ichip++ ) {
        
        common::TChipIndex aChipIndex = fScanHisto->GetChipIndex( ichip );
        
        if ( !HasData( aChipIndex ) ) {
            if ( GetVerboseLevel() > kSILENT ) {
                cout << "TAlpideDecoder::WriteDataToFile() - Chip ID = "
                << aChipIndex.chipId ;
                if ( aChipIndex.ladderId ) {
                    cout << " , Ladder ID = " << aChipIndex.ladderId;
                }
                cout << " : no data, skipped." <<  endl;
            }
            continue;  // write files only for chips with data
        }
        string filename = common::GetFileName( aChipIndex, suffix );
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TAlpideDecoder::WriteDataToFile() - Chip ID = "<< aChipIndex.chipId ;
            if ( aChipIndex.ladderId ) {
                cout << " , Ladder ID = " << aChipIndex.ladderId;
            }
            cout << endl;
        }
        strcpy( fNameChip, filename.c_str());
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceDigitalScan::WriteDataToFile() - Writing data to file "<< fNameChip << endl;
        }
        if ( Recreate ) fp = fopen(fNameChip, "w");
        else            fp = fopen(fNameChip, "a");
        if ( !fp ) {
            throw runtime_error( "TDeviceDigitalScan::WriteDataToFile() - output file not found." );
        }
        for ( unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++ ) {
            for ( unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++ ) {
                double hits = (*fScanHisto)(aChipIndex,icol,iaddr);
                if (hits > 0) {
                    fprintf(fp, "%d %d %d\n", icol, iaddr, (int)hits);
                }
            }
        }
        if (fp) fclose (fp);
    }
}

//___________________________________________________________________
unsigned int TAlpideDecoder::GetNHits() const
{
    if ( !fScanHisto ) {
        throw runtime_error( "TAlpideDecoder::GetNHits() - scan histo is a null pointer !" );
    }

    unsigned int nHits = 0;

    for ( unsigned int ichip = 0; ichip < fScanHisto->GetChipListSize(); ichip++ ) {
        
        common::TChipIndex aChipIndex = fScanHisto->GetChipIndex( ichip );
        nHits += fScanHisto->GetChipNEntries( aChipIndex );
    }
    return nHits;
}

//___________________________________________________________________
bool TAlpideDecoder::DecodeEvent( unsigned char* data,
                                 int nBytes,
                                 unsigned int boardIndex,
                                 unsigned int boardReceiver,
                                 unsigned int ladderId )
{
    // refresh variables for each new event
    fBunchCounter = 0;
    fFlags  = 0;
    fChipId = -1;
    fRegion = -1;
    fBoardIndex = boardIndex;
    fBoardReceiver = boardReceiver;
    fLadderId = ladderId;
    fDataType = TDataType::kUNKNOWN;
    bool started = false; // event has started, i.e. chip header has been found
    bool finished = false; // event trailer found
    bool corrupt  = false; // corrupt data found (i.e. data without region or chip)
    int byte = 0;
    
    unsigned char last;
    
    // decode the event
    while ( byte < nBytes ) {
        
        last = data[byte];
        FindDataType( data[byte] );
        
        switch ( fDataType ) {
            case TDataType::kIDLE:
                byte +=GetWordLength();
                break;
            case TDataType::kBUSYON:
                byte += GetWordLength();
                break;
            case TDataType::kBUSYOFF:
                byte += GetWordLength();
                break;
            case TDataType::kEMPTYFRAME:
                started = true;
                DecodeEmptyFrame( data + byte );
                byte += GetWordLength();
                finished = true;
                break;
            case TDataType::kCHIPHEADER:
                started = true;
                finished = false;
                DecodeChipHeader( data + byte );
                byte += GetWordLength();
                break;
            case TDataType::kCHIPTRAILER:
                if ( !started ) {
                    cerr << "TAlpideDecoder::DecodeEvent() - Error: chip trailer found before chip header" << endl;
                    return false;
                }
                if ( finished ) {
                    cerr << "TAlpideDecoder::DecodeEvent() - Error: chip trailer found after event was finished" << endl;
                    return false;
                }
                DecodeChipTrailer( data + byte );
                finished = true;
                fChipId = -1;
                byte += GetWordLength();
                break;
            case TDataType::kREGIONHEADER:
                if (!started) {
                    cerr << "TAlpideDecoder::DecodeEvent() - Error: region header found before chip header or after chip trailer" << endl;
                    return false;
                }
                DecodeRegionHeader( data + byte );
                byte += GetWordLength();
                break;
            case TDataType::kDATASHORT:
                if ( !started ) {
                    cerr << "TAlpideDecoder::DecodeEvent() - Error: hit data found before chip header or after chip trailer" << endl;
                    return false;
                }
                if ( fRegion == -1 ) {
                    cout << "TAlpideDecoder::DecodeEvent() - Warning: data word without region, skipping (Chip " << fChipId << ")" << endl;
                    corrupt = true;
                } else {
                    if ( !IsValidChipId() ) {
                        cerr << "TAlpideDecoder::DecodeEvent() - Warning: unexpected chip id (Chip " << fChipId << ") , TDataType::kDATASHORT" << endl;
                        if ( GetVerboseLevel() > kCHATTY ) {
                            for ( int i = 0; i < nBytes; i++ ) {
                                printf("%02x ", data[i]);
                            }
                            printf("\n");
                        }
                    }
                    bool corrupted = DecodeDataWord( data + byte, false );
                    if ( corrupted ) {
                        corrupt = true;
                    }
                }
                byte += GetWordLength();
                break;
            case TDataType::kDATALONG:
                if ( !started ) {
                    cerr << "TAlpideDecoder::DecodeEvent() - Error: hit data found before chip header or after chip trailer" << endl;
                    return false;
                }
                if ( fRegion == -1 ) {
                    cerr << "TAlpideDecoder::DecodeEvent() - Warning: data word without region, skipping (Chip " << fChipId << ")" << endl;
                    corrupt = true;
                } else {
                    if ( !IsValidChipId() ) {
                        cerr << "TAlpideDecoder::DecodeEvent() - Warning: unexpected chip id (Chip " << fChipId << ") , TDataType::kDATALONG" << endl;
                        if ( GetVerboseLevel() > kCHATTY ) {
                            for ( int i = 0; i < nBytes; i++ ) {
                                printf("%02x ", data[i]);
                            }
                            printf("\n");
                        }
                    }
                    bool corrupted = DecodeDataWord( data + byte, true );
                    if (corrupted) {
                        corrupt = true;
                    }
                }
                byte += GetWordLength();
                break;
            case TDataType::kUNKNOWN:
                cerr << "TAlpideDecoder::DecodeEvent() - Error: data of unknown type 0x" << std::hex << data[byte] << std::dec << endl;
                return false;
        }
    }
    
    FillHistoWithEvent();
    
    if ( started && finished ) {
        return (!corrupt);
    } else {
        if ( started && !finished ) {
            cout << "TAlpideDecoder::DecodeEvent() - Warning (chip "<< fChipId << "): event not finished at end of data, last byte was 0x" << std::hex << (int) last << std::dec << ", event length = " << nBytes << endl;
            return false;
        }
        if ( !started ) {
            cout << "TAlpideDecoder::DecodeEvent() - Warning: event not started at end of data" << endl;
            return false;
        }
    }
    
    return (!corrupt);
}

//___________________________________________________________________
void TAlpideDecoder::FindDataType( unsigned char dataWord )
{
    if      ( dataWord == 0xff )          fDataType = TDataType::kIDLE;
    else if ( dataWord == 0xf1 )          fDataType = TDataType::kBUSYON;
    else if ( dataWord == 0xf0 )          fDataType = TDataType::kBUSYOFF;
    else if ( (dataWord & 0xf0) == 0xa0 ) fDataType = TDataType::kCHIPHEADER;
    else if ( (dataWord & 0xf0) == 0xb0 ) fDataType = TDataType::kCHIPTRAILER;
    else if ( (dataWord & 0xf0) == 0xe0 ) fDataType = TDataType::kEMPTYFRAME;
    else if ( (dataWord & 0xe0) == 0xc0 ) fDataType = TDataType::kREGIONHEADER;
    else if ( (dataWord & 0xc0) == 0x40 ) fDataType = TDataType::kDATASHORT;
    else if ( (dataWord & 0xc0) == 0x0 )  fDataType = TDataType::kDATALONG;
    else fDataType = TDataType::kUNKNOWN;

    if ( GetVerboseLevel() > kCHATTY ) {
        
        cout << "TAlpideDecoder::FindDataType() - dataWord = " << endl;
        printf ("%02x ", (int)dataWord);
        cout << endl << "TAlpideDecoder::FindDataType() - fDataType = " ;
        switch ( (int)fDataType ) {
            case (int)TDataType::kIDLE:
                cout << "TDataType::kIDLE" << endl;
                break;
            case (int)TDataType::kCHIPHEADER:
                cout << "TDataType::kCHIPHEADER" << endl;
                break;
            case (int)TDataType::kCHIPTRAILER:
                cout << "TDataType::kCHIPTRAILER" << endl;
                break;
            case (int)TDataType::kEMPTYFRAME:
                cout << "TDataType::kEMPTYFRAME" << endl;
                break;
            case (int)TDataType::kREGIONHEADER:
                cout << "TDataType::kREGIONHEADER" << endl;
                break;
            case (int)TDataType::kDATASHORT:
                cout << "TDataType::kDATASHORT" << endl;
                break;
            case (int)TDataType::kDATALONG:
                cout << "TDataType::kDATALONG" << endl;
                break;
            case (int)TDataType::kBUSYON:
                cout << "TDataType::kBUSYON" << endl;
                break;
            case (int)TDataType::kBUSYOFF:
                cout << "TDataType::kBUSYOFF" << endl;
                break;
            default:
                cout << "TDataType::kUNKNOWN" << endl;
                break;
        }
    }
}

//___________________________________________________________________
int TAlpideDecoder::GetWordLength() const
{
    if ( fDataType == TDataType::kDATALONG ) {
    return 3;
  }
    else if ( (fDataType == TDataType::kDATASHORT) || (fDataType == TDataType::kCHIPHEADER) || (fDataType == TDataType::kEMPTYFRAME) ) {
    return 2;
  }
  else return 1;
}

//___________________________________________________________________
void TAlpideDecoder::DecodeChipHeader( unsigned char* data )
{
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  fBunchCounter = data_field & 0xff;
  fChipId       = (data_field >> 8) & 0xf;
  fNewEvent     = true;
}

//___________________________________________________________________
void TAlpideDecoder::DecodeChipTrailer( unsigned char* data )
{
  fFlags = data[0] & 0xf;
}

//___________________________________________________________________
void TAlpideDecoder::DecodeRegionHeader( unsigned char* data )
{
    fRegion = data[0] & 0x1f;
    fNewEvent = false;
}

//___________________________________________________________________
void TAlpideDecoder::DecodeEmptyFrame( unsigned char* data )
{
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  fBunchCounter = data_field & 0xff;
  fChipId       = (data_field >> 8) & 0xf;
}

//___________________________________________________________________
bool TAlpideDecoder::DecodeDataWord( unsigned char* data,
                                    bool datalong )
{
    auto hit = make_shared<TPixHit>();
    hit->SetVerboseLevel( this->GetVerboseLevel() );
    
    int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

    if ( GetVerboseLevel() > kCHATTY ) {
        cout << "TAlpideDecoder::DecodeDataWord() - data word = 0x" << std::hex << (int) data_field << std::dec << endl;
    }
  
    bool corrupt = false;

    hit->SetBoardIndex( fBoardIndex );
    hit->SetBoardReceiver( fBoardReceiver );
    hit->SetLadderId( fLadderId );
    hit->SetChipId( fChipId ); // first basic checks on chip id done here
    hit->SetRegion( fRegion ); // can generate a bad region flag
    hit->SetDoubleColumn( (data_field & 0x3c00) >> 10 ); // can generate a bad dcol flag
    
    unsigned int address = (data_field & 0x03ff);

    int hitmap_length;
    
    if ( datalong ) {
        hitmap_length = 7; // clustering enabled
    } else {
        hitmap_length = 0; // clustering disabled
    }

    for ( int i = -1; i < hitmap_length; i ++ ) {
        
        if ((i >= 0) && (!((data[2] >> i) & 0x1))) continue;
        shared_ptr<TPixHit> singleHit( new TPixHit( hit ) ); // deep copy

        // set hit address on the new shared ptr, can generate a bad address flag
        singleHit->SetAddress( address + (i + 1) );

        // check if pixel deserves a stuck flag
        if ( (fHits.size() > 0) && (!fNewEvent) ) {
            if ( (singleHit->GetRegion() == (fHits.back())->GetRegion())
                && ( singleHit->GetDoubleColumn() == (fHits.back())->GetDoubleColumn())
                && (singleHit->GetAddress() == (fHits.back())->GetAddress()) ) {
                cerr << "TAlpideDecoder::DecodeDataWord() - received pixel twice." << endl;
                singleHit->SetPixFlag( TPixFlag::kSTUCK );
                (fHits.back())->SetPixFlag( TPixFlag::kSTUCK );
                cerr << "\t -- current hit pixel :" << endl;
                singleHit->DumpPixHit();
                cerr << "\t -- previous hit pixel :" << endl;
                (fHits.back())->DumpPixHit();
            }
            else if ( (singleHit->GetRegion() == (fHits.back())->GetRegion() )
                     && (singleHit->GetDoubleColumn() == (fHits.back())->GetDoubleColumn())
                     && (singleHit->GetAddress() < (fHits.back())->GetAddress()) ) {
                cerr << "TAlpideDecoder::DecodeDataWord() - address of pixel is lower than previous one in same double column." << endl;
                singleHit->SetPixFlag( TPixFlag::kSTUCK );
                (fHits.back())->SetPixFlag( TPixFlag::kSTUCK );
                cerr << "\t -- current hit pixel :" << endl;
                singleHit->DumpPixHit();
                cerr << "\t -- previous hit pixel :" << endl;
                (fHits.back())->DumpPixHit();
            }
        }
        // check if chip index (board id, receiver id, chip id) belongs to
        // the list of the expected indexes for the device currently being read
        if ( !IsValidChipIndex( singleHit ) ) {
            cerr << "TAlpideDecoder::DecodeDataWord() - found bad chip index, data word = 0x" << std::hex << (int) data_field << std::dec << endl;
            singleHit->SetPixFlag( TPixFlag::kBAD_CHIPID );
            cerr << "\t -- current hit pixel :" << endl;
            singleHit->DumpPixHit();
        }
        if ( singleHit->GetPixFlag() == TPixFlag::kUNKNOWN ) { // nothing bad detected, it means that the flag still has its initialization value => the pixel hit is ok
            singleHit->SetPixFlag( TPixFlag::kOK );
        }
        if ( GetVerboseLevel() > kCHATTY ) {
            cout << "TAlpideDecoder::DecodeDataWord() - new hit found" << endl;
            singleHit->DumpPixHit();
        }
        // data word is corrupted if there is any bad hit found
        corrupt = corrupt | singleHit->IsPixHitCorrupted();
        fHits.push_back( move(singleHit) ); // vector only owns hit with the address set
        if ( GetVerboseLevel() > kCHATTY ) {
            cout << "\t TAlpideDecoder::DecodeDataWord() - hit added in vector." << endl;
        }
    }
    hit.reset();
    fNewEvent = false;
    return corrupt;
}

//___________________________________________________________________
void TAlpideDecoder::FillHistoWithEvent()
{
    if ( !fScanHisto ) {
        throw runtime_error( "TDeviceDigitalScan::FillHistoEvent() - can not use a null pointer to fScanHisto !" );
    }

    common::TChipIndex idx;
    
    for ( int i = 0; i < fHits.size(); i++ ) {

        if ( (fHits.at(i))->IsPixHitCorrupted() ) {

            if ( GetVerboseLevel() > kSILENT ) {
                cout << "TAlpideDecoder::FillHistoEvent() - bad pixel coordinates, skipping hit" << endl;
                (fHits.at(i))->DumpPixHit();
            }
            shared_ptr<TPixHit> badHit( new TPixHit( fHits.at(i) ) ); // deep copy
            fErrorCounter->AddCorruptedHit( badHit );
            
        } else {
            
            idx.boardIndex    = (fHits.at(i))->GetBoardIndex();
            idx.dataReceiver  = (fHits.at(i))->GetBoardReceiver();
            idx.ladderId      = (fHits.at(i))->GetLadderId();
            idx.chipId        = (fHits.at(i))->GetChipId();
            unsigned int dcol = (fHits.at(i))->GetDoubleColumn();
            unsigned int addr = (fHits.at(i))->GetAddress();

            fScanHisto->Incr(idx, dcol, addr);
            
            if ( GetVerboseLevel() > kULTRACHATTY ) {
                cout << "TAlpideDecoder::FillHistoEvent() - add hit" << endl;
                (fHits.at(i))->DumpPixHit();
            }

        }
    }
    fHits.clear();
    return;
}

//___________________________________________________________________
bool TAlpideDecoder::HasData( const common::TChipIndex& idx )
{
    if ( !fScanHisto->IsValidChipIndex( idx ) ) {
        return false;
    }
    for (unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++) {
        for (unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++) {
            if ((*fScanHisto)(idx,icol,iaddr) > 0) return true;
        }
    }
    
    return false;
}

//___________________________________________________________________
bool TAlpideDecoder::IsValidChipIndex( std::shared_ptr<TPixHit> hit )
{
    bool is_valid = false;
    common::TChipIndex idx;
    if ( hit ) {
        idx.boardIndex    = hit->GetBoardIndex();
        idx.dataReceiver  = hit->GetBoardReceiver();
        idx.ladderId      = hit->GetLadderId();
        idx.chipId        = hit->GetChipId();
        is_valid = fScanHisto->IsValidChipIndex( idx );
    }
    return is_valid;
}

//___________________________________________________________________
bool TAlpideDecoder::IsValidChipId()
{
    bool is_valid = false;
    for ( unsigned int i = 0 ; i < fScanHisto->GetChipListSize(); i++ ) {
        int legitimateChipId = (int)((fScanHisto->GetChipIndex(i)).chipId);
        if ( fChipId == legitimateChipId ) {
            is_valid = true;
            return is_valid;
        }
    }
    return is_valid;
}
