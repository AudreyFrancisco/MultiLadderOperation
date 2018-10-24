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
#include <deque>
#include "Common.h"
#include "TChipErrorCounter.h"
#include "TPixHit.h"
#include "TVerbosity.h"

class TScanHisto;

class TErrorCounter : public TVerbosity {
    
    /// number of times any readout board had a timeout error
    unsigned int fNTimeout;
    
    /// number of corrupted events
    unsigned int fNCorruptEvent;
    
    /// number of event over size errors
    unsigned int fNEventOverSizeError;

    /// error counter (one per chip index)
    std::map<int, TChipErrorCounter> fCounterCollection;

    /// list of hits with bad chip id flag
    std::deque<std::shared_ptr<TPixHit>> fBadChipIdHits;

    /// device type
    TDeviceType fDeviceType;

public:
    
    /// default constructor
    TErrorCounter();

    /// constructor that inits the device type
    TErrorCounter( const TDeviceType dt );

    /// destructor
    ~TErrorCounter();
    
    /// add a corrupted pixel hit to the list
    void AddCorruptedHit( std::shared_ptr<TPixHit> badHit );

    /// add a dead pixel to the list
    void AddDeadPixel( const common::TChipIndex idx,
                       const unsigned int icol, const unsigned int iaddr );

    /// add an inefficient pixel to the list
    void AddInefficientPixel( const common::TChipIndex idx,
                             const unsigned int icol, const unsigned int iaddr,
                             const double nhits );

    /// add a hot pixel to the list
    void AddHotPixel( const common::TChipIndex idx,
                      const unsigned int icol, const unsigned int iaddr,
                      const double nhits);

    /// create the collection of chip error counters from the map of histograms
    void Init( std::shared_ptr<TScanHisto> aScanHisto,
               const unsigned int nInjections );
    
    /// print all error counters on screen
    void Dump();
    
    /// count bad hits for each type of flag and for each chip
    void ClassifyCorruptedHits();
    
    /// write list of hit pixels with a bad flag in an output file for each chip
    void WriteCorruptedHitsToFile( const char *fName, const bool Recreate = true );
    
    /// draw and save hit map of bad pixels and their firing frequency distribution
    void DrawAndSaveToFile( const char *fName );
    
#pragma mark - setters

    /// set the number of timeout errors to the given value
    inline void SetNTimeout( const unsigned int value ) { fNTimeout = value; }

    /// set the number of corrupted events to the given value
    inline void SetNCorruptEvent( const unsigned int value ) { fNCorruptEvent = value; }
 
    /// set the number of event over size errors
    inline void SetNEventOverSizeError( const unsigned int value ) { fNEventOverSizeError = value; }

   /// propagate the verbosity level to data members
    virtual void SetVerboseLevel( const int level );
    
#pragma mark - increment

    /// increment the number of timeout errors by the given value
    inline void IncrementNTimeout( const unsigned int value = 1 )
    { fNTimeout += value; }

    /// increment the number of corrupted events by the given value
    inline void IncrementNCorruptEvent( const unsigned int value = 1 )
    { fNCorruptEvent += value; }

    /// increment the number of event over size errors
    inline void IncrementNEventOverSizeError(const unsigned int value = 1 )
    { fNEventOverSizeError += value; }

    /// increment the number of 8b10b encoder errors by the given value
    void IncrementN8b10b( const unsigned int boardReceiver,
                          const unsigned int value = 1 );
    
    /// increment the number of priority encoder errors by the given value
    void IncrementNPrioEncoder( std::shared_ptr<TPixHit> badHit, const unsigned int value = 1 );

#pragma mark - getters
    
    /// return the number of timeout errors
    inline unsigned int GetNTimeout() const { return fNTimeout; }

    /// return the number of corrupted events
    inline unsigned int GetNCorruptEvent() const { return fNCorruptEvent; }

    /// return the number of event over size errors
    inline unsigned int GetNEventOverSizeError() const { return fNEventOverSizeError; }


private:
    
    /// add an error counter for a given chip index
    void AddChipErrorCounter( const common::TChipIndex idx,
                              const unsigned int nInjections  );
    
};

#endif
