#ifndef TPOWERTEST_H
#define TPOWERTEST_H

#include <mutex>

#include "Common.h"
#include "TScan.h"
#include "THisto.h"


typedef struct {
  float iddaSwitchon;
  float idddSwitchon;
  float iddaClocked;
  float idddClocked;
  float iddaConfigured;
  float idddConfigured;
  float ibias0;
  float ibias3;
} THicCurrents;

class TPowerTest : public TScan {
 private: 
  THic *m_testHic;
  void CreateMeasurements();
 protected:
 public:
  TPowerTest  (TScanConfig                   *config, 
               std::vector <TAlpide *>        chips, 
               std::vector <THic*>            hics,
               std::vector <TReadoutBoard *>  boards, 
               std::deque<TScanHisto>        *histoque, 
               std::mutex                    *aMutex);
  ~TPowerTest () {};

  void Init        ();
  void Execute     ();
  void Terminate   ();
  void LoopStart   (int loopIndex) {m_value[loopIndex] = m_start[loopIndex];};
  void LoopEnd     (int loopIndex) {};
  void PrepareStep (int loopIndex);
};



#endif
