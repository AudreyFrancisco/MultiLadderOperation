// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e. fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules, 
// all chip accesses should be done with a loop over all elements of the chip vector. 
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);  
// For an example how to access board-specific functions see the power off at the end of main. 
//
// The functions that should be modified for the specific test are configureChip() and main()


#include <unistd.h>
#include <string.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "TSetup.h"


// !!! NOTE: Scan parameters are now set via Config file

TBoardType fBoardType;
std::vector <TReadoutBoard *> fBoards;
std::vector <TAlpide *>       fChips;
TConfig *fConfig;

int myNTriggers;
int myMaskStages;
int myPixPerRegion;

int myChargeStart;
int myChargeStop;
int myChargeStep;  // currently unused

int fEnabled = 0;  // variable to count number of enabled chips; leave at 0

int HitData     [16][100][512][1024];
int ChargePoints[100];
int ievt = 0;


void InitScanParameters() {
  myMaskStages   = fConfig->GetScanConfig()->GetParamValue("NMASKSTAGES");
  myPixPerRegion = fConfig->GetScanConfig()->GetParamValue("PIXPERREGION");
  myNTriggers    = fConfig->GetScanConfig()->GetParamValue("NINJ");
  myChargeStart  = fConfig->GetScanConfig()->GetParamValue("CHARGESTART");
  myChargeStop   = fConfig->GetScanConfig()->GetParamValue("CHARGESTOP");
  myChargeStep   = fConfig->GetScanConfig()->GetParamValue("CHARGESTEP");
}


void ClearHitData() {
  for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
    ChargePoints[icharge-myChargeStart] = icharge;
    for (int ichip = 0; ichip < 16; ichip ++) {
      for (int icol = 0; icol < 512; icol ++) {
        for (int iaddr = 0; iaddr < 1024; iaddr ++) {
          HitData[ichip][icharge-myChargeStart][icol][iaddr] = 0;
        }
      }
    }
  }
}


void CopyHitData(std::vector <TPixHit> *Hits, int charge) {
  for (unsigned int ihit = 0; ihit < Hits->size(); ihit ++) {
    HitData[Hits->at(ihit).chipId][charge-myChargeStart][Hits->at(ihit).dcol + Hits->at(ihit).region * 16][Hits->at(ihit).address] ++;
  }
  Hits->clear();
}


void WriteDataToFile (const char *fName, bool Recreate) {
  char  fNameChip[100];
  FILE *fp;

  char fNameTemp[100];
  sprintf(fNameTemp,"%s", fName);
  strtok (fNameTemp, "."); 
  bool  HasData;

  for (unsigned int ichip = 0; ichip < fChips.size(); ichip ++) {
    int chipId = fChips.at(ichip)->GetConfig()->GetChipId() & 0xf;
    if (!fChips.at(ichip)->GetConfig()->IsEnabled()) continue;  // write files only for enabled chips
    if (fChips.size() > 1) {
      sprintf(fNameChip, "%s_Chip%d.dat", fNameTemp, chipId);
    }
    else {
      sprintf(fNameChip, "%s.dat", fNameTemp);
    }
    if (Recreate) fp = fopen(fNameChip, "w");
    else          fp = fopen(fNameChip, "a");

    for (int icol = 0; icol < 512; icol ++) {
      for (int iaddr = 0; iaddr < 1024; iaddr ++) {
        HasData = false;
        for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
          if (HitData[chipId][icharge - myChargeStart][icol][iaddr] > 0) HasData = true;
        }
      
        if (HasData) {
          for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
            fprintf(fp, "%d %d %d %d\n", icol, iaddr, icharge, HitData[chipId][icharge - myChargeStart][icol][iaddr]);
	  }
        }
      }
    }
    fclose (fp);
  }
}


int configureFromu(TAlpide *chip) {
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x20);            // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetParamValue("STROBEDURATION"));  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, chip->GetConfig()->GetParamValue("STROBEDELAYCHIP"));   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, chip->GetConfig()->GetParamValue("PULSEDURATION"));   // fromu pulsing 2: pulse length 
  return 0;
}


int configureChip(TAlpide *chip) {
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);

  AlpideConfig::ConfigureCMU (chip);

  return 0;
}


void WriteScanConfig(const char *fName, TAlpide *chip, TReadoutBoardDAQ *daqBoard) {
  char Config[1000];
  FILE *fp = fopen(fName, "w");

  chip     -> DumpConfig("", false, Config);
  std::cout << Config << std::endl;
  fprintf(fp, "%s\n", Config);
  if(daqBoard) daqBoard -> DumpConfig("", false, Config);
  fprintf(fp, "%s\n", Config);
  std::cout << Config << std::endl;

  fprintf(fp, "\n");

  fprintf(fp, "NTRIGGERS %i\n", myNTriggers);
  fprintf(fp, "MASKSTAGES %i\n", myMaskStages);

  fprintf(fp, "CHARGESTART %i\n", myChargeStart);
  fprintf(fp, "CHARGESTOP %i\n", myChargeStop);

    
  fclose(fp);
}



