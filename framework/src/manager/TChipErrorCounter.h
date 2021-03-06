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
 * bad hit types). This class also counts the number of 8b10b encoder errors and
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
 * with a possible selection on the type of flaw. Thanks to its data member of
 * type THitMapDiscordant, this class also plot a hit map of the corrupted pixels
 * and their firing frequency given the number of injected triggers per pixel.
 */


#include "Common.h"
#include "TPixHit.h"
#include <memory>
#include <deque>

class THitMapDiscordant;

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
    std::deque<std::shared_ptr<TPixHit>> fCorruptedHits;
    
    /// class used to locate bad pixel on a hit map
    std::shared_ptr<THitMapDiscordant> fHitMap;

public:
    
    /// default constructor
    TChipErrorCounter();

    /// constructor that sets the chip index
    TChipErrorCounter(  const TDeviceType dt,
                        const common::TChipIndex aChipIndex,
                        const unsigned int nInjections );

    /// destructor
    ~TChipErrorCounter();
    
    /// propagate the verbosity level to data members
    virtual void SetVerboseLevel( const int level );

    /// set the device type
    void SetDeviceType( const TDeviceType dt );
    
    /// add a corrupted pixel hit to the list
    void AddCorruptedHit( std::shared_ptr<TPixHit> badHit );
    
    /// add a dead pixel to the list
    void AddDeadPixel( const unsigned int icol, const unsigned int iaddr );
    
    /// add an inefficient pixel to the list
    void AddInefficientPixel( const unsigned int icol, const unsigned int iaddr,
                              const double nhits );

    /// add a hot pixel to the list
    void AddHotPixel( const unsigned int icol, const unsigned int iaddr,
                      const double nhits );

    /// count bad hits for each type of flag
    void ClassifyCorruptedHits();
    
    /// dump all errors
    void Dump();
    
    /// write list of hit pixels with a bad flag in an output file
    void WriteCorruptedHitsToFile( const char *fName, bool Recreate = true );
    
    /// draw and save hit map of bad pixels and their firing frequency distribution
    void DrawAndSaveToFile( const char *fName );

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
                                  const bool Recreate = true );

};

#endif
