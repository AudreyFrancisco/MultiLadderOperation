#ifndef TSCANALYSIS_H
#define TSCANALYSIS_H

#include <deque>
#include <mutex>
#include <map>
#include <string>
#include <string.h>

#include "THIC.h"
#include "Common.h"

class THisto;
class TScan;
class TScanConfig;
class TScanHisto;

enum THicClassification {CLASS_UNTESTED, CLASS_GREEN, CLASS_RED, CLASS_ORANGE};

// base class for classes that contain chip results
// derive class for each analysis
class TScanResultChip {
 public:
  TScanResultChip () {};
  virtual void WriteToFile (FILE *fp) = 0;
};


class TScanResultHic {
  friend class TScanAnalysis;
 protected: 
  std::map <int, TScanResultChip*> m_chipResults;
  char                             m_resultFile[200];
  THicClassification               m_class;
 public:
  TScanResultHic () {};
  virtual void       WriteToFile       (FILE *fp) = 0;
  int                AddChipResult     (int aChipId, TScanResultChip *aChipResult);
  void               SetResultFile     (const char *fName) {strcpy(m_resultFile, fName);};
  THicClassification GetClassification ()                  {return m_class;};
};


// base class for classes containing complete results
// derive class for each analysis
class TScanResult {
 private:
 protected: 
  std::map <int, TScanResultChip*>        m_chipResults;
  std::map <std::string, TScanResultHic*> m_hicResults;
 public: 
  TScanResult   () {};
  //virtual TScanResult *clone() const=0;
 //TScanResult(const TScanResult &other){m_chipResults=other.m_chipResults;}
 //assignment operation from my base class 
//TScanResult &operator=(const TScanResult &other){if (&other!=this) return *this; m_chipResults=other.m_chipResults; return *this;} 
  int              AddChipResult     (common::TChipIndex idx, 
		                      TScanResultChip *aChipResult);
  int              AddChipResult     (int aIntIndex, TScanResultChip *aChipResult);
  int              AddHicResult      (std::string hicId,   TScanResultHic  *aHicResult);
  int              GetNChips         ()     {return (int) m_chipResults.size();};
  int              GetNHics          ()     {return (int) m_hicResults.size();};
  void             WriteToFile       (const char *fName);
  virtual void     WriteToFileGlobal (FILE *fp)          = 0;
  virtual void     WriteToDB         (const char *hicID) = 0;
  TScanResultChip *GetChipResult     (common::TChipIndex idx);
  TScanResultHic  *GetHicResult      (std::string hic);
  std::map <std::string, TScanResultHic*> GetHicResults () {return m_hicResults;};  
};


typedef enum resultType {status, deadPix, noisyPix, ineffPix, badDcol, thresh, noise, threshRms, noiseRms, noiseOcc, fifoErr0, fifoErrf, fifoErra, fifoErr5} TResultVariable;


class TScanAnalysis {
 protected:
  std::deque <TScanHisto>         *m_histoQue;
  std::vector<common::TChipIndex>  m_chipList;
  std::vector<THic *>              m_hics;
  std::mutex                      *m_mutex;
  std::map <const char *, TResultVariable> m_variableList;
  TScan                       *m_scan;
  TScanConfig                 *m_config;
  TScanResult                 *m_result;
  bool                         m_first;
  virtual TScanResultChip     *GetChipResult     () = 0;
  virtual TScanResultHic      *GetHicResult      () = 0;
  void                         CreateHicResults  ();
  virtual void                 CreateResult      () = 0;
  int                          ReadChipList      ();
 public:
  TScanAnalysis (std::deque<TScanHisto> *histoQue, 
                 TScan                  *aScan, 
                 TScanConfig            *aScanConfig, 
                 std::vector <THic*>     hics,
                 std::mutex             *aMutex);
  virtual void Initialize      () = 0; 
  virtual void Run             () = 0;
  virtual void Finalize        () = 0; 
  std::map <const char *, TResultVariable> GetVariableList () {return m_variableList;}
};


#endif