void scan() {   
  unsigned char         buffer[1024*4000]; 
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  int                   nBad = 0, skipped = 0;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;
  std::vector<int> myVPULSEH;

  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

  Hits->clear();
  if (myMOSAIC) {
    myMOSAIC->StartRun();
  }

  for (unsigned int i = 0; i < fChips.size(); i++) { //Read VPULSEH from Config and save it at vector temporarily
    myVPULSEH.push_back(fChips.at(i)->GetConfig()->GetParamValue("VPULSEH"));
  }
 
  for (int istage = 0; istage < myMaskStages; istage ++) {
    std::cout << "Mask stage " << istage << std::endl;
    for (unsigned int i = 0; i < fChips.size(); i ++) {
      if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
      AlpideConfig::ConfigureMaskStage(fChips.at(i), myPixPerRegion, istage);
    }

    for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
      //std::cout << "Charge = " << icharge << std::endl;
      for (unsigned int i = 0; i < fChips.size(); i ++) {
        if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
        fChips.at(i)->WriteRegister (Alpide::REG_VPULSEL, myVPULSEH[i] - icharge);  //Automatically matches max pulse = VPULSEH in config
      }
      fBoards.at(0)->Trigger(myNTriggers);

//std::cout << " >>>>" << myMOSAIC->GetConfig()->GetPollingDataTimeout() << endl; 

      int itrg = 0;
      int trials = 0;
      while(itrg < myNTriggers * fEnabled) {
        if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
          usleep(1000); // Increment from 100us
          trials ++;
          if (trials == 10) {
        	std::cout << "Reached 10 timeouts, giving up on this event" << std::endl;
            itrg = myNTriggers * fEnabled;
            skipped ++;
            trials = 0;
          }
          continue;
        }
        else {
          // decode DAQboard event
          //if (ievt < 20) {
	  //   FILE *fDebug = fopen ("DebugData.dat", "a");
          //  fprintf(fDebug, "Normal event:\n");
          //  for (int iByte=0; iByte<n_bytes_data; ++iByte) {
          //    fprintf (fDebug, "%02x ", (int) buffer[iByte]);
          //  }
          //  fprintf(fDebug, "\nFull Event:\n"); 
          //  for (int ibyte = 0; ibyte < fDebugBuffer.size(); ibyte ++) {
          //    fprintf (fDebug, "%02x ", (int) fDebugBuffer.at(ibyte));
          //  }
          //  fprintf(fDebug, "\n\n");
          //  fclose (fDebug);
          //  ievt ++;
	  // }
          BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
          // decode Chip event
          int n_bytes_chipevent=n_bytes_data-n_bytes_header;//-n_bytes_trailer;
          if (boardInfo.eoeCount < 2) n_bytes_chipevent -= n_bytes_trailer;
          if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits)) {
   	    	std::cout << "Found bad event, length = " << n_bytes_chipevent << std::endl;
	    	nBad ++;
            if (nBad > 10) continue;
	    	FILE *fDebug = fopen ("DebugData.dat", "a");
            fprintf(fDebug, "Bad event:\n");
            for (int iByte=0; iByte<n_bytes_data + 1; ++iByte) {
              fprintf (fDebug, "%02x ", (int) buffer[iByte]);
            }
            fprintf(fDebug, "\nFull Event:\n"); 
            for (unsigned int ibyte = 0; ibyte < fDebugBuffer.size(); ibyte ++) {
              fprintf (fDebug, "%02x ", (int) fDebugBuffer.at(ibyte));
            }
            fprintf(fDebug, "\n\n");
            fclose (fDebug);
	  	  }
          itrg++;
        }
      }
	  //usleep(100);

      //std::cout << "Number of hits: " << Hits->size() << std::endl;
      CopyHitData(Hits, icharge);
    }
  }

  if (myMOSAIC) {
    myMOSAIC->StopRun();
  }
  std::cout << "Scan finished, skipped " << skipped << " points." << std::endl;
}


int main(int argc, char** argv) {

  decodeCommandParameters(argc, argv);
  initSetup(fConfig,  &fBoards,  &fBoardType, &fChips);
  InitScanParameters();
    char Suffix[20], fName[100]; //Config[1000];

  ClearHitData();
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (unsigned int i = 0; i < fChips.size(); i ++) {
      if (fChips.at(i)->GetConfig()->IsEnabled()) {
        fEnabled ++;        
        configureChip (fChips.at(i));
      }
      else if (fChips.at(i)->GetConfig()->HasEnabledSlave()) {
	AlpideConfig::BaseConfigPLL(fChips.at(i));
      }
    }

    std::cout << "Found " << fEnabled << " enabled chips." << std::endl;
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

    // put your test here... 
    if (fBoards.at(0)->GetConfig()->GetBoardType() == kBOARD_MOSAIC) {
      fBoards.at(0)->SetTriggerConfig (true, true, 
                                       fBoards.at(0)->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                                       fBoards.at(0)->GetConfig()->GetParamValue("PULSEDELAY"));
      fBoards.at(0)->SetTriggerSource (kTRIG_INT);
    }
    else if (fBoards.at(0)->GetConfig()->GetBoardType() == kBOARD_DAQ) {
      // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
      // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
      fBoards.at(0)->SetTriggerConfig (true, false, 
                                       0,
                                       2 * fBoards.at(0)->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
      fBoards.at(0)->SetTriggerSource (kTRIG_EXT);
    }

    scan();

    sprintf(fName, "Data/ThresholdScan_%s.dat", Suffix);
    WriteDataToFile (fName, true);
    sprintf(fName, "Data/ScanConfig_%s.cfg", Suffix);

    if (myDAQBoard) {
      WriteScanConfig (fName, fChips.at(0), myDAQBoard);
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
