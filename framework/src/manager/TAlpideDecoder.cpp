#include "TAlpideDecoder.h"
#include "TPixHit.h"
#include <stdint.h>
#include <iostream>
#include <bitset>

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
    fDataType( TDataType::kUNKNOWN )
{
    
}

//___________________________________________________________________
TAlpideDecoder::~TAlpideDecoder()
{
    
}

//___________________________________________________________________
void TAlpideDecoder::SetDataType( unsigned char dataWord )
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
        
        cout << "TAlpideDecoder::SetDataType() - dataWord = " << endl;
        printf ("%02x ", (int)dataWord);
        cout << endl << "TAlpideDecoder::SetDataType() - fDataType = " ;
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
bool TAlpideDecoder::DecodeEvent( unsigned char* data,
                                int nBytes,
                                vector<shared_ptr<TPixHit>> hits,
                                 int boardIndex,
                                 int boardReceiver )
{
    fBunchCounter = 0;
    fFlags  = 0;
    fChipId = -1;
    fRegion = -1;
    fBoardIndex = boardIndex;
    fBoardReceiver = boardReceiver;
    fDataType = TDataType::kUNKNOWN;
    bool started = false; // event has started, i.e. chip header has been found
    bool finished = false; // event trailer found
    bool corrupt  = false; // corrupt data found (i.e. data without region or chip)
    int byte = 0;
    
    unsigned char last;
    
    while ( byte < nBytes ) {
        
        last = data[byte];
        SetDataType( data[byte] );
        
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
                }
                else if ( hits.data() ) {
                    if ( fChipId == -1 ) {
                        cerr << "TAlpideDecoder::DecodeEvent() - Warning: found chip id -1, TDataType::kDATASHORT" << endl;
                        for ( int i = 0; i < nBytes; i++ ) {
                            printf("%02x ", data[i]);
                        }
                        printf("\n");
                    }
                    bool corrupted = DecodeDataWord( data + byte, hits, false );
                    if (corrupted) {
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
                }
                else if ( hits.data() ) {
                    if ( fChipId == -1 ) {
                        cerr << "TAlpideDecoder::DecodeEvent() - Warning: found chip id -1, TDataType::kDATALONG" << endl;
                        for ( int i = 0; i < nBytes; i++ ) {
                            printf("%02x ", data[i]);
                        }
                        printf("\n");
                    }
                    bool corrupted = DecodeDataWord( data + byte, hits, true );
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
    if ( started && finished ) return (!corrupt);
    else {
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
}

//___________________________________________________________________
void TAlpideDecoder::DecodeEmptyFrame ( unsigned char* data )
{
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  fBunchCounter = data_field & 0xff;
  fChipId       = (data_field >> 8) & 0xf;
}

//___________________________________________________________________
bool TAlpideDecoder::DecodeDataWord( unsigned char* data,
                                    vector<shared_ptr<TPixHit>> hits,
                                    bool datalong )
{
    auto hit = make_shared<TPixHit>();
    
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

    if ( (hits.size() > 0) && (!fNewEvent) ) {
        if ( (hit->GetRegion() == (hits.back())->GetRegion())
            && ( hit->GetDoubleColumn() == (hits.back())->GetDoubleColumn())
            && (address == (hits.back())->GetAddress()) ) {
            cout << "TAlpideDecoder::DecodeDataWord() - Warning (chip "<< fChipId << "): received pixel " << hit->GetRegion() << "/" << hit->GetDoubleColumn()
                << "/" << address <<  " twice." << endl;
            fPrioErrors++;
            hit->SetPixFlag( TPixFlag::kSTUCK );
        }
        else if ( (hit->GetRegion() == (hits.back())->GetRegion() )
                 && (hit->GetDoubleColumn() == (hits.back())->GetDoubleColumn())
                 && (address < (hits.back())->GetAddress()) ) {
            cout << "TAlpideDecoder::DecodeDataWord() - Warning (chip "<< fChipId << "): address of pixel " << hit->GetRegion() << "/" << hit->GetDoubleColumn() << "/" << address <<  " is lower than previous one ("
                << (hits.back())->GetAddress()
                << ") in same double column." << endl;
            fPrioErrors++;
            hit->SetPixFlag( TPixFlag::kSTUCK );
        }
    }

    int hitmap_length;
    
    if ( datalong ) {
        hitmap_length = 7; // clustering enabled
    } else {
        hitmap_length = 0;
    }

    TPixFlag pixFlag = hit->GetPixFlag();
    for ( int i = -1; i < hitmap_length; i ++ ) {
        if ((i >= 0) && (! (data[2] >> i) & 0x1)) continue;
        auto singleHit( hit ); // copy constructor to make a new shared ptr
        singleHit->SetAddress( address + (i + 1) ); // set hit address on the new shared ptr
        if ( pixFlag == TPixFlag::kSTUCK) { // keep stuck flag, higher priority than bad address flag
            singleHit->SetPixFlag( TPixFlag::kSTUCK );
        }
        pixFlag = singleHit->GetPixFlag();
        if ( GetVerboseLevel() > kCHATTY ) {
            cout << "TAlpideDecoder::DecodeDataWord() - new hit :" << endl;
            cout << "\t board:receiver:chip:region:dcol:add , flag "
                << singleHit->GetBoardIndex() << ":"
                << singleHit->GetBoardReceiver() << ":"
                << singleHit->GetChipId() << ":"
                << singleHit->GetRegion() << ":"
                << singleHit->GetDoubleColumn() << ":"
                << singleHit->GetAddress() << " , " << (int)pixFlag << endl;
        }
        corrupt = ( (pixFlag == TPixFlag::kBAD_ADDRESS)
                   || (pixFlag == TPixFlag::kBAD_DCOLID)
                   || (pixFlag == TPixFlag::kBAD_REGIONID)
                   || (pixFlag == TPixFlag::kBAD_CHIPID)
                   || (pixFlag == TPixFlag::kUNKNOWN) ) ? true : false;

        hits.push_back( move(singleHit) ); // vector owns hit with the address set
        if ( GetVerboseLevel() > kCHATTY ) {
            cout << "TAlpideDecoder::DecodeDataWord() - hit added." << endl;
        }
    }
    fNewEvent = false;
    return corrupt;
}
