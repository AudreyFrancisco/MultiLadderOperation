#include <iostream>
#include <vector>
#include "TDigitalAnalysis.h"

TDigitalAnalysis::TDigitalAnalysis(std::deque<TScanHisto> *histoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(histoQue, aScan, aScanConfig, aMutex) 
{
  m_ninj = m_config->GetParamValue("NINJ");
}


//TODO: Implement HasData
bool TDigitalAnalysis::HasData(TScanHisto &histo, TChipIndex idx, int col) 
{
  return true;
}


void TDigitalAnalysis::InitCounters (std::vector <TChipIndex> chipList) 
{
  m_counters.clear();
  for (int i = 0; i < chipList.size(); i++) {
    TCounter counter;
    counter.boardIndex = chipList.at(i).boardIndex;
    counter.receiver   = chipList.at(i).dataReceiver;
    counter.chipId     = chipList.at(i).chipId;
    counter.nCorrect   = 0;
    counter.nIneff     = 0;
    counter.nNoisy     = 0;
    m_counters.push_back(counter);
  }
}


void TDigitalAnalysis::WriteHitData(std::vector <TChipIndex> chipList, TScanHisto histo, int row) 
{
  char fName[100];
  for (int ichip = 0; ichip < chipList.size(); ichip++) {
    sprintf(fName, "Digital_%s_B%d_Rcv%d_Ch%d.dat", m_config->GetfNameSuffix(), 
	                                            chipList.at(ichip).boardIndex, 
                                                    chipList.at(ichip).dataReceiver, 
                                                    chipList.at(ichip).chipId);
    FILE *fp = fopen (fName, "a");
    for (int icol = 0; icol < 1024; icol ++) {
      if (histo(chipList.at(ichip), icol) > 0) {  // write only non-zero values
        fprintf(fp, "%d %d %d\n", icol, row, (int) histo(chipList.at(ichip), icol));
      }
    }
    fclose(fp);
  }
}


void TDigitalAnalysis::Run() 
{
  std::vector <TChipIndex> chipList;

  while (m_histoQue->size() == 0) {
    sleep(1);
  }

  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    if (m_histoQue->size() > 0) {
      while (!(m_mutex->try_lock()));
    
      TScanHisto histo = m_histoQue->front();
      if (m_first) {
        histo.GetChipList(chipList);
        InitCounters     (chipList);
        m_first = false;
      }

      m_histoQue->pop_front();
      m_mutex   ->unlock();

      int row = histo.GetIndex();
      std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size() << std::endl;
      WriteHitData(chipList, histo, row);
      for (int ichip = 0; ichip < chipList.size(); ichip++) {
        for (int icol = 0; icol < 1024; icol ++) {
          int hits = (int) histo (chipList.at(ichip), icol);
          if      (hits == m_ninj) m_counters.at(ichip).nCorrect ++;         
          else if (hits >  m_ninj) m_counters.at(ichip).nNoisy ++;
          else if (hits >  0)      m_counters.at(ichip).nIneff ++;
        }
      }
    }
    else usleep (300);
  }
}

