#ifndef TALPIDEDECODER_H
#define TALPIDEDECODER_H

#include <vector>
#include <memory>
#include "TVerbosity.h"

enum class TDataType {
    kIDLE,
    kCHIPHEADER,
    kCHIPTRAILER,
    kEMPTYFRAME,
    kREGIONHEADER,
    kDATASHORT,
    kDATALONG,
    kBUSYON,
    kBUSYOFF,
    kUNKNOWN
};

class TPixHit;

class TAlpideDecoder : public TVerbosity {
    
private:

    bool fNewEvent;
    unsigned int fBunchCounter;
    int fFlags;
    int fChipId;
    int fRegion;
    int fBoardIndex;
    int fBoardReceiver;
    int fPrioErrors;
    TDataType fDataType;
    
public:

    // constructor / destructor
    
    TAlpideDecoder();
    ~TAlpideDecoder();

    // the sole purpose of this class : decode each event read by the readout board

    bool DecodeEvent( unsigned char* data, int nBytes,
                     std::vector<std::shared_ptr<TPixHit>> hits,
                     int boardIndex,
                     int boardReceiver );
    
    // getters
    
    inline int GetPrioErrors() const { return fPrioErrors; }

private:
    
    void SetDataType( unsigned char dataWord );
    int GetWordLength() const;
    void DecodeChipHeader( unsigned char* data );
    void DecodeChipTrailer( unsigned char* data );
    void DecodeRegionHeader( unsigned char* data );
    void DecodeEmptyFrame( unsigned char* data );
    bool DecodeDataWord( unsigned char* data,
                        std::vector<std::shared_ptr<TPixHit>> hits,
                        bool datalong );
};

#endif
