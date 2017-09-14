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
    
    /// number of errors from the priority encoders (double column) for the current event
    unsigned int fPrioErrors;
    
    /// type of the data word currently being decoded
    TDataType fDataType;

    /// hit pixel list with all decoded hits for the current event
    std::vector<std::shared_ptr<TPixHit>> fHits;

    /// corrupted hit pixel list
    std::vector<std::shared_ptr<TPixHit>> fCorruptedHits;

    /// map to histograms (one per chip) of hit pixels, accumulating over events
    std::shared_ptr<TScanHisto> fScanHisto;

    
public:

    /// default constructor
    TAlpideDecoder();
    
    /// constructor that set the pointer to the map to histograms of hit pixels
    TAlpideDecoder( std::shared_ptr<TScanHisto> aScanHisto );
    
    /// destructor
    ~TAlpideDecoder();

    /// set the pointer to the map containing histograms of hit pixels vs chip index
    void SetScanHisto( std::shared_ptr<TScanHisto> aScanHisto );
    
    /// get the number of priority encoder errors for the current event
    inline int GetPrioErrors() const { return fPrioErrors; }
    
    /// get the map of histograms (one per chip) of hit pixels
    inline std::shared_ptr<TScanHisto> GetScanHisto() { return fScanHisto; }
    
    /// write hit data to a text file
    void WriteDataToFile( const char *fName, bool Recreate = true );
    
    /// compute the total number of hits over the full extent of the digital scan
    unsigned int GetNHits() const;
    
    /// dump the list of corrupted hit pixels over the full extent of the digital scan
    void DumpCorruptedHits();

    /// the sole purpose of this class : decode each event read by the readout board
    bool DecodeEvent( unsigned char* data, int nBytes,
                     unsigned int boardIndex,
                     unsigned int boardReceiver );
    
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
};

#endif
