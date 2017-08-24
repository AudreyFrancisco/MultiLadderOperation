#include "AlpideDecoder.h"
#include "TPixHit.h"
#include <stdint.h>
#include <iostream>
#include <bitset>

using namespace std;

bool AlpideDecoder::fNewEvent = false;

//___________________________________________________________________
TDataType AlpideDecoder::GetDataType( unsigned char dataWord )
{
    cout << "AlpideDecoder::GetDataType() - dataWord = " << endl;
    printf ("%02x ", (int)dataWord);
    cout << endl;
    if      ( dataWord == 0xff )          return TDataType::kIDLE;
    else if ( dataWord == 0xf1 )          return TDataType::kBUSYON;
    else if ( dataWord == 0xf0 )          return TDataType::kBUSYOFF;
    else if ( (dataWord & 0xf0) == 0xa0 ) return TDataType::kCHIPHEADER;
    else if ( (dataWord & 0xf0) == 0xb0 ) return TDataType::kCHIPTRAILER;
    else if ( (dataWord & 0xf0) == 0xe0 ) return TDataType::kEMPTYFRAME;
    else if ( (dataWord & 0xe0) == 0xc0 ) return TDataType::kREGIONHEADER;
    else if ( (dataWord & 0xc0) == 0x40 ) return TDataType::kDATASHORT;
    else if ( (dataWord & 0xc0) == 0x0 )  return TDataType::kDATALONG;
    else return TDataType::kUNKNOWN;
}

//___________________________________________________________________
int AlpideDecoder::GetWordLength( TDataType dataType )
{
    if ( dataType == TDataType::kDATALONG ) {
    return 3;
  }
    else if ( (dataType == TDataType::kDATASHORT) || (dataType == TDataType::kCHIPHEADER) || (dataType == TDataType::kEMPTYFRAME) ) {
    return 2;
  }
  else return 1;
}

//___________________________________________________________________
bool AlpideDecoder::DecodeEvent( unsigned char* data,
                                int nBytes,
                                vector<shared_ptr<TPixHit>> hits )
{
    int       byte    = 0;
    int       region  = -1;
    int       chip    = -1;
    int       flags   = 0;
    bool      started = false; // event has started, i.e. chip header has been found
    bool      finished = false; // event trailer found
    TDataType type;
    
    unsigned char last;
    
    unsigned int BunchCounterTmp;
    
    while ( byte < nBytes ) {
        
        last = data[byte];
        type = GetDataType( data[byte] );
        
        switch ( type ) {
            case TDataType::kIDLE:
                byte +=1;
                break;
            case TDataType::kBUSYON:
                byte += 1;
                break;
            case TDataType::kBUSYOFF:
                byte += 1;
                break;
            case TDataType::kEMPTYFRAME:
                started = true;
                DecodeEmptyFrame( data + byte, chip, BunchCounterTmp );
                cout << "AlpideDecoder::DecodeEvent() - empty frame" << endl;
                byte += 2;
                finished = true;
                break;
            case TDataType::kCHIPHEADER:
                started = true;
                finished = false;
                DecodeChipHeader( data + byte, chip, BunchCounterTmp );
                cout << "AlpideDecoder::DecodeEvent() - chip header" << endl;
                byte += 2;
                break;
            case TDataType::kCHIPTRAILER:
                if ( !started ) {
                    cerr << "AlpideDecoder::DecodeEvent() - Error, chip trailer found before chip header" << endl;
                    return false;
                }
                if ( finished ) {
                    cerr << "AlpideDecoder::DecodeEvent() - Error, chip trailer found after event was finished" << endl;
                    return false;
                }
                cout << "AlpideDecoder::DecodeEvent() - chip trailer" << endl;
                DecodeChipTrailer( data + byte, flags );
                finished = true;
                chip = -1;
                byte += 1;
                break;
            case TDataType::kREGIONHEADER:
                if (!started) {
                    cerr << "AlpideDecoder::DecodeEvent() - Error, region header found before chip header or after chip trailer" << endl;
                    return false;
                }
                cout << "AlpideDecoder::DecodeEvent() - region header" << endl;
                DecodeRegionHeader( data + byte, region );
                byte +=1;
                break;
            case TDataType::kDATASHORT:
                if ( !started ) {
                    cerr << "AlpideDecoder::DecodeEvent() - Error, hit data found before chip header or after chip trailer" << endl;
                    return false;
                }
                if ( region == -1 ) {
                    cout << "AlpideDecoder::DecodeEvent() - Warning: data word without region, skipping (Chip " << chip << ")" << endl;
                }
                else if ( hits.data() ) {
                    if ( chip == -1 ) {
                        cerr << "AlpideDecoder::DecodeEvent() - TDataType::kDATASHORT" << endl;
                        for ( int i = 0; i < nBytes; i++ ) {
                            printf("%02x ", data[i]);
                        }
                        printf("\n");
                    }
                    DecodeDataWord( data + byte, chip, region, hits, false );
                }
                byte += 2;
                break;
            case TDataType::kDATALONG:
                if ( !started ) {
                    cerr << "AlpideDecoder::DecodeEvent() - Error, hit data found before chip header or after chip trailer" << endl;
                    return false;
                }
                if ( region == -1 ) {
                    cerr << "AlpideDecoder::DecodeEvent() - Warning: data word without region, skipping (Chip " << chip << ")" << endl;
                }
                else if ( hits.data() ) {
                    if ( chip == -1 ) {
                        cerr << "AlpideDecoder::DecodeEvent() - TDataType::kDATALONG" << endl;
                        for ( int i = 0; i < nBytes; i++ ) {
                            printf("%02x ", data[i]);
                        }
                        printf("\n");
                    }
                    DecodeDataWord( data + byte, chip, region, hits, true );
                }
                byte += 3;
                break;
            case TDataType::kUNKNOWN:
                cerr << "AlpideDecoder::DecodeEvent() - Error, data of unknown type 0x" << std::hex << data[byte] << std::dec << endl;
                return false;
        }
    }
    //cout << "Found " << Hits->size() - NOldHits << " hits" << endl;
    if ( started && finished ) return true;
    else {
        if ( started && !finished ) {
            cout << "AlpideDecoder::DecodeEvent() - Warning (chip "<< chip << "), event not finished at end of data, last byte was 0x" << std::hex << (int) last << std::dec << ", event length = " << nBytes << endl;
            return false;
        }
        if ( !started ) {
            cout << "AlpideDecoder::DecodeEvent() - Warning, event not started at end of data" << endl;
            return false;
        }
    }
    return true;
}

