#ifndef BOARDDECODER_H
#define BOARDDECODER_H 

#include <memory>
#include <cstdint>
#include <string>
#include "TVerbosity.h"

enum class TBoardType;

class TBoardDecoder : public TVerbosity {
    
    TBoardType fBoardType;
    
    // firmware version and header type are only used for the DAQ board (so far)
    std::uint32_t fDAQ_firmwareVersion;
    int fDAQ_headerType;
    
    // firmware version for MOSAIC board
    std::string fMOSAIC_firmwareVersion;
    
    // put all header and trailer information here
    // (both for mosaic and DAQ board)
    
    // MOSAIC
    int  fMOSAIC_channel;
    int  fMOSAIC_eoeCount;
    bool fMOSAIC_timeout;
    bool fMOSAIC_endOfRun;
    bool fMOSAIC_overflow;
    bool fMOSAIC_headerError;  // the received Frame contains error in the transmission
    bool fMOSAIC_decoder10b8bError; // the MOSAIC board reports a 10b8b conversion error
    
    // DAQ board
    bool            fDAQ_almostFull;
    int             fDAQ_trigType;
    int             fDAQ_bufferDepth;
    std::uint64_t   fDAQ_eventId;
    std::uint64_t   fDAQ_timestamp;
    int             fDAQ_eventSize;
    int             fDAQ_strobeCount;
    int             fDAQ_trigCountChipBusy;
    int             fDAQ_trigCountDAQBusy;
    int             fDAQ_extTrigCount;

public:
    
    // constructor / destructor

    TBoardDecoder();
    ~TBoardDecoder();
    
    // setter

    void SetBoardType( const TBoardType type );
    void SetFirmwareVersion( const std::uint32_t DAQfirmwareVersion,
                            const int DAQheaderType );
    void SetFirmwareVersion( std::string MOSAICfirmwareVersion );

    // getters
    
    inline TBoardType GetBoardType() const { return fBoardType; }

    inline std::string GetMosaicFirmwareVersion() const { return fMOSAIC_firmwareVersion; }
    inline int GetMosaicChannel() const { return fMOSAIC_channel; }
    inline int GetMosaicEoeCount() const { return fMOSAIC_eoeCount; }
    inline bool GetMosaicTimeout() const { return fMOSAIC_timeout; }
    inline bool GetMosaicEnfOfRun() const { return fMOSAIC_endOfRun; }
    inline bool GetMosaicOverflow() const { return fMOSAIC_overflow; }
    inline bool GetMosaicHeaderError() const { return fMOSAIC_headerError; }
    inline bool GetMosaicDecoder10b8bError() const { return fMOSAIC_decoder10b8bError; }
    
    inline std::uint32_t GetDaqFirmwareVersion() const { return fDAQ_firmwareVersion; }
    inline int GetDaqHeaderType() const { return fDAQ_headerType; }
    inline bool GetDaqAlmostFull() const { return fDAQ_almostFull; }
    inline int GetDaqTrigType() const { return fDAQ_trigType; }
    inline int GetDaqBufferDepth() const { return fDAQ_bufferDepth; }
    inline std::uint64_t GetDaqEventId() const { return fDAQ_eventId; }
    inline std::uint64_t GetDaqTimestamp() const { return fDAQ_timestamp; }
    inline int GetDaqEventSize() const { return fDAQ_eventSize; }
    inline int GetDaqStrobeCount() const { return fDAQ_strobeCount; }
    inline int GetDaqTrigCountChipBusy() const { return fDAQ_trigCountChipBusy; }
    inline int GetDaqTrigCountDAQBusy() const { return fDAQ_trigCountDAQBusy; }
    inline int GetDaqExtTrigCount() const { return fDAQ_extTrigCount; }
    
    // the decoder function
    
    bool DecodeEvent( unsigned char *data,
                     const int nBytes,
                     int &nBytesHeader, // length in bytes
                     int &nBytesTrailer ); // length in bytes
    
private:
    
    // Decodes the Event Header for a MOSAIC board
    bool DecodeEventMOSAIC( unsigned char *data,
                           const int nBytes,
                           int &nBytesHeader,
                           int &nBytesTrailer );
    
    // Decodes the Event Header for a DAQ board
    bool DecodeEventDAQ( unsigned char *data,
                        const int nBytes, // The value of nBytes is filled with the length of read header
                        int &nBytesHeader,
                        int &nBytesTrailer );

    
};

namespace DAQBoardDecoder {
    
    int GetDAQEventHeaderLength( const std::uint32_t firmwareVersion = 0x247E0611,
                                const int headerType = 1 );
    
    inline int GetDAQEventTrailerLength() { return 8; }
    
    std::uint32_t GetIntFromBinaryString(int numByte, unsigned char *str);
    
    std::uint32_t GetIntFromBinaryStringReversed(int numByte, unsigned char *str);
    
    static const std::uint32_t TRAILER_WORD = 0xbfbfbfbf;
    
}

namespace MOSAICBoardDecoder {
    
    /// Adapt the (char x 4) -> (unsigned int) conversion depending to the endianess
    std::uint32_t EndianAdjust(unsigned char *buf);

}

#endif
