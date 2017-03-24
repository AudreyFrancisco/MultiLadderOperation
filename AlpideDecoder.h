#ifndef ALPIDEDECODER_H
#define ALPIDEDECODER_H

#include <vector>
#include <cstdint>

enum TDataType {DT_IDLE, DT_CHIPHEADER, DT_CHIPTRAILER, DT_EMPTYFRAME, DT_REGIONHEADER, DT_DATASHORT, DT_DATALONG, DT_BUSYON, DT_BUSYOFF, DT_UNKNOWN};

typedef struct {
  int channel;
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
   static void      DecodeDataWord     (unsigned char *data, int chip, int region, std::vector <TPixHit> *hits, bool datalong, int channel, int &prioErrors);
 protected:
 public:
   static TDataType GetDataType        (unsigned char dataWord);
   static int       GetWordLength      (TDataType dataType);
   static bool      DecodeEvent        (unsigned char *data, int nBytes, std::vector <TPixHit> *hits, int channel, int &prioErrors);
   static bool      ExtractNextEvent    (unsigned char *data, int nBytes, int &eventStart, int &eventEnd, bool& isError, bool logging=false);
};



#endif
