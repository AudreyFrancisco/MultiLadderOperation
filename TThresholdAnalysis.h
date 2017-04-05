#ifndef TTHRESHOLDANALYSIS_H
#define TTHRESHOLDANALYSIS_H

#include <deque>
#include <mutex>

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"

class TThresholdAnalysis : public TScanAnalysis {
 private:
  bool HasData(TScanHisto histo, TChipIndex idx, int col);
 protected:
 public:
  TThresholdAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  void Run();
 };

#endif
