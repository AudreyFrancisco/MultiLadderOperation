#ifndef TCHIPERRORCOUNTER_H
#define TCHIPERRORCOUNTER_H

/**
 * \class TChipErrorCounter
 *
 * \brief Container for various types of bad pixel hits for a chip
 *
 * \author Andry Rakotozafindrabe
 *
 * For a given chip, this class collect the list of corrupted hit pixels. They can
 * be due to a stuck pixel, a bad region Id, a bad double column Id, a bad address, 
 * a dead, inefficient, or a hot pixel (see TPixHit class for the enumeration of
 * bad hits). This class also counts the number of 8b10b encoder errors and 
 * the number of priority encoder errors for the chip.
 *
 * For a given chip, the number of occurences of the following errors are extracted 
 * from the list of corrupted hit pixels:
 * - number of stuck pixel hits (due to priority encoder errors)
 * - number of pixel hits with a bad address id
 * - number of pixel hits with a bad region id
 * - number of pixel hits with a bad double column id
 * - number of dead pixels
 * - number of inefficient pixels 
 * - number of hot pixels
 * This class can also print the bad pixel hits to screen or to output files, 
 * with a possible selection on the type of flaw.
 */


#include "Common.h"
#include "TPixHit.h"
#include <memory>
#include <vector>

class TChipErrorCounter : public TVerbosity {
    
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

    /// number of inefficient pixels
    unsigned int fNInefficientPixels;
    
    /// number of hot pixels
    unsigned int fNHotPixels;
    
    /// number of 8b10b encoder errors
    unsigned int fN8b10b;
    
    /// boolean to check if the error counters have been filled with values
    bool fFilledErrorCounters;
    
    /// index of the chip for which we collect errors
    common::TChipIndex fIdx;

    /// list of corrupted pixel hits
    std::vector<std::shared_ptr<TPixHit>> fCorruptedHits;

public:
    
    /// default constructor
    TChipErrorCounter();

    /// constructor that sets the chip index
    TChipErrorCounter( const common::TChipIndex aChipIndex );

    /// destructor
    ~TChipErrorCounter();
    
    /// add a corrupted pixel hit to the list
    void AddCorruptedHit( std::shared_ptr<TPixHit> badHit );
    
    /// add a dead pixel to the list
    void AddDeadPixel( unsigned int icol, unsigned int iaddr );
    
    /// add an inefficient pixel to the list
    void AddInefficientPixel( unsigned int icol, unsigned int iaddr );

    /// add a hot pixel to the list
    void AddHotPixel( unsigned int icol, unsigned int iaddr );

    /// count bad hits for each type of flag
    void FindCorruptedHits();
    
    /// dump all errors
    void Dump();
    
    /// write list of hit pixels with a bad flag in an output file
    void WriteCorruptedHitsToFile( const char *fName, bool Recreate = true );

#pragma mark - getters

    /// return the number of 8b10b encoder errors
    inline unsigned int GetN8b10b() const { return fN8b10b; }
    
#pragma mark - increment
    
    /// increment the number of priority encoder errors by the given value
    inline void IncrementNPrioEncoder( const unsigned int value = 1 )
    { fNPrioEncoder += value; }
    
    /// increment the number of 8b10b encoder errors by the given value for this chip
    void IncrementN8b10b( const unsigned int boardReceiver,
                          const unsigned int value = 1 );
    
private:
    
    /// find the bad hits based on a given flag
    void FindCorruptedHits( const TPixFlag flag );
    
    /// write list of corrupted hits in an output file for a given flag
    void WriteCorruptedHitsToFile( const TPixFlag flag, const char *fName,
                                  bool Recreate = true );

};

#endif
