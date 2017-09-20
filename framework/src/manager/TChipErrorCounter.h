#ifndef TCHIPERRORCOUNTER_H
#define TCHIPERRORCOUNTER_H

/**
 * \class TChipErrorCounter
 *
 * \brief Container for various types of bad pixel hits for a chip
 *
 * \author Andry Rakotozafindrabe
 *
 * The occurences of the following errors are counted in this class:
 * - number of 8b10b encoder errors
 * - number of corrupted events (events with bad hits, namely stuck pixel hits, or a bad
 *   chip Id, or a bad region Id, or a bad double column Id, or a bad address); see
 *   TPixHit class for the definition of bad hits
 * - number of priority encoder errors (stuck pixel hits), i.e. a subset of corrupted
 *   events
 * - number of times any readout board had a timeout error
 * This class also collect the bad pixel hits and can print them to screen, with a
 * possible selection on the type of flaw.
 */


#include "Common.h"
#include "TPixHit.h"
#include <memory>
#include <vector>

class TChipErrorCounter {
    
    /// number of priority encoder errors
    unsigned int fNPrioEncoder;

    /// number of hits with bad address flag
    unsigned int fNBadAddressIdFlag;
    
    /// number of hits with bad col id flag
    unsigned int fNBadColIdFlag;
    
    /// number of hits with bad region id flag
    unsigned int fNBadRegionIdFlag;
    
    /// number of hits with stuck pixel flag
    unsigned int fNStuckPixelFlag;

    /// number of dead pixels
    unsigned int fNDeadPixels;

    /// number of almost dead pixels
    unsigned int fNAlmostDeadPixels;
    
    /// list of corrupted pixel hits
    std::vector<std::shared_ptr<TPixHit>> fCorruptedHits;
    

public:
    
    /// default constructor
    TChipErrorCounter();
    
    /// destructor
    ~TChipErrorCounter();
    
    /// add a corrupted pixel hit to the list
    void AddCorruptedHit( std::shared_ptr<TPixHit> badHit );
    
    /// add a dead pixel to the list
    void AddDeadPixel( common::TChipIndex idx,
                      unsigned int icol, unsigned int iaddr );
    
    /// add an almost dead pixel to the list
    void AddAlmostDeadPixel( common::TChipIndex idx,
                            unsigned int icol, unsigned int iaddr );
    
    /// dump the list of bad hits
    void DumpCorruptedHits();
    
    /// dump all errors
    void Dump( common::TChipIndex idx );
    
#pragma mark - setters
    
    /// set the number of priority encoder errors to the given value
    inline void SetNPrioEncoder( const unsigned int value ) { fNPrioEncoder = value; }

    
#pragma mark - increment
    
    /// increment the number of priority encoder errors by the given value
    inline void IncrementNPrioEncoder( const unsigned int value = 1 )
    { fNPrioEncoder += value; }
    
private:
    
    /// dump the list of bad hits
    void DumpCorruptedHits( const TPixFlag flag );

};

#endif
