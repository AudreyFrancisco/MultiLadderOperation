#include <iostream>
#include "TBoardConfig.h"
#include "TBoardDecoder.h"
#include "MosaicSrc/mboard.h"
#include "MosaicSrc/ipbus.h"
#include "MosaicSrc/TAlpideDataParser.h"

using namespace std;

//___________________________________________________________________
TBoardDecoder::TBoardDecoder() : TVerbosity(),
fBoardType( TBoardType::kBOARD_UNKNOWN ),
fFirmwareVersion( 0x247E0611 ),
fHeaderType( 1 ),
fMOSAIC_channel( -1 ),
fMOSAIC_eoeCount( 0 ),
fMOSAIC_timeout( false ),
fMOSAIC_endOfRun( false ),
fMOSAIC_overflow( false ),
fMOSAIC_headerError( false ),
fMOSAIC_decoder10b8bError( false ),
fDAQ_almostFull( false ),
fDAQ_trigType( -1 ),
fDAQ_bufferDepth( 0 ),
fDAQ_eventId( 0 ),
fDAQ_timestamp( 0 ),
fDAQ_eventSize( 0 ),
fDAQ_strobeCount( 0 ),
fDAQ_trigCountChipBusy( 0 ),
fDAQ_trigCountDAQBusy( 0 ),
fDAQ_extTrigCount( 0 )
{
    
}

//___________________________________________________________________
TBoardDecoder::~TBoardDecoder()
{
    
}

//___________________________________________________________________
void TBoardDecoder::SetBoardType(const TBoardType type,
                                 const uint32_t firmwareVersion,
                                 const int headerType )
{
    fBoardType = type;
    fFirmwareVersion = firmwareVersion;
    fHeaderType = headerType;
    if ( GetVerboseLevel() > kTERSE ) {
        cout << "TBoardDecoder::SetBoardType() - board type = " ;
        switch ( (int)fBoardType ) {
            case (int)TBoardType::kBOARD_DAQ :
                cout << "DAQ board" << endl;
                break;
            case (int)TBoardType::kBOARD_MOSAIC :
                cout << "MOSAIC board" << endl;
                break;
            default:
                cout << "UNKNOWN board !!!" << endl;
                break;
        }
        cout << "TBoardDecoder::SetBoardType() - firmware version = " << fFirmwareVersion << endl;
        cout << "TBoardDecoder::SetBoardType() - header type = " << fHeaderType << endl;
    }
}

//___________________________________________________________________
bool TBoardDecoder::DecodeEvent( unsigned char *data,
                                const int nBytes,
                                int &nBytesHeader,
                                int &nBytesTrailer )
{
    bool success = false;
    switch ( (int)fBoardType ) {
        case (int)TBoardType::kBOARD_DAQ :
            success = DecodeEventDAQ( data, nBytes, nBytesHeader, nBytesTrailer );
            break;
        case (int)TBoardType::kBOARD_MOSAIC :
            success = DecodeEventMOSAIC( data, nBytes, nBytesHeader, nBytesTrailer );
            break;
        default:
            cerr << "TBoardDecoder: UNKNOWN board type !!!" << endl;
            break;
    }
    return success;
 }


// Decodes the Event Header
// The value of nBytes is filled with the length of read header
//___________________________________________________________________
bool TBoardDecoder::DecodeEventMOSAIC( unsigned char *data,
                                      int nBytes,
                                      int &nBytesHeader,
                                      int &nBytesTrailer )
{
    uint32_t blockFlags = MOSAICBoardDecoder::EndianAdjust(data+4);
    
    fMOSAIC_overflow    = blockFlags & MBoard::flagOverflow;
    fMOSAIC_endOfRun    = blockFlags & MBoard::flagCloseRun;
    fMOSAIC_timeout     = blockFlags & MBoard::flagTimeout;
    fMOSAIC_eoeCount    = 1;
    fMOSAIC_channel     = MOSAICBoardDecoder::EndianAdjust(data+12);
    nBytesHeader        = MosaicIPbus::HEADER_SIZE; // #define MOSAIC_HEADER_LENGTH 64
    nBytesTrailer       = 1; // #define The MOSAIC trailer length
    
    uint8_t MOSAICtransmissionFlag = data[nBytes-1];	// last byte is the trailer
    fMOSAIC_headerError = MOSAICtransmissionFlag & TAlpideDataParser::flagHeaderError;
    fMOSAIC_decoder10b8bError = MOSAICtransmissionFlag & TAlpideDataParser::flagDecoder10b8bError;
    
    if ( MOSAICtransmissionFlag ) {
        return false;
    }
    return true;
};

