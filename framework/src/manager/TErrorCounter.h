#ifndef TERRORCOUNTER_H
#define TERRORCOUNTER_H

/**
 * \class TErrorCounter
 *
 * \brief Simple container for various types of readout errors
 *
 * The occurences of the following errors are counted in this class:
 * - number of 8b10b encoder errors
 * - number of corrupted events (events with for e.g. two or more hits for the same pixel)
 * - number of priority encoder errors
 * - number of times any readout board had a timeout error
 */

class TErrorCounter {
    
    /// number of 8b10b encoder errors
    unsigned int fN8b10b;
    
    /// number of corrupted events (events with for e.g. many hits for the same pixel)
    unsigned int fNCorruptEvent;
    
    /// number of priority encoder errors
    unsigned int fNPrioEncoder;
    
    /// number of times any readout board had a timeout error
    unsigned int fNTimeout;
    
public:
    
    TErrorCounter();
    ~TErrorCounter();
    void Dump();
    
#pragma mark - setters
    
    inline void SetN8b10b( const unsigned int value ) { fN8b10b = value; }
    inline void SetNCorruptEvent( const unsigned int value ) { fNCorruptEvent = value; }
    inline void SetNPrioEncoder( const unsigned int value ) { fNPrioEncoder = value; }
    inline void SetNTimeout( const unsigned int value ) { fNTimeout = value; }
    
#pragma mark - increment

    inline void IncrementN8b10b( const unsigned int value = 1 )
        { fN8b10b += value; }
    inline void IncrementNCorruptEvent( const unsigned int value = 1 )
        { fNCorruptEvent += value; }
    inline void IncrementNPrioEncoder( const unsigned int value = 1 )
        { fNPrioEncoder += value; };
    inline void IncrementNTimeout( const unsigned int value = 1 )
        { fNTimeout += value; }

#pragma mark - getters
    
    inline unsigned int GetN8b10b() const { return fN8b10b; }
    inline unsigned int GetNCorruptEvent() const { return fNCorruptEvent; }
    inline unsigned int GetNPrioEncoder() const { return fNPrioEncoder; }
    inline unsigned int GetNTimeout() const { return fNTimeout; }

};

#endif
