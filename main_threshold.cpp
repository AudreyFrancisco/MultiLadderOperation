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
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include "TConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"


//int myVCASN   = 57;
//int myITHR    = 51;
//int myVCASN2  = 64;
//int myVCLIP   = 0;
//int myVRESETD = 147;

int myStrobeLength = 80;      // strobe length in units of 25 ns
int myStrobeDelay  = 0;
int myPulseLength  = 500;

int myPulseDelay   = 40;
int myNTriggers    = 50;
int myMaskStages   = 82;    // full: 4096

int myChargeStart  = 0;
//int myChargeStop   = 50;   // if > 100 points, increase array sizes
int myChargeStop   = 90;   // if > 100 points, increase array sizes

int HitData     [100][512][1024];
int ChargePoints[100];


void ClearHitData() {
  for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
    ChargePoints[icharge-myChargeStart] = icharge;
    for (int icol = 0; icol < 512; icol ++) {
      for (int iaddr = 0; iaddr < 1024; iaddr ++) {
        HitData[icharge-myChargeStart][icol][iaddr] = 0;
      }
    }
  }
}


void CopyHitData(std::vector <TPixHit> *Hits, int charge) {
  for (int ihit = 0; ihit < Hits->size(); ihit ++) {
    HitData[charge-myChargeStart][Hits->at(ihit).dcol + Hits->at(ihit).region * 16][Hits->at(ihit).address] ++;
  }
  Hits->clear();
}


void WriteDataToFile (const char *fName, bool Recreate) {
  FILE *fp;
  bool  HasData;
  if (Recreate) fp = fopen(fName, "w");
  else          fp = fopen(fName, "a");

  for (int icol = 0; icol < 512; icol ++) {
    for (int iaddr = 0; iaddr < 1024; iaddr ++) {
      HasData = false;
      for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
        if (HitData[icharge - myChargeStart][icol][iaddr] > 0) HasData = true;
      }
      
      if (HasData) {
        for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
          fprintf(fp, "%d %d %d %d\n", icol, iaddr, icharge, HitData[icharge - myChargeStart][icol][iaddr]);
	}
      }
    }
  }
  fclose (fp);
}


int configureFromu(TAlpide *chip) {
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x20);            // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  myStrobeLength);  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay);   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse length 
}


// setting of mask stage during scan
int configureMaskStage(TAlpide *chip, int istage) {
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   true);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);

  for (int icol = 0; icol < 1024; icol += 8) {
    AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_MASK,   false, istage % 512, icol + istage / 512);
    AlpideConfig::WritePixRegSingle (chip, Alpide::PIXREG_SELECT, true,  istage % 512, icol + istage / 512);   
  }

}


int configureChip(TAlpide *chip) {
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);
  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x21); // strobed readout mode


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

  fprintf(fp, "\n", Config);

  fprintf(fp, "NTRIGGERS %i\n", myNTriggers);
  fprintf(fp, "MASKSTAGES %i\n", myMaskStages);

  fprintf(fp, "CHARGESTART %i\n", myChargeStart);
  fprintf(fp, "CHARGESTOP %i\n", myChargeStop);

    
  fclose(fp);
}



void scan() {   
  unsigned char         buffer[1024*4000]; 
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;


  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

  if (myMOSAIC) {
    myMOSAIC->StartRun();
  }

  for (int istage = 0; istage < myMaskStages; istage ++) {
    std::cout << "Mask stage " << istage << std::endl;
    for (int i = 0; i < fChips.size(); i ++) {
      configureMaskStage (fChips.at(i), istage);
    }
    //configureMaskStage (fChips.at(0), istage);

    for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
      //std::cout << "Charge = " << icharge << std::endl;
      fChips.at(0)->WriteRegister (Alpide::REG_VPULSEL, 170 - icharge);
      fBoards.at(0)->Trigger(myNTriggers);

      int itrg = 0;
      while(itrg < myNTriggers) {
        if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
          usleep(100);
          continue;
        }
        else {
          // decode DAQboard event
          BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
          // decode Chip event
          int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
          AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits);

          itrg++;
        }
      } 
      //std::cout << "Number of hits: " << Hits->size() << std::endl;
      CopyHitData(Hits, icharge);
    }
  }

  if (myMOSAIC) {
    myMOSAIC->StopRun();
  }
}


int main() {
  initSetup();

  char Suffix[20], fName[100], Config[1000];

  ClearHitData();
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (int i = 0; i < fChips.size(); i ++) {
      configureChip (fChips.at(i));
    }

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

    // put your test here... 
    if (fBoards.at(0)->GetConfig()->GetBoardType() == boardMOSAIC) {
      fBoards.at(0)->SetTriggerConfig (true, true, myPulseDelay, myPulseLength * 2);
      fBoards.at(0)->SetTriggerSource (trigInt);
    }
    else if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
      fBoards.at(0)->SetTriggerConfig (true, false, myStrobeDelay, myPulseDelay);
      fBoards.at(0)->SetTriggerSource (trigExt);
    }

    scan();

    sprintf(fName, "Data/ThresholdScan_%s.dat", Suffix);
    WriteDataToFile (fName, true);
    sprintf(fName, "Data/ScanConfig_%s.cfg", Suffix);
    WriteScanConfig (fName, fChips.at(0), myDAQBoard);

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}
