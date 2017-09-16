#ifndef TERRORCOUNTER_H
#define TERRORCOUNTER_H

/**
 * \class TErrorCounter
 *
 * \brief Simple container for various types of readout errors
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
 */

#include <vector>
#include <memory>
#include "TPixHit.h"

class TErrorCounter {
    
    /// number of 8b10b encoder errors
    unsigned int fN8b10b;
    
    /// number of corrupted events
    unsigned int fNCorruptEvent;
    
    /// number of priority encoder errors
    unsigned int fNPrioEncoder;
    
    /// number of times any readout board had a timeout error
    unsigned int fNTimeout;
    
    /// list of corrupted pixel hits
    std::vector<std::shared_ptr<TPixHit>> fCorruptedHits;
    
public:
    
    /// constructor
    TErrorCounter();
    
    /// destructor
    ~TErrorCounter();
    
    /// add a corrupted pixel hit to the list
    void AddCorruptedHit( std::shared_ptr<TPixHit> badHit );
    
    /// print all error counters on screen
    void Dump();
    
    /// dump the list of corrupted pixel hits
    void DumpCorruptedHits( const TPixFlag flag = TPixFlag::kUNKNOWN );
    
#pragma mark - setters

    /// set the number of 8b10b encoder errors to the given value
    inline void SetN8b10b( const unsigned int value ) { fN8b10b = value; }
    
    /// set the number of corrupted events to the given value
    inline void SetNCorruptEvent( const unsigned int value ) { fNCorruptEvent = value; }
    
    /// set the number of priority encoder errors to the given value
    inline void SetNPrioEncoder( const unsigned int value ) { fNPrioEncoder = value; }
    
    /// set the number of timeout errors to the given value
    inline void SetNTimeout( const unsigned int value ) { fNTimeout = value; }
    
#pragma mark - increment

    /// increment the number of 8b10b encoder errors by the given value
    inline void IncrementN8b10b( const unsigned int value = 1 )
        { fN8b10b += value; }

    /// increment the number of corrupted events by the given value
    inline void IncrementNCorruptEvent( const unsigned int value = 1 )
        { fNCorruptEvent += value; }

    /// increment the number of priority encoder errors by the given value
    inline void IncrementNPrioEncoder( const unsigned int value = 1 )
        { fNPrioEncoder += value; };
    
    /// increment the number of timeout errors by the given value
    inline void IncrementNTimeout( const unsigned int value = 1 )
        { fNTimeout += value; }

#pragma mark - getters
    
    /// return the number of 8b10b encoder errors
    inline unsigned int GetN8b10b() const { return fN8b10b; }
    
    /// return the number of corrupted events
    inline unsigned int GetNCorruptEvent() const { return fNCorruptEvent; }
    
    /// return the number of priority encoder errors
    inline unsigned int GetNPrioEncoder() const { return fNPrioEncoder; }
    
    /// return the number of timeout errors
    inline unsigned int GetNTimeout() const { return fNTimeout; }

};

#endif
