#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "TThresholdAnalysis.h" 

#include "THisto.h"
#include "TScan.h"
#include "TScanConfig.h"

#include <TCanvas.h> /*H*/
#include <TF1.h>
#include <TFile.h> /*H*/
#include <TGraph.h>
#include <TH1D.h> /*H*/
#include <TH2D.h> /*H*/
#include <TMath.h>
#include <TPaveText.h>

TThresholdResultChip::TThresholdResultChip (): TScanResultChip()
{;}

TThresholdResultChip::~TThresholdResultChip ()
{;}
 
void TThresholdResultChip::SetBoardIndex (unsigned int aBoardIndex)
{m_boardIndex = aBoardIndex;} 

void TThresholdResultChip::SetDataReceiver(unsigned int aDataReceiver)
{m_dataReceiver = aDataReceiver;}

void TThresholdResultChip::SetChipId(unsigned int aChipId)
{m_chipId = aChipId;}

void TThresholdResultChip::SetVPulseL(int aVPulseL)
{m_vPulseL = aVPulseL;}    

void TThresholdResultChip::SetVPulseH(int aVPulseH)
{m_vPulseH = aVPulseH;}    

void TThresholdResultChip::SetVPulseStep(int aVPulseStep)
{m_vPulseStep = aVPulseStep;} 

void TThresholdResultChip::SetNMask(int aNMask)
{m_nMask = aNMask;}
  
void TThresholdResultChip::SetCounterPixelsNoHits(int aCounterPixelsNoHits)
{m_counterPixelsNoHits = aCounterPixelsNoHits;}

void TThresholdResultChip::SetCounterPixelsStuck(int aCounterPixelsStuck)
{m_counterPixelsStuck = aCounterPixelsStuck;}

void TThresholdResultChip::SetCounterPixelsNoThreshold(int aCounterPixelsNoThreshold)
{m_counterPixelsNoThreshold = aCounterPixelsNoThreshold;}
  
void TThresholdResultChip::SetThresholdMean(double aThresholdMean){m_thresholdMean =  aThresholdMean;}

void TThresholdResultChip::SetThresholdStdDev(double aThresholdStdDev)
{m_thresholdStdDev = aThresholdStdDev;}

void TThresholdResultChip::SetNoiseMean(double aNoiseMean)
{m_noiseMean =  aNoiseMean;}

void TThresholdResultChip::SetNoiseStdDev(double aNoiseStdDev)
{m_noiseStdDev  = aNoiseStdDev;}

unsigned int TThresholdResultChip::GetBoardIndex()
{return m_boardIndex;} 

unsigned int TThresholdResultChip::GetDataReceiver()
{return m_dataReceiver;}

unsigned int TThresholdResultChip::GetChipId()
{return m_chipId;}

int TThresholdResultChip::GetVPulseL()
{return m_vPulseL;}

int TThresholdResultChip::GetVPulseH()
{return m_vPulseH;}

int TThresholdResultChip::GetVPulseStep()
{return m_vPulseStep;}

int TThresholdResultChip::GetNMask()
{return m_nMask;}

int TThresholdResultChip::GetCounterPixelsNoHits()
{return m_counterPixelsNoHits;}

int TThresholdResultChip::GetCounterPixelsStuck()
{return m_counterPixelsStuck;}

int TThresholdResultChip::GetCounterPixelsNoThreshold()
{return m_counterPixelsNoThreshold;}
  
double TThresholdResultChip::GetThresholdMean()
{return m_thresholdMean;}

double TThresholdResultChip::GetThresholdStdDev()
{return m_thresholdStdDev;}

double TThresholdResultChip::GetNoiseMean()
{return m_noiseMean;}

double TThresholdResultChip::GetNoiseStdDev()
{return m_noiseStdDev;}

// ================================
// ================================

TThresholdResult::TThresholdResult (): TScanResult () {;}

TThresholdResult::~TThresholdResult () {;}

void TThresholdResult::SetFileHicResult(FILE* aFileName)
{m_fileHicResult = aFileName;}

void TThresholdResult::SetFilePixelByPixelResult(FILE* aFileName)
{m_filePixelByPixelResult = aFileName;}

void TThresholdResult::SetFileStuckPixels(FILE* aFileName)
{m_fileStuckPixels = aFileName;}

FILE* TThresholdResult::GetFileHicResult()
{return m_fileHicResult;}

FILE* TThresholdResult::GetFilePixelByPixelResult()
{return m_filePixelByPixelResult;}

