#ifndef TTHRESHOLDSCAN_H
#define TTHRESHOLDSCAN_H

#include <deque>
#include <vector>
#include <memory>
#include "TScan.h"

class THisto;
class TScanHisto;
class TPixHit;
class AlpideDecoder;

class TScanConfig;
class TDevice;

class TThresholdScan : public TMaskScan {
private:
    int         fVPULSEH;
    int         fNTriggers;
    std::vector<std::shared_ptr<TPixHit>> fHits;
    
    void ConfigureChip  ( const int ichip);
    void ConfigureBoard ( const int iboard );
    void FillHistos     ( const int iboard );
protected:
    std::shared_ptr<THisto> CreateHisto();
    friend class AlpideDecoder;
public:
    TThresholdScan();
    TThresholdScan( std::shared_ptr<TScanConfig> config,
                   std::shared_ptr<TDevice> aDevice,
                   std::deque<TScanHisto> *histoQue );
    virtual ~TThresholdScan() {};
    
    void Init        ();
    void PrepareStep ( const int loopIndex );
    void LoopEnd     ( const int loopIndex );
    void LoopStart   ( const int loopIndex ) { fValue[loopIndex] = fStart[loopIndex];};
    void Execute     ();
    void Terminate   ();
};


#endif
