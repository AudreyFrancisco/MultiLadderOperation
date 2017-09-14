/**
 * \namespace Common
 *
 * \brief Contain types, constants and functions for common use in manager classes.
 *
 */


#ifndef COMMON_H
#define COMMON_H

#include <string>

class TPixHit;

namespace common {
    
    /// \typedef TChipIndex
    /// Struct gathers info to uniquely identify a chip (board id, receiver id, chip id)
    typedef struct {
        unsigned int boardIndex;
        unsigned int dataReceiver;
        unsigned int chipId;
    } TChipIndex;
    
    /// Function that helps to generate part of the filename from TChipIndex
    extern std::string GetFileName( TChipIndex aChipIndex,
                                   std::string suffix );
    
    /// id of the last region of the chip
    const unsigned int MAX_REGION = 31;  // [0 .. 31] 32 regions
    
    /// id of the last double column of the chip
    const unsigned int MAX_DCOL = 511;  // [0 .. 511] 16 double columns / region
    
    /// number of double column per region
    const unsigned int NDCOL_PER_REGION = 16;  // 16 double columns / region
    
    /// index of the last pixel in a double column of the chip
    const unsigned int MAX_ADDR = 1023;  // [0 .. 1023] 1024 pixels / double column

    
}

#endif
