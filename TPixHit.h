#ifndef TPIXHIT_H
#define TPIXHIT_H

/**
 * \class TPixHit
 *
 * \brief Simple container for the full address of a "hit" (responding) pixel
 *
 * The "hit" (responding) pixel is identified thanks to the following coordinates:
 * chip id, region id, double colum id, index (address) of the pixel in the double 
 * column.
 */

class TPixHit {
    
    /// id of the chip to which belong the hit pixel
    unsigned int fChipId;
    
    /// region id
    unsigned int fRegion;
    
    /// double column id
    unsigned int fDcol;
    
    /// index (address) of the pixel in the double column
    unsigned int fAddress;
    
    /// id of the last region of the chip
    static const unsigned int MAX_REGION = 31;  // [0 .. 31] 32 regions

    /// id of the last double column in a region of the chip
    static const unsigned int MAX_DCOL = 15;  // [0 .. 15] 16 double columns / region

    /// index of the last pixel in a double column of the chip
    static const unsigned int MAX_ADDR = 1023;  // [0 .. 1023] 1024 pixels / double column

    /// illegal chip id, used for initialization
    static const unsigned int ILLEGAL_CHIP_ID = 15;  // i.e. 4'b1111

public:
    
    TPixHit();
    virtual ~TPixHit();

#pragma mark - setters

    void SetChipId( const unsigned int value );
    void SetRegion( const unsigned int value );
    void SetDoubleColumn( const unsigned int value );
    void SetAddress( const unsigned int value );

#pragma mark - getters

    unsigned int GetChipId() const;
    unsigned int GetRegion() const;
    unsigned int GetDoubleColumn() const;
    unsigned int GetAddress() const;
};

#endif