//___________________________________________________________________
bool TBoardDecoder::DecodeEventDAQ( unsigned char *data,
                                  const int nBytes,
                                  int &nBytesHeader,
                                  int &nBytesTrailer )
{

  nBytesHeader = DAQBoardDecoder::GetDAQEventHeaderLength( fFirmwareVersion, fHeaderType );
  nBytesTrailer = DAQBoardDecoder::GetDAQEventTrailerLength();

  // ------ HEADER


  //bool TDAQBoard::DecodeEventHeader  (unsigned char *data_buf, TEventHeader *AHeader) {
  const int header_length = nBytesHeader/4; // length in terms of 32-bit words

  int Header[header_length];
  for (int i = 0; i < header_length; i++) {
      Header[i] = DAQBoardDecoder::GetIntFromBinaryStringReversed(4, data + i*4);
    //#ifdef MYDEBUG
    //        std::cout << "Header word: 0x" << std::hex << Header[i] << std:: dec << std::endl;
    //#endif
  }

  //return DecodeEventHeader(Header, length, AHeader);

  //bool  TDAQBoard::DecodeEventHeader  (int *Header, int length, TEventHeader *AHeader){
  // all header words are supposed to have a zero MSB
  for (int i=0; i<header_length; ++i) {
    if (0x80000000 & Header[i]) {
      std::cout << "Corrupt header data, MSB of header word active!" << std::endl;
      std::cout << std::hex << "0x" << Header[i] << "\t0x" << (0x80000000 & Header[i]) << std::dec << std::endl;
      return false;
    }
  }

  bool    AFull       = false;
  int     TrigType    = -1;
  int     BufferDepth = -1;
  uint64_t Event_ID   = (uint64_t)-1;
  uint64_t TimeStamp  = (uint64_t)-1;
  int StrobeCountTotal  = (header_length>5) ? Header[5] : -1;
  int TrigCountChipBusy = -1;
  int TrigCountDAQbusy  = -1;
  int ExtTrigCounter    = -1;
  if (header_length==3) {
    switch( fFirmwareVersion ) {
      case 0x257E0602:
      case 0x247E0602:
        Event_ID         = (uint64_t)Header[0] & 0x7fffffff;
        TimeStamp        = (uint64_t)Header[1] & 0x7fffffff;
        break;
      case 0x257E0610:
      case 0x247E0610:
      default:
        Event_ID         = (uint64_t)Header[0] & 0x00ffffff;
        //TimeStamp        = (uint64_t)Header[1] & 0x7fffffff | ((uint64_t)Header[0] & 0x7f000000) << 7; // Original
        TimeStamp        = ((uint64_t)Header[1] & 0x7fffffff) | ((uint64_t)Header[0] & 0x7f000000) << 7; // Caterina: added ()
    }
    TrigCountDAQbusy = (Header[2] & 0x7fff0000)>>8;
    StrobeCountTotal = (Header[2] & 0x00007fff);
  }
  else if (header_length==5 || header_length==9) {
    AFull       = (bool) (Header[0] & 0x40);
    TrigType    = (Header[0] & 0x1c00) >> 10;
    BufferDepth = (Header[0] & 0x1e000) >> 13;
    Event_ID   = ((uint64_t) Header[1] & 0xffffff) | ( ((uint64_t) Header[2] & 0xffffff) << 24 );
    TimeStamp  = ((uint64_t) Header[3] & 0xffffff) | ( ((uint64_t) Header[4] & 0xffffff) << 24 );
    StrobeCountTotal  = (header_length>5) ? Header[5] : -1;
    TrigCountChipBusy = (header_length>6) ? Header[6] : -1;
    TrigCountDAQbusy  = (header_length>7) ? Header[7] : -1;
    ExtTrigCounter    = (header_length>8) ? Header[8] : -1;

    // few consistency checks:
    if ((Header[0] & 0xfffe03bf) != 0x8) {
      std::cout << "Corrupt header word 0: 0x" << std::hex << Header[0] << std::dec << std::endl;
      return false;
    }
    if ((Header[1] & 0xff000000) || (Header[2] & 0xff000000) || (Header[3] & 0xff000000)) {
      std::cout << "Corrupt header, missing at least one of the leading 0s in word 1-4" << std::endl;
      return false;
    }
    if ((TrigType < 1) || (TrigType > 2)) {
      std::cout << "Bad Trigger Type " << TrigType << std::endl;
      return false;
    }
  }  
  //#ifdef MYDEBUG
  //    std::cout << "Header: Trigger type = " << TrigType << std::endl;
  //    std::cout << "Header: Almost full  = " << (int) AFull << std::endl;
  //    std::cout << "Header: Event ID     = " << Event_ID << std::endl;
  //    std::cout << "Header: Time stamp   = " << TimeStamp << std::endl;
  //    std::cout << "             in sec  = " << (float)TimeStamp / 8e7 << std::endl;
  //    std::cout << "Header: Total Strobe Count       = " << StrobeCountTotal << std::endl;
  //    std::cout << "Header: Trigger Count Chip Busy  = " << TrigCountChipBusy << std::endl;
  //    std::cout << "Header: Trigger Count DAQ Busy   = " << TrigCountDAQbusy << std::endl;
  //    std::cout << "Header: External Trigger Count   = " << ExtTrigCounter << std::endl;
  //#endif

  fDAQ_almostFull        = AFull;
  fDAQ_bufferDepth       = BufferDepth;
  fDAQ_eventId           = Event_ID;
  fDAQ_timestamp         = TimeStamp;
  fDAQ_trigType          = TrigType;
  fDAQ_strobeCount       = StrobeCountTotal;
  fDAQ_trigCountChipBusy = TrigCountChipBusy;
  fDAQ_trigCountDAQBusy  = TrigCountDAQbusy;
  fDAQ_extTrigCount      = ExtTrigCounter;

  // TRAILER

  //bool TDAQBoard::DecodeEventTrailer (unsigned char *data_buf, TEventHeader *AHeader) {
  int Trailer[2];
  for (int i = 0; i < 2; i++) {
      Trailer[i] = DAQBoardDecoder::GetIntFromBinaryStringReversed(4, (data + nBytes - nBytesTrailer) + i*4);
    //#ifdef MYDEBUG
    //        std::cout << "Trailer word: 0x" << std::hex << Trailer[i] << std:: dec << std::endl;
    //#endif
  }

  //    return DecodeEventTrailer(Trailer, AHeader);
  //bool TDAQBoard::DecodeEventTrailer (int * Trailer, TEventHeader *ATrailer) {
  if (Trailer[1] != (int)DAQ_TRAILER_WORD) {
    std::cout << "Corrupt trailer, expecting 0x " << std::hex << DAQ_TRAILER_WORD << ", found 0x" << Trailer[1] << std::dec << std::endl;
    return false;
  }
  int EventSize = Trailer[0];

  fDAQ_eventSize = EventSize;
  //#ifdef MYDEBUG
  //  std::cout << "Trailer: Event size = " << EventSize << std::endl;
  //  std::cout << std::hex<< "Trailer: 2 word = " << Trailer[1] <<  std::dec << std::endl;
  //#endif

  return true;
}

