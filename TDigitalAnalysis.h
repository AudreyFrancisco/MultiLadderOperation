#ifndef TDIGITALANALYSIS_H
#define TDIGITALANALYSIS_H

#include <deque>
#include <mutex>
#include <vector>

#include "TScanAnalysis.h"
#include "TScanConfig.h"
#include "TScan.h"
#include "THisto.h"

typedef struct {
  int boardIndex;
  int receiver;
  int chipId;
  int nCorrect;
  int nIneff;
  int nNoisy;
} TCounter;


class TDigitalAnalysis : public TScanAnalysis {
 private:
  std::vector <TCounter> m_counters;
  int                    m_ninj;
  bool HasData      (TScanHisto &histo, TChipIndex idx, int col);
  void InitCounters (std::vector <TChipIndex> chipList);
  void WriteHitData (std::vector <TChipIndex> chipList, TScanHisto histo, int row); 
 protected:
 public:
  TDigitalAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex);
  void Run();
  std::vector <TCounter> GetCounters() {return m_counters;};
 };

#endif
