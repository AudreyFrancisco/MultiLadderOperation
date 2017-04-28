#ifndef TTHRESHOLDSCAN_H
#define TTHRESHOLDSCAN_H

#include <deque>
#include <vector>
#include <memory>
#include "TScan.h"
#include "AlpideDecoder.h"

class THisto;
class TScanHisto;

class TThresholdScan : public TMaskScan {
private:
    int         fVPULSEH;
    int         fNTriggers;
    
    void ConfigureChip  ( const int ichip);
    void ConfigureBoard ( const int iboard );
    void FillHistos     ( std::vector<TPixHit> *Hits, const int iboard );
protected:
    std::shared_ptr<THisto> CreateHisto();
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