//___________________________________________________________________
int DAQBoardDecoder::GetDAQEventHeaderLength( const uint32_t firmwareVersion,
                                             const int headerType )
{
  switch(firmwareVersion) {
    case 0x257E030A:
    case 0x247E030A:
    case 0x257E031D:
    case 0x247E031D:
      return 36;
      break;
    case 0:
      return -1;
      break;
    case 0x257E0602:
    case 0x247E0602:
    case 0x257E0610:
    case 0x247E0610:
      return 12;
      break;
    case 0x247E0611:
    case 0x347E0803:
      return ( headerType == 0 ) ? 36 : 12 ;
      break;
    default:
      return 20;
      break;
  }
  return 20;
}

//___________________________________________________________________
uint32_t DAQBoardDecoder::GetIntFromBinaryString(int numByte, unsigned char *str){
  uint32_t number=0;
  int pos=0;
  int exp = numByte -1;
  while (pos < numByte){
    number= number + (uint32_t)(str[pos] << 8*exp);
    exp--;
    pos++;
  }
  return number;
}

//___________________________________________________________________
uint32_t DAQBoardDecoder::GetIntFromBinaryStringReversed(int numByte, unsigned char *str){
  uint32_t number = 0;
  int      pos    = 0;
  while (pos < numByte){
    number= number + (uint32_t)(str[pos] << 8*pos);
    pos++;
  }
  return number;
}

//___________________________________________________________________
uint32_t MOSAICBoardDecoder::EndianAdjust(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
  return (*(uint32_t *) buf) & 0xffffffff;
#else
  uint32_t d;
  d = *buf++;
  d |= (*buf++) << 8;
  d |= (*buf++) << 16;
  d |= (*buf++) << 24;
  return d;
#endif
}






