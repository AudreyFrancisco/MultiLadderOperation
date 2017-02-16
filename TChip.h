#ifndef CHIP_H
#define CHIP_H

#include <stdint.h>
#include <string>

#include "MosaicSrc/mexception.h"

namespace TChipData {
    const int kInitValue = -1;
}

class TChip {
    
public:
    
    TChip();
    TChip( const int chipId, const int ci, const int rc );
    ~TChip() {}
    
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

class TChipError : public MException
{
public:
    explicit TChipError( const std::string& __arg );
};


#endif /* CHIP_H */
