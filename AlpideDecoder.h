#ifndef ALPIDEDECODER_H
#define ALPIDEDECODER_H

#include <vector>

enum class TDataType {kIDLE, kCHIPHEADER, kCHIPTRAILER, kEMPTYFRAME, kREGIONHEADER, kDATASHORT, kDATALONG, kBUSYON, kBUSYOFF, kUNKNOWN};

typedef struct TPixHit {
  int chipId;
  int region; 
  int dcol;
  int address;
} TPixHit;

class AlpideDecoder {
 private:
   static void      DecodeChipHeader   (unsigned char *data, int &chipId, unsigned int &bunchCounter);
   static void      DecodeChipTrailer  (unsigned char *data, int &flags);
   static void      DecodeRegionHeader (unsigned char *data, int &region);
   static void      DecodeEmptyFrame   (unsigned char *data, int &chipId, unsigned int &bunchCounter);
   static void      DecodeDataWord     (unsigned char *data, int chip, int region, std::vector <TPixHit> *hits, bool datalong);
 protected:
 public:
   static TDataType GetDataType        (unsigned char dataWord);
   static int       GetWordLength      (TDataType dataType);
   static bool      DecodeEvent        (unsigned char *data, int nBytes, std::vector <TPixHit> *hits);
};



#endif
