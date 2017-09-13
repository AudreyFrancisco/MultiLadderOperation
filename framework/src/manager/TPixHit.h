#ifndef TPIXHIT_H
#define TPIXHIT_H

/**
 * \class TPixHit
 *
 * \brief Simple container for the full address of a "hit" (responding) pixel
 *
 * The "hit" (responding) pixel is identified thanks to the following coordinates:
 * chip id, double colum id, index (address) of the pixel in the double 
 * column. 
 * The container also store the board index and the board receiver id that are
 * collecting the data from the chip.
 */

#include "Common.h"
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

class TPixHit {

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
    unsigned int GetChipId( const bool print_warning = false ) const;
    unsigned int GetRegion( const bool print_warning = false ) const;
    unsigned int GetDoubleColumn( const bool print_warning = false ) const;
    unsigned int GetAddress( const bool print_warning = false ) const;
    TPixFlag GetPixFlag( const bool print_warning = false ) const;
    bool IsPixHitCorrupted( const bool print_warning = false ) const;
    
#pragma mark - other
    
    void DumpPixHit();
    
};

#endif
