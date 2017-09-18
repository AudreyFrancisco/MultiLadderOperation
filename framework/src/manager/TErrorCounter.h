#ifndef TERRORCOUNTER_H
#define TERRORCOUNTER_H

/**
 * \class TErrorCounter
 *
 * \brief Simple container for readout errors from boards or chips in the device
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
 * - number of bad hits for each type of bad hits for each chip.
 * See the class TChipErrorCounter for more details about the errors that are 
 * considered from a given chip.
 */

#include <map>
#include <memory>
#include <vector>
#include "Common.h"
#include "TChipErrorCounter.h"
#include "TPixHit.h"

class TScanHisto;

class TErrorCounter {
    
    /// number of times any readout board had a timeout error
    unsigned int fNTimeout;
    
    /// number of 8b10b encoder errors
    unsigned int fN8b10b;
    
    /// number of corrupted events
    unsigned int fNCorruptEvent;

    /// list of chips
    std::vector<common::TChipIndex> fChipList;
    
    /// error counter (one per chip index)
    std::map<int, TChipErrorCounter> fCounterCollection;
    
public:
    
    /// default constructor
    TErrorCounter();

    /// destructor
    ~TErrorCounter();
    
    /// add a corrupted pixel hit to the list
    void AddCorruptedHit( std::shared_ptr<TPixHit> badHit );
    
    /// add a dead pixel to the list
    void AddDeadPixel( common::TChipIndex idx,
                       unsigned int icol, unsigned int iaddr );

    /// add an almost dead pixel to the list
    void AddAlmostDeadPixel( common::TChipIndex idx,
                             unsigned int icol, unsigned int iaddr );

    /// create the collection of chip error counters from the map of histograms
    void Init( std::shared_ptr<TScanHisto> aScanHisto );
    
    /// print all error counters on screen
    void Dump();
    
#pragma mark - setters

    /// set the number of timeout errors to the given value
    inline void SetNTimeout( const unsigned int value ) { fNTimeout = value; }

    /// set the number of corrupted events to the given value
    inline void SetNCorruptEvent( const unsigned int value ) { fNCorruptEvent = value; }

    /// set the number of 8b10b encoder errors to the given value
    inline void SetN8b10b( const unsigned int value ) { fN8b10b = value; }
    
#pragma mark - increment

    /// increment the number of timeout errors by the given value
    inline void IncrementNTimeout( const unsigned int value = 1 )
    { fNTimeout += value; }

    /// increment the number of corrupted events by the given value
    inline void IncrementNCorruptEvent( const unsigned int value = 1 )
    { fNCorruptEvent += value; }

    /// increment the number of 8b10b encoder errors by the given value
    inline void IncrementN8b10b( const unsigned int value = 1 )
    { fN8b10b += value; }
    
    /// increment the number of priority encoder errors by the given value
    void IncrementNPrioEncoder( std::shared_ptr<TPixHit> badHit, const unsigned int value = 1 );

#pragma mark - getters
    
    /// return the number of timeout errors
    inline unsigned int GetNTimeout() const { return fNTimeout; }

    /// return the number of corrupted events
    inline unsigned int GetNCorruptEvent() const { return fNCorruptEvent; }

    /// return the number of 8b10b encoder errors
    inline unsigned int GetN8b10b() const { return fN8b10b; }

private:
    
    /// add an error counter for a given chip index
    void AddChipErrorCounter( common::TChipIndex idx );
    
    /// return the integer that indexes the TChipIndex in the map of TChipErrorCounters
    int GetMapIntIndex( common::TChipIndex idx ) const;

};

#endif
