#ifndef TPIXHIT_H
#define TPIXHIT_H

/**
 * \class TPixHit
 *
 * \brief Simple container for the full address of a "hit" (responding) pixel
 *
 * \author Andry Rakotozafindrabe
 *
 * The "hit" (i.e. responding) pixel is identified thanks to the following coordinates:
 * chip id, double column id, index (address) of the pixel in the double
 * column. The region id is also stored.
 *
 * The container also store the board index and the board receiver id used to
 * collect the data from the chip.
 *
 * Sanity checks are always run on the validity of the chip id, the region id, the
 * double column id and the address. They are used to put a quality flag on the pixel 
 * hit. If the pixel hit is bad in many ways, only one of them can be stored since
 * the flag can only be chosen among a simple enum.
 */

#include "Common.h"
#include "TVerbosity.h"
#include <memory>

enum class TPixFlag {
    kOK = 0 ,
    kBAD_CHIPID = 1,
    kBAD_REGIONID = 2,
    kBAD_DCOLID = 3,
    kBAD_ADDRESS = 4,
    kSTUCK = 5,
    kUNKNOWN = 6
};

class TPixHit : public TVerbosity {

    /// id of the board that read the chip to which belong the hit pixel
    unsigned int fBoardIndex;

    /// id of the receiver on the readout board that gets the chip data
    unsigned int fBoardReceiver;

    /// id of the chip to which belong the hit pixel
    unsigned int fChipId;
    
    /// region id
    unsigned int fRegion;
    
    /// double column id
    unsigned int fDcol;
    
    /// index (address) of the pixel in the double column
    unsigned int fAddress;
    
    /// flag to check the status of this hit pixel
    TPixFlag fFlag;
    
    /// illegal chip id, used for initialization
    static const unsigned int ILLEGAL_CHIP_ID = 15;  // i.e. 4'b1111

public:
    
    /// default constructor
    TPixHit();
    
    /// copy constructors
    TPixHit( const TPixHit& obj );
    TPixHit( const std::shared_ptr<TPixHit> obj );
    
    /// destructor
    virtual ~TPixHit();
    
    /// assignement operator
    TPixHit& operator=(const TPixHit& obj );

#pragma mark - setters

    inline void SetBoardIndex( const unsigned int value ) { fBoardIndex = value; }
    inline void SetBoardReceiver( const unsigned int value ) { fBoardReceiver = value; }
    void SetChipId( const unsigned int value );
    void SetRegion( const unsigned int value );
    void SetDoubleColumn( const unsigned int value );
    void SetAddress( const unsigned int value );
    inline void SetPixFlag( const TPixFlag flag ) { fFlag = flag; }

#pragma mark - getters

    inline unsigned int GetBoardIndex() const { return fBoardIndex; }
    inline unsigned int GetBoardReceiver() const { return fBoardReceiver; }
    unsigned int GetChipId() const;
    unsigned int GetRegion() const;
    unsigned int GetDoubleColumn() const;
    unsigned int GetAddress() const;
    TPixFlag GetPixFlag() const;
    bool IsPixHitCorrupted() const;
    
#pragma mark - other
    
    void DumpPixHit( const bool with_reminder = true );
    
};

#endif