FILE* TThresholdResult::GetFileStuckPixels()
{return m_fileStuckPixels;}

// ================================
// ================================


TThresholdAnalysis::TThresholdAnalysis(std::deque<TScanHisto> *aScanHistoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(aScanHistoQue, aScan, aScanConfig, aMutex)  
{
  
  // It is pulse amplitude, not charge, yet.
  m_startPulseAmplitude = m_config->GetChargeStart();
  m_stopPulseAmplitude  = m_config->GetChargeStop();
  m_stepPulseAmplitude  = m_config->GetChargeStep();
  m_nPulseInj           = m_config->GetNInj();
  
  m_fDoDumpRawData = true;
  m_fDoFit         = true;
  
  m_resultThreshold = new TThresholdResult();
  
  int nPoints= (m_stopPulseAmplitude - m_startPulseAmplitude)/ m_stepPulseAmplitude;
  
  hSuperMeanA    = new TH1D("","Mean;Threshold [e]; entries [a.u.]",1000,0,(1+(m_stopPulseAmplitude - m_startPulseAmplitude) )*m_electronPerDac);/*H*/
  hSuperNoiseA   = new TH1D("","Noise;Noise [e];entries [a.u.]",100,0,100);/*H*/
  hSuperRedChi2A = new TH1D("","RedChi2;RedChi2 [a.u.];entries [a.u.]",100,0,1000);/*H*/
  hSuperStatusA  = new TH1D("","Status;Fit status error code [a.u.];entries [a.u.]",10,0,10);/*H*/
  
  hSuperDummyA = new TH2D("","; Pulse Amplitude[e]; Entries[a.u.]",/*H*/
			  nPoints+1, 0, (1+(m_stopPulseAmplitude - m_startPulseAmplitude) )*m_electronPerDac,/*H*/
			  m_nPulseInj+10, 0, m_nPulseInj+10);/*H*/
  
  hSuperNPointsVsStatusA= new TH2D("","NPointsVsStatus;Fit Status[a.u.];Points[a.u.]",/*H*/
				   10,0,10,/*H*/
				   nPoints+10,0,nPoints+10);/*H*/
  
  hSuperNPointsVsChi2A = new TH2D("","NPointsVsChi2;RedChi2[a.u.];Points[a.u.]",
				  100,0,1000,/*H*/
				  nPoints+10,0,nPoints+10);/*H*/
  
  
  hSuperMeanB    = new TH1D("","Mean;Threshold [e]; entries [a.u.]",1000,0,(1+(m_stopPulseAmplitude - m_startPulseAmplitude) )*m_electronPerDac);
  hSuperNoiseB   = new TH1D("","Noise;Noise [e];entries [a.u.]",100,0,100);/*H*/
  hSuperRedChi2B = new TH1D("","RedChi2;RedChi2 [a.u.];entries [a.u.]",100,0,1000);/*H*/
  hSuperStatusB  = new TH1D("","Status;Fit status error code [a.u.];entries [a.u.]",10,0,10);/*H*/
  
  hSuperDummyB = new TH2D("","; Pulse Amplitude[e]; Entries[a.u.]",/*H*/
			  nPoints+1, 0, (1+(m_stopPulseAmplitude - m_startPulseAmplitude)*m_electronPerDac ),/*H*/
			  m_nPulseInj+10, 0, m_nPulseInj+10);/*H*/
  
  hSuperNPointsVsStatusB= new TH2D("","NPointsVsStatus;Fit Status[a.u.];Points[a.u.]",/*H*/
				   10,0,10,/*H*/
				   nPoints+10,0,nPoints+10);/*H*/
  
  hSuperNPointsVsChi2B = new TH2D("","NPointsVsChi2;RedChi2[a.u.];Points[a.u.]",
				  100,0,1000,/*H*/
				  nPoints+10,0,nPoints+10);/*H*/
  
  
  hSuperMeanC = new TH1D("","Mean;Threshold [e]; entries [a.u.]",1000,0,(1+(m_stopPulseAmplitude - m_startPulseAmplitude) )*m_electronPerDac);/*H*/
  hSuperNoiseC = new TH1D("","Noise;Noise [e];entries [a.u.]",100,0,100);/*H*/
  hSuperRedChi2C = new TH1D("","RedChi2;RedChi2 [a.u.];entries [a.u.]",100,0,1000);/*H*/
  hSuperStatusC  = new TH1D("","Status;Fit status error code [a.u.];entries [a.u.]",10,0,10);/*H*/
  
  hSuperDummyC = new TH2D("","; Pulse Amplitude[e]; Entries[a.u.]",/*H*/
			  nPoints+1, 0, (1+(m_stopPulseAmplitude - m_startPulseAmplitude) )*m_electronPerDac,/*H*/
			  m_nPulseInj+10, 0, m_nPulseInj+10);/*H*/
  
  hSuperNPointsVsStatusC = new TH2D("","NPointsVsStatus;Fit Status[a.u.];Points[a.u.]",/*H*/
				    10,0,10,/*H*/
				    nPoints+10,0,nPoints+10);/*H*/
  
  hSuperNPointsVsChi2C = new TH2D("","NPointsVsChi2;RedChi2[a.u.];Points[a.u.]",
				  100,0,1000,/*H*/
				  nPoints+10,0,nPoints+10);/*H*/
  
}

//TODO: Implement HasData.
bool TThresholdAnalysis::HasData(TScanHisto &histo, common::TChipIndex idx, int col) 
{
  return true;
}


bool TThresholdAnalysis::CheckPixelNoHits(TGraph* aGraph)
{
  
  for (int itrPoint=0; itrPoint<aGraph->GetN(); itrPoint++){
    double x =0;
    double y =0;
    aGraph->GetPoint(itrPoint, x, y);
    if (y!=0){return false;} 
  }
  
  return true;
}

bool TThresholdAnalysis::CheckPixelStuck(TGraph* aGraph)
{
  double y_dummy =0;
  for (int itrPoint=0; itrPoint<aGraph->GetN(); itrPoint++){
    double x =0;
    double y =0;
    aGraph->GetPoint(itrPoint, x, y);
    if (y<0.5*m_nPulseInj){return false;} 
  }
  
  return true;
}

double ErrorFunc(double* x, double* par)
{
  double y = par[0]+par[1]*TMath::Erf( (x[0]-par[2]) / par[3] );
  return y;
}

common::TErrFuncFitResult TThresholdAnalysis::DoFit(TGraph* aGraph)
{
  TF1* fitfcn = new TF1("fitfcn", 
			ErrorFunc,
			m_startPulseAmplitude*m_electronPerDac,
			m_stopPulseAmplitude*m_electronPerDac,
			4);
  // y@50%.
  fitfcn->SetParameter(0,0.5*m_nPulseInj); 
  // 0.5 of max. amplitude.
  fitfcn->SetParameter(1,0.5*m_nPulseInj); 
  // x@50%.
  fitfcn->SetParameter(2,0.5*(m_stopPulseAmplitude - m_startPulseAmplitude)*m_electronPerDac);
  // slope of s-curve.
  fitfcn->SetParameter(3,0.5);
  
  aGraph->Fit("fitfcn","RQ");
  
  common::TErrFuncFitResult fitResult_dummy;
  fitResult_dummy.status    = aGraph->Fit("fitfcn","RQ");
  fitResult_dummy.threshold = fitfcn->GetParameter(2);
  fitResult_dummy.noise     = fitfcn->GetParameter(3);
  fitResult_dummy.redChi2   = fitfcn->GetChisquare()/fitfcn->GetNDF();
  
  // std::cout << fitResult_dummy.threshold << "\t"
  //  	    << fitResult_dummy.noise << "\t"
  //  	    << fitResult_dummy.redChi2 << "\t"
  //  	    << fitResult_dummy.status << "\t"
  //  	    << std::endl;
  
  delete fitfcn; 
  
  return fitResult_dummy;
}

void TThresholdAnalysis::Initialize()
{
  
  // Retrieving HistoMap from TThresholdScan, after it is initialized.
  // Creating output files.
  // Initializing TThresholdResult variables.
  // Initializing counters.
  
  while (!m_scan->IsRunning()){sleep(1);}
  
  std::cout << "Initializing " << m_analisysName << std::endl;
  
  std::string fileNameDummy;
  fileNameDummy  = m_analisysName;
  fileNameDummy += "-";
  fileNameDummy += m_config->GetfNameSuffix();
  fileNameDummy += "-HicResult";
  fileNameDummy += ".dat";
  
  m_resultThreshold->SetFileHicResult(fopen(fileNameDummy.c_str(),"w"));

  fileNameDummy  = m_analisysName;
  fileNameDummy += "-";
  fileNameDummy += m_config->GetfNameSuffix();
  fileNameDummy += "-PixelByPixel";
  fileNameDummy += ".dat"; 
  
  m_resultThreshold->SetFilePixelByPixelResult(fopen(fileNameDummy.c_str(),"w"));
  
  fileNameDummy  = m_analisysName;
  fileNameDummy += "-";
  fileNameDummy += m_config->GetfNameSuffix();
  fileNameDummy += "-StuckPixels";
  fileNameDummy += ".dat"; 
  
  m_resultThreshold->SetFileStuckPixels(fopen(fileNameDummy.c_str(),"w"));
    
  TScanHisto histoDummy = m_scan->GetTScanHisto();
  
  std::map<int, THisto> histoMap_dummy= histoDummy.GetHistoMap();
  
  for (std::map<int, THisto>::iterator itr=histoMap_dummy.begin();
       itr!=histoMap_dummy.end(); 
       ++itr) {
    
    common::TChipIndex chipIndexDummy = common::GetChipIndex(itr->first);
    
    m_chipList.push_back(chipIndexDummy);
    
    TThresholdResultChip* dummyResultChip = new TThresholdResultChip();
    dummyResultChip->SetCounterPixelsNoHits(0);
    dummyResultChip->SetCounterPixelsStuck(0);
    dummyResultChip->SetCounterPixelsNoThreshold(0);
    
    m_resultThreshold->AddChipResult(itr->first,
    				     dummyResultChip);
  }
  
  std::pair<int,common::TStatVar> pairDummy;
  for (std::map<int, THisto>::iterator itr=histoMap_dummy.begin();
       itr!=histoMap_dummy.end(); 
       ++itr) {
    
    common::TStatVar thresholdDummy;
    thresholdDummy.sum=0;
    thresholdDummy.sum2=0;
    thresholdDummy.entries=0;
    pairDummy = std::make_pair(itr->first,thresholdDummy);
    m_threshold.insert(pairDummy);
    
    common::TStatVar noiseDummy;
    noiseDummy.sum=0;
    noiseDummy.sum2=0;
    noiseDummy.entries=0;
    pairDummy = std::make_pair(itr->first,noiseDummy);
    m_noise.insert(pairDummy);
  } 
  
}

void TThresholdAnalysis::Run() 
{
  
  while (m_histoQue->size() == 0){sleep(1);}
  
  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    
    if (m_histoQue->size()<= 0){usleep(300);continue;}
    
    while (!(m_mutex->try_lock()));
    
    TScanHisto scanHisto = m_histoQue->front();      
    
    m_histoQue->pop_front();
    m_mutex->unlock();
    
    int row = scanHisto.GetIndex();
    
    std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size() << std::endl;
    
    for (int iChip = 0; iChip < m_chipList.size(); iChip++) {
      
      for (int iCol = 0; iCol < common::nCols; iCol ++) {
	
     	TGraph* gDummy = new TGraph();
	
    	int pulseRangeDummy = ((float)abs( m_startPulseAmplitude - m_stopPulseAmplitude))/ m_stepPulseAmplitude;
	
    	for (int iPulse = 0; iPulse < pulseRangeDummy; iPulse++) {
	  
    	  int entries =(int)scanHisto(m_chipList.at(iChip), 
    				      iCol, 
    				      iPulse);
	  
    	  gDummy->SetPoint(gDummy->GetN(),
    			   iPulse*m_electronPerDac,
    			   entries);
	  
	  if(m_fDoDumpRawData){;}
	  
	} // end loop over iPulse.
	
	if (gDummy->GetN()==0){ delete gDummy;continue;}
	
	common::TChipIndex dummyChipIndex = m_chipList.at(iChip);
	
	TThresholdResultChip* resultDummy = (TThresholdResultChip*) m_resultThreshold->GetChipResult(dummyChipIndex);
	
	bool fPixelNoHits= CheckPixelNoHits(gDummy);
	bool fPixelStuck = CheckPixelStuck (gDummy);
	
	if (fPixelNoHits){
	  int dummyCounter = resultDummy->GetCounterPixelsNoHits();
	  resultDummy->SetCounterPixelsNoHits(dummyCounter+1);
	} else if (fPixelStuck){
	  int dummyCounter = resultDummy->GetCounterPixelsStuck();
	  resultDummy->SetCounterPixelsStuck(dummyCounter+1);
	} else if (m_fDoFit){
	  
	  common::TErrFuncFitResult fitResult;
	  fitResult=DoFit(gDummy);
	  
	  fprintf(m_resultThreshold->GetFilePixelByPixelResult(),
		  "%d %d %d %d %d %f %f %f\n", 
		  dummyChipIndex.boardIndex,
		  dummyChipIndex.dataReceiver,
		  dummyChipIndex.chipId,
		  iCol,row,
		  fitResult.threshold,
		  fitResult.noise,
		  fitResult.redChi2);
	  
	  // MB - NEED TO SELECT GOOD FIT.
	  // if (fitResult.status!=4){continue;}
	  int maxRedChi2 = 5;// From MK.
	  
	  for (int itr=0; itr<gDummy->GetN(); itr++) {
	    
	    double x =0;
	    double y =0;
	    
	    gDummy->GetPoint(itr,x,y);
	    
	    hSuperDummyA->Fill(x,y);/*H*/
	    if (fitResult.status==0){
	      hSuperDummyB->Fill(x,y);/*H*/
	    }else if (fitResult.status==4){
	      hSuperDummyC->Fill(x,y);/*H*/
	    }
	    
	  }
	  
	  int intIndexDummy = common::GetChipIntIndex(dummyChipIndex);
	  m_threshold.at(intIndexDummy).sum+=fitResult.threshold;
	  m_threshold.at(intIndexDummy).sum2+=pow(fitResult.threshold,2);
	  m_threshold.at(intIndexDummy).entries+=1;
	  m_noise.at(intIndexDummy).sum+=fitResult.noise;
	  m_noise.at(intIndexDummy).sum2+=pow(fitResult.noise,2);
	  m_noise.at(intIndexDummy).entries+=1;
	  
	  hSuperMeanA->Fill(fitResult.threshold);/*H*/
	  hSuperNoiseA->Fill(fitResult.noise);/*H*/
	  hSuperRedChi2A->Fill(fitResult.redChi2);/*H*/
	  hSuperStatusA->Fill(fitResult.status);/*H*/
	  
	  hSuperNPointsVsStatusA->Fill(fitResult.status, /*H*/
				       gDummy->GetN());/*H*/
	  hSuperNPointsVsChi2A->Fill(fitResult.redChi2, 
				     gDummy->GetN());/*H*/
	  
	  if (fitResult.status==0) {
	    hSuperMeanB->Fill(fitResult.threshold);/*H*/
	    hSuperNoiseB->Fill(fitResult.noise);/*H*/
	    hSuperRedChi2B->Fill(fitResult.redChi2);/*H*/
	    hSuperStatusB->Fill(fitResult.status);/*H*/
	    
	    hSuperNPointsVsStatusB->Fill(fitResult.status, /*H*/
					 gDummy->GetN());/*H*/
	    hSuperNPointsVsChi2B->Fill(fitResult.redChi2, 
				       gDummy->GetN());/*H*/
	  } else if (fitResult.status==4) {
	    hSuperMeanC->Fill(fitResult.threshold);/*H*/
	    hSuperNoiseC->Fill(fitResult.noise);/*H*/
	    hSuperRedChi2C->Fill(fitResult.redChi2);/*H*/
	    hSuperStatusC->Fill(fitResult.status);/*H*/
	    
	    hSuperNPointsVsStatusC->Fill(fitResult.status, /*H*/
					 gDummy->GetN());/*H*/
	    hSuperNPointsVsChi2C->Fill(fitResult.redChi2, 
				       gDummy->GetN());/*H*/
	  }
	  
	}
	
	delete gDummy;
      } // end loop over iCol.
      
    } // end loop over iChip.
  } // end of "while" on m_scan and m_scanHistoQue.
  
}