//___________________________________________________________________
void AlpideDecoder::DecodeChipHeader( unsigned char* data,
                                      int& chipId,
                                      unsigned int& bunchCounter )
{
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  bunchCounter = data_field & 0xff;
  chipId       = (data_field >> 8) & 0xf;
  fNewEvent    = true;
}

//___________________________________________________________________
void AlpideDecoder::DecodeChipTrailer( unsigned char* data, int& flags )
{
  flags = data[0] & 0xf;
}

//___________________________________________________________________
void AlpideDecoder::DecodeRegionHeader( unsigned char* data, int& region )
{
  region = data[0] & 0x1f;
}

//___________________________________________________________________
void AlpideDecoder::DecodeEmptyFrame ( unsigned char* data,
                                      int& chipId,
                                      unsigned int& bunchCounter )
{
  int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

  bunchCounter = data_field & 0xff;
  chipId       = (data_field >> 8) & 0xf;
}

//___________________________________________________________________
void AlpideDecoder::DecodeDataWord( unsigned char* data,
                                   int chip,
                                   int region,
                                   vector<shared_ptr<TPixHit>> hits,
                                   bool datalong )
{
    cout << "AlpideDecoder::DecodeDataWord() - start" << endl;

    auto hit = make_shared<TPixHit>();
    
    unsigned int address;
    int hitmap_length;

    int16_t data_field = (((int16_t) data[0]) << 8) + data[1];

    if ( chip == -1 ) {
        cout << "AlpideDecoder::DecodeDataWord() - Warning, found chip id -1, dataword = 0x" << std::hex << (int) data_field << std::dec << endl;
    }
    hit->SetChipId( chip );
    hit->SetRegion( region );
    hit->SetDoubleColumn( (data_field & 0x3c00) >> 10 );
    address = (data_field & 0x03ff);

    if ( (hits.size() > 0) && (!fNewEvent) ) {
        if ( (hit->GetRegion() == (hits.back())->GetRegion())
            && ( hit->GetDoubleColumn() == (hits.back())->GetDoubleColumn())
            && (address == (hits.back())->GetAddress()) ) {
            cout << "AlpideDecoder::DecodeDataWord() - Warning (chip "<< chip << "), received pixel " << hit->GetRegion() << "/" << hit->GetDoubleColumn()
                << "/" << address <<  " twice." << endl;
        }
        else if ( (hit->GetRegion() == (hits.back())->GetRegion() )
                 && (hit->GetDoubleColumn() == (hits.back())->GetDoubleColumn())
                 && (address < (hits.back())->GetAddress()) ) {
            cout << "AlpideDecoder::DecodeDataWord() - Warning (chip "<< chip << "), address of pixel " << hit->GetRegion() << "/" << hit->GetDoubleColumn() << "/" << address <<  " is lower than previous one ("
                << (hits.back())->GetAddress()
                << ") in same double column." << endl;
        }
    }

    if ( datalong ) {
        hitmap_length = 7;
    } else {
        hitmap_length = 0;
    }

    for ( int i = -1; i < hitmap_length; i ++ ) {
        if ((i >= 0) && (! (data[2] >> i) & 0x1)) continue;
        hit->SetAddress( address + (i + 1) );
        /*
        if ( hit->GetChipId() == -1 ) {
            cout << "AlpideDecoder::DecodeDataWord() - Warning, found chip id -1"
                 << endl;
        }
         */
        hits.push_back( move(hit) );
        cout << "AlpideDecoder::DecodeDataWord() - hit added." << endl;
    }
    fNewEvent = false;
}
