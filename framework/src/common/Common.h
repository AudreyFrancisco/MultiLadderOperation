/**
 * \namespace Common
 *
 * \brief Contain types, constants and functions for common use in manager classes.
 *
 */


#ifndef COMMON_H
#define COMMON_H

#include <string>

// definition of standard setup types:
//   - single chip in OB mode with MOSAIC
//   - single chip in IB mode with MOSAIC
//   - MFT 5-chips ladder with MOSAIC
//   - etc ...
enum class TDeviceType {
    kCHIP_DAQ,
    kTELESCOPE,
    kOBHIC,
    kIBHIC,
    kMFT_LADDER5,
    kMFT_LADDER4,
    kMFT_LADDER3,
    kMFT_LADDER2,
    kOBCHIP_MOSAIC,
    kIBCHIP_MOSAIC,
    kHALFSTAVE,
    kUNKNOWN
};

class TPixHit;

namespace common {
    
    /// \typedef TChipIndex
    /// Struct gathers info to uniquely identify a chip
    typedef struct {
        unsigned int boardIndex;
        unsigned int dataReceiver;
        TDeviceType deviceType; // not taken into account for the mapping
        unsigned int deviceId; // for e.g. ladder Id
        unsigned int chipId;
    } TChipIndex;
    
    /// Function that helps to generate part of the filename from TChipIndex
    extern std::string GetFileName( const TChipIndex aChipIndex,
                                   std::string suffix, std::string optional = "",
                                   std::string fileExtention = ".dat");
    
    /// Compare two TChipIndex structures
    extern bool SameChipIndex( const TChipIndex lhs, const TChipIndex rhs );

    /// Check if the device type corresponds to a MFT ladder
    extern bool IsMFTladder( const TChipIndex aChipIndex );
    
    /// Check if the device type corresponds to an ITS IB hic or an IB-like telescope
    extern bool IsIBhic( const TChipIndex aChipIndex );

    /// dump to screen the id
    extern void DumpId( const TChipIndex aChipIndex );

    /// dump to screen the id
    extern void DumpDeviceId( const TChipIndex aChipIndex );

    /// id of the last region of the chip
    const unsigned int MAX_REGION = 31;  // [0 .. 31] 32 regions
    
    /// id of the last double column of the chip
    const unsigned int MAX_DCOL = 511;  // [0 .. 511] 16 double columns / region
    
    /// number of double column per region
    const unsigned int NDCOL_PER_REGION = 16;  // 16 double columns / region
    
    /// index of the last pixel in a double column of the chip
    const unsigned int MAX_ADDR = 1023;  // [0 .. 1023] 1024 pixels / double column

    /// number of pixels in a row of the chip
    const unsigned int NPIX_PER_ROW = 1024;  // 1024 pixels / row
    
    /// number of lines in the pixel matrix
    const unsigned int NLINES = 512;
    
    /// return the integer that indexes the TChipIndex in the map
    extern int GetMapIntIndex( const TChipIndex idx );
    
    /// return the TChipIndex that corresponds to the integer that is used as map index
    extern TChipIndex GetChipIndexFromMapInt( const int intIndex );
    
}

#endif
