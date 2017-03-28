#ifndef CHIP_H
#define CHIP_H

#include <string>

namespace TChipData {
    const int kInitValue = -1;
}

class TChip {
    
public:
    
    TChip();
    TChip( const int chipId, const int ci, const int rc );
    virtual ~TChip() {}
    
    void SetChipId( const int chipId );
    void SetControlInterface( const int ci );
    void SetReceiver( const int rc );
    inline void SetEnable( const bool value = true ) { fEnabled = value; }
    
    int GetChipId() const;
    int GetControlInterface() const;
    int GetReceiver() const;
    inline bool IsEnabled() const { return fEnabled; }

protected:
    
    bool fEnabled;
    int  fChipId;
    int  fControlInterface;
    int  fReceiver;
    
};

#endif /* CHIP_H */
