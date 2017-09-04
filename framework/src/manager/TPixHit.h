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
 * The container also store the board index and the board receiver id that are
 * collecting the data from the chip.
 */

enum class TPixFlag {
    kOK,
    kBAD_CHIPID,
    kBAD_REGIONID,
    kBAD_DCOLID,
    kBAD_ADDRESS,
    kSTUCK,
    kUNKNOWN
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
    
    /// id of the last region of the chip
    static const unsigned int MAX_REGION = 31;  // [0 .. 31] 32 regions

    /// id of the last double column of the chip
    static const unsigned int MAX_DCOL = 511;  // [0 .. 511] 16 double columns / region

    /// index of the last pixel in a double column of the chip
    static const unsigned int MAX_ADDR = 1023;  // [0 .. 1023] 1024 pixels / double column

    /// illegal chip id, used for initialization
    static const unsigned int ILLEGAL_CHIP_ID = 15;  // i.e. 4'b1111

public:
    
    TPixHit();
    virtual ~TPixHit();

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
};

#endif
