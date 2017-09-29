#ifndef TALPIDEDECODER_H
#define TALPIDEDECODER_H

#include <vector>
#include <memory>
#include "TVerbosity.h"
#include "Common.h"

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
class TScanHisto;
class TErrorCounter;

class TAlpideDecoder : public TVerbosity {
    
private:

    /// allows the decoder to know if the current data corresponds to a new event
    bool fNewEvent;
    
    /// bunch counter decoded from the chip header
    unsigned int fBunchCounter;

    /// flag decoded from chip trailer
    int fFlags;

    /// chip id decoded from chip header
    int fChipId;
    
    /// region id decoded from region header
    int fRegion;
    
    /// id of the board that read the chip (must be given by the user)
    unsigned int fBoardIndex;
    
    /// id of the board data receiver that read the chip (must be given by the user)
    unsigned int fBoardReceiver;
    
    /// id of the ladder to which belong the chip
    unsigned int fLadderId;
    
    /// type of the data word currently being decoded
    TDataType fDataType;

    /// hit pixel list with all decoded hits for the current event
    std::vector<std::shared_ptr<TPixHit>> fHits;

    /// map to histograms (one per chip) of hit pixels, accumulating over events
    std::shared_ptr<TScanHisto> fScanHisto;
    
    /// error counter, accumulating over events
    std::shared_ptr<TErrorCounter> fErrorCounter;

    
public:

    /// default constructor
    TAlpideDecoder();
    
    /// constructor that sets pointers to map of hit pixel histograms and to error counter
    TAlpideDecoder( std::shared_ptr<TScanHisto> aScanHisto,
                    std::shared_ptr<TErrorCounter> anErrorCounter );
    
    /// destructor
    ~TAlpideDecoder();

    /// set the pointer to the map containing histograms of hit pixels vs chip index
    void SetScanHisto( std::shared_ptr<TScanHisto> aScanHisto );

    /// set the pointer to the error container
    void SetErrorCounter( std::shared_ptr<TErrorCounter> anErrorCounter );
    
    /// get the map of histograms (one per chip) of hit pixels
    inline std::shared_ptr<TScanHisto> GetScanHisto() { return fScanHisto; }
    
    /// write hit data to a text file
    void WriteDataToFile( const char *fName, bool Recreate = true );
    
    /// compute the total number of hits over the full extent of the scan
    unsigned int GetNHits() const;
    
    /// the sole purpose of this class : decode each event read by the readout board
    bool DecodeEvent( unsigned char* data, int nBytes,
                     unsigned int boardIndex,
                     unsigned int boardReceiver,
                     unsigned int ladderId );
        
private:
    
    /// find the data type of the given data word
    void FindDataType( unsigned char dataWord );
    
    /// return the length of the data word depending on its data type
    int GetWordLength() const;
    
    /// extract the bunch counter and chip id from a data word of type "chip header"
    void DecodeChipHeader( unsigned char* data );
    
    /// extract the flag from a data word of type "chip trailer"
    void DecodeChipTrailer( unsigned char* data );
    
    /// extract the region id from a data word of type "region header"
    void DecodeRegionHeader( unsigned char* data );
    
    /// extract the bunch counter and chip id from a data word of type "empty frame"
    void DecodeEmptyFrame( unsigned char* data );
    
    /// extract hit pixels from a data word of type "data short" or "data long"
    bool DecodeDataWord( unsigned char* data,
                        bool datalong );
    
    /// fill the histogram with good quality pixel hits extracted from the current event
    void FillHistoWithEvent();
    
    /// check if there is any hit for the requested chip index
    bool HasData( const common::TChipIndex& idx );
    
    /// check if the hit would have a legitimate chip index
    bool IsValidChipIndex( std::shared_ptr<TPixHit> hit );
    
    /// check if the current chip id is legitimate
    bool IsValidChipId();

};

#endif
