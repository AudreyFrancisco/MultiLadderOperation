#include "TAlpideDecoder.h"
#include "TPixHit.h"
#include "THisto.h"
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
    fPrioErrors( 0 ),
    fDataType( TDataType::kUNKNOWN ),
    fScanHisto( nullptr )
{
    
}

//___________________________________________________________________
TAlpideDecoder::TAlpideDecoder( shared_ptr<TScanHisto> aScanHisto ) : TVerbosity(),
    fNewEvent( false ),
    fBunchCounter( 0 ),
    fFlags( 0 ),
    fChipId( -1 ),
    fRegion( -1 ),
    fBoardIndex( 0 ),
    fBoardReceiver( 0 ),
    fPrioErrors( 0 ),
    fDataType( TDataType::kUNKNOWN ),
    fScanHisto( nullptr )
{
    try {
        SetScanHisto( aScanHisto );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        exit(0);
    }
}


//___________________________________________________________________
TAlpideDecoder::~TAlpideDecoder()
{
    
}

//___________________________________________________________________
void TAlpideDecoder::SetScanHisto( shared_ptr<TScanHisto> aScanHisto )
{
    if ( !aScanHisto ) {
        throw runtime_error( "TDeviceDigitalScan::SetScanHisto() - can not use a null pointer !" );
    }
    fScanHisto = aScanHisto;
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
    
    fScanHisto->FindChipList();
    
    for ( unsigned int ichip = 0; ichip < fScanHisto->GetChipListSize(); ichip++ ) {
        
        common::TChipIndex aChipIndex = fScanHisto->GetChipIndex( ichip );
        
        if ( !HasData( aChipIndex ) ) {
            if ( GetVerboseLevel() > kSILENT ) {
                cout << "TAlpideDecoder::WriteDataToFile() - Chip ID "
                << aChipIndex.chipId << " : no data, skipped." <<  endl;
            }
            continue;  // write files only for chips with data
        }
        string filename = common::GetFileName( aChipIndex, suffix );
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TAlpideDecoder::WriteDataToFile() - Chip ID = "<< aChipIndex.chipId << endl;
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

    fScanHisto->FindChipList();

    for ( unsigned int ichip = 0; ichip < fScanHisto->GetChipListSize(); ichip++ ) {
        
        common::TChipIndex aChipIndex = fScanHisto->GetChipIndex( ichip );
        nHits += fScanHisto->GetChipNEntries( aChipIndex );
    }
    return nHits;
}

//___________________________________________________________________
void TAlpideDecoder::DumpCorruptedHits()
{
    if( fCorruptedHits.size() ) {
        cout << "------------------------------- TAlpideDecoder::DumpCorruptedHits() "
             << endl;
        cout << "board.receiver / chip / dcol.addr (flag) " << endl;
    }
    for ( unsigned int i = 0; i < fCorruptedHits.size(); i++ ) {
        cout << (fCorruptedHits.at(i))->GetBoardIndex() << "."
             << (fCorruptedHits.at(i))->GetBoardReceiver() << " / "
             << (fCorruptedHits.at(i))->GetChipId() << " / "
             << (fCorruptedHits.at(i))->GetDoubleColumn() << "."
             << (fCorruptedHits.at(i))->GetAddress() << " ("
             << (int)((fCorruptedHits.at(i))->GetPixFlag()) << ")" << endl;
    }
    if( fCorruptedHits.size() ) {
        cout << "------------------------------- " << endl;
    }
}

//___________________________________________________________________
bool TAlpideDecoder::DecodeEvent( unsigned char* data,
                                 int nBytes,
                                 unsigned int boardIndex,
                                 unsigned int boardReceiver )
{
    // refresh variables for each new event
    fBunchCounter = 0;
    fFlags  = 0;
    fChipId = -1;
    fRegion = -1;
    fBoardIndex = boardIndex;
    fBoardReceiver = boardReceiver;
    fPrioErrors = 0;
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
                    if ( fChipId == -1 ) {
                        cerr << "TAlpideDecoder::DecodeEvent() - Warning: found chip id -1, TDataType::kDATASHORT" << endl;
                        for ( int i = 0; i < nBytes; i++ ) {
                            printf("%02x ", data[i]);
                        }
                        printf("\n");
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
                    if ( fChipId == -1 ) {
                        cerr << "TAlpideDecoder::DecodeEvent() - Warning: found chip id -1, TDataType::kDATALONG" << endl;
                        for ( int i = 0; i < nBytes; i++ ) {
                            printf("%02x ", data[i]);
                        }
                        printf("\n");
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
    hit->SetRegion( fRegion );
    hit->SetDoubleColumn( (data_field & 0x3c00) >> 10 );

    if ( fChipId == -1 ) {
        cout << "TAlpideDecoder::DecodeDataWord() - Warning: found chip id -1, data word = 0x" << std::hex << (int) data_field << std::dec << endl;
        // store one hit with the flag indicating a bad chip id
        hit->SetAddress( 0 );
        hit->SetChipId( 0 );
        hit->SetPixFlag( TPixFlag::kBAD_CHIPID );
        fNewEvent = false;
        corrupt = true;
        return corrupt;
    }
    hit->SetChipId( fChipId );
    
    unsigned int address = (data_field & 0x03ff);

    int hitmap_length;
    
    if ( datalong ) {
        hitmap_length = 7; // clustering enabled
    } else {
        hitmap_length = 0;
    }

    for ( int i = -1; i < hitmap_length; i ++ ) {
        
        if ((i >= 0) && (! (data[2] >> i) & 0x1)) continue;
        shared_ptr<TPixHit> singleHit( new TPixHit( hit ) ); // deep copy
        singleHit->SetAddress( address + (i + 1) ); // set hit address on the new shared ptr, can generate a bad address flag
        
        if ( (fHits.size() > 0) && (!fNewEvent) ) {
            if ( (singleHit->GetRegion() == (fHits.back())->GetRegion())
                && ( singleHit->GetDoubleColumn() == (fHits.back())->GetDoubleColumn())
                && (singleHit->GetAddress() == (fHits.back())->GetAddress()) ) {
                cout << "TAlpideDecoder::DecodeDataWord() - Warning : received pixel twice." << endl;
                fPrioErrors++;
                singleHit->SetPixFlag( TPixFlag::kSTUCK );
                (fHits.back())->SetPixFlag( TPixFlag::kSTUCK );
                cout << "\t -- current hit pixel :" << endl;
                singleHit->DumpPixHit();
                cout << "\t -- previous hit pixel :" << endl;
                (fHits.back())->DumpPixHit();
            }
            else if ( (singleHit->GetRegion() == (fHits.back())->GetRegion() )
                     && (singleHit->GetDoubleColumn() == (fHits.back())->GetDoubleColumn())
                     && (singleHit->GetAddress() < (fHits.back())->GetAddress()) ) {
                cout << "TAlpideDecoder::DecodeDataWord() - Warning : address of pixel is lower than previous one in same double column." << endl;
                fPrioErrors++;
                singleHit->SetPixFlag( TPixFlag::kSTUCK );
                (fHits.back())->SetPixFlag( TPixFlag::kSTUCK );
                cout << "\t -- current hit pixel :" << endl;
                singleHit->DumpPixHit();
                cout << "\t -- previous hit pixel :" << endl;
                (fHits.back())->DumpPixHit();
            }
        }
        
        if ( singleHit->GetPixFlag() == TPixFlag::kUNKNOWN ) { // nothing bad detected, the flag still has its initialization value => the pixel is ok
            singleHit->SetPixFlag( TPixFlag::kOK );
        }
        if ( GetVerboseLevel() > kCHATTY ) {
            cout << "TAlpideDecoder::DecodeDataWord() - new hit found" << endl;
            singleHit->DumpPixHit();
        }
        corrupt = singleHit->IsPixHitCorrupted();
        fHits.push_back( move(singleHit) ); // vector owns hit with the address set
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
            fCorruptedHits.push_back( move(badHit) );
            
        } else {
            
            idx.boardIndex    = (fHits.at(i))->GetBoardIndex();
            idx.dataReceiver  = (fHits.at(i))->GetBoardReceiver();
            idx.chipId        = (fHits.at(i))->GetChipId();
            unsigned int dcol = (fHits.at(i))->GetDoubleColumn();
            unsigned int addr = (fHits.at(i))->GetAddress();
            if ( GetVerboseLevel() > kULTRACHATTY ) {
                cout << "TAlpideDecoder::FillHistoEvent() - add hit" << endl;
                (fHits.at(i))->DumpPixHit();
            }
            
            fScanHisto->Incr(idx, dcol, addr);

        }
    }
    fHits.clear();
    return;
}

//___________________________________________________________________
bool TAlpideDecoder::HasData( const common::TChipIndex& idx )
{
    for (unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++) {
        for (unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++) {
            if ((*fScanHisto)(idx,icol,iaddr) > 0) return true;
        }
    }
    
    return false;
}



