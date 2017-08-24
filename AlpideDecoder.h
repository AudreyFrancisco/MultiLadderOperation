#ifndef ALPIDEDECODER_H
#define ALPIDEDECODER_H

#include <vector>
#include <memory>

enum class TDataType { kIDLE, kCHIPHEADER, kCHIPTRAILER, kEMPTYFRAME, kREGIONHEADER, kDATASHORT, kDATALONG, kBUSYON, kBUSYOFF, kUNKNOWN };

class TPixHit;

class AlpideDecoder {
    
private:

    static bool fNewEvent;
    
public:
    
    static TDataType GetDataType        ( unsigned char dataWord );
    static int       GetWordLength      ( TDataType dataType );
    static bool      DecodeEvent        ( unsigned char* data, int nBytes,
                                         std::vector<std::shared_ptr<TPixHit>> hits );
private:
    
    static void      DecodeChipHeader   ( unsigned char* data, int& chipId,
                                         unsigned int& bunchCounter );
    static void      DecodeChipTrailer  ( unsigned char* data, int& flags );
    static void      DecodeRegionHeader ( unsigned char* data, int& region);
    static void      DecodeEmptyFrame   ( unsigned char* data, int& chipId,
                                         unsigned int& bunchCounter );
    static void      DecodeDataWord     ( unsigned char* data, int chip, int region,
                                         std::vector<std::shared_ptr<TPixHit>> hits, bool datalong );
};



#endif