void TThresholdAnalysis::Finalize()
{
  
  std::cout << "Finalizing " 
	    << m_analisysName 
	    << std::endl;
  
  // Sanity check.
  // if ( m_resultChip.size()!=m_threshold.size() || 
  //      m_resultChip.size()!=m_noise.size()){ 
  //   std::cout<< "ERROR in " 
  //  	     << m_analisysName  
  //  	     << "!!!"
  //  	     <<std::endl; exit(EXIT_FAILURE);;
  // }
  
  for (std::map<int,common::TStatVar>::iterator itr=m_threshold.begin();
       itr!=m_threshold.end(); 
       ++itr) {
    
    if (itr->second.entries==0){continue;}
    
    double mean   = itr->second.sum/itr->second.entries;
    double stdDev = sqrt((itr->second.sum2/itr->second.entries) 
    			 - pow(mean,2) );
    
    itr->second.mean   = mean;
    itr->second.stdDev = stdDev;
  }
  
  for (std::map<int,common::TStatVar>::iterator itr=m_noise.begin();
       itr!=m_noise.end(); 
       ++itr) {
    
    if (itr->second.entries==0){continue;}
    
    double mean   = itr->second.sum/itr->second.entries;
    double stdDev = sqrt((itr->second.sum2/itr->second.entries) 
    			 - pow(mean,2) );
    
    itr->second.mean   = mean;
    itr->second.stdDev = stdDev;
    
  }
  
  for (int iChip=0; iChip < m_chipList.size(); iChip++) {
    
    common::TChipIndex dummyChipIndex = m_chipList.at(iChip);
    
    TThresholdResultChip* resultDummy = (TThresholdResultChip*) m_resultThreshold->GetChipResult(dummyChipIndex);
    
    int dummyIntIndex = common::GetChipIntIndex(dummyChipIndex);
    
    resultDummy->SetThresholdMean(m_threshold.at(dummyIntIndex).mean);
    resultDummy->SetThresholdStdDev(m_threshold.at(dummyIntIndex).stdDev);
    resultDummy->SetNoiseMean(m_noise.at(dummyIntIndex).mean);
    resultDummy->SetNoiseStdDev(m_noise.at(dummyIntIndex).stdDev);
    
    fprintf( m_resultThreshold->GetFileHicResult(), 
	     "%d %d %d %f %f %f %f",
	     dummyChipIndex.boardIndex,
	     dummyChipIndex.dataReceiver,
	     dummyChipIndex.chipId,
	     resultDummy->GetCounterPixelsNoHits(),
	     resultDummy->GetCounterPixelsStuck(),
	     resultDummy->GetCounterPixelsNoThreshold(),
	     m_threshold.at(dummyIntIndex).mean,
	     m_threshold.at(dummyIntIndex).stdDev,
	     m_noise.at(dummyIntIndex).mean,
	     m_noise.at(dummyIntIndex).stdDev
	     );
  }
  
  fclose(m_resultThreshold->GetFileHicResult());
  fclose(m_resultThreshold->GetFilePixelByPixelResult());
  fclose(m_resultThreshold->GetFileStuckPixels());
  
  TPaveText* p0;
  
  TCanvas* c0 = new TCanvas();/*H*/
  c0->cd();/*H*/
  hSuperDummyA->Draw("COLZ");/*H*/
  c0->SaveAs("hSuperDummyA.pdf");/*H*/
  
  c0->cd();/*H*/
  hSuperDummyB->Draw("COLZ");/*H*/
  c0->SaveAs("hSuperDummyB.pdf");/*H*/
  
  c0->cd();/*H*/
  hSuperDummyC->Draw("COLZ");/*H*/
  c0->SaveAs("hSuperDummyC.pdf");/*H*/
  
  // Save plots.
  TFile* file_output =new TFile( Form("TThresholdAnalysis-%s.root",m_config->GetfNameSuffix()),"RECREATE");
  file_output->cd();
  
  hSuperMeanA->Write("MeanA");/*H*/
  hSuperNoiseA->Write("NoiseA");/*H*/
  hSuperRedChi2A->Write("RedChi2A");/*H*/
  hSuperStatusA->Write("StatusA");/*H*/
  hSuperDummyA->Write("hSuperDummyA");/*H*/
  hSuperNPointsVsStatusA->Write("NPointsVsStatusA");/*H*/
  hSuperNPointsVsChi2A->Write("NPointsVsChi2A");/*H*/
  
  hSuperMeanB->Write("MeanB");/*H*/
  hSuperNoiseB->Write("NoiseB");/*H*/
  hSuperRedChi2B->Write("RedChi2B");/*H*/
  hSuperStatusB->Write("StatusB");/*H*/
  hSuperDummyB->Write("hSuperDummyB");/*H*/
  hSuperNPointsVsStatusB->Write("NPointsVsStatusB");/*H*/
  hSuperNPointsVsChi2B->Write("NPointsVsChi2B");/*H*/
  
  hSuperMeanC->Write("MeanC");/*H*/
  hSuperNoiseC->Write("NoiseC");/*H*/
  hSuperRedChi2C->Write("RedChi2C");/*H*/
  hSuperStatusC->Write("StatusC");/*H*/
  hSuperDummyC->Write("hSuperDummyC");/*H*/
  hSuperNPointsVsStatusC->Write("NPointsVsStatusC");/*H*/
  hSuperNPointsVsChi2C->Write("NPointsVsChi2C");/*H*/
  
  file_output->Print();
  file_output->Close();
  
}

