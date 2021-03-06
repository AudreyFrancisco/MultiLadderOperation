#include "TConfig.h" 
#include "TBoardConfigDAQ.h"
#include "TBoardConfigMOSAIC.h"
#include <iostream>
#include <string.h>

using namespace std;

//construct Config from config file
TConfig::TConfig (const char *fName) {
  fDeviceType = TYPE_UNKNOWN;   // will be overwritten in read config file
  ReadConfigFile (fName);
}


// construct Config in the application using only number of boards and number of chips / vector of chip Ids
// for the time being use one common config for all board types (change this?)
// this constructor does not set the device type correctly 
// (not clear right now, which setup this constructor will be used for)
TConfig::TConfig (int nBoards, std::vector <int> chipIds, TBoardType boardType) {
  std::cout << "Warning: using deprecated constructur that does not set setup type correctly" << std::endl;
  fDeviceType = TYPE_UNKNOWN;
  Init(nBoards, chipIds, boardType);
}


// construct a config for a single chip setup (one board and one chip only)
TConfig::TConfig (int chipId, TBoardType boardType) {
  Init(chipId, boardType);
}


void TConfig::Init (int nBoards, std::vector <int> chipIds, TBoardType boardType) {
  for (int iboard = 0; iboard < nBoards; iboard ++) {
    if (boardType == kBOARD_DAQ) {
      fBoardConfigs.push_back (new TBoardConfigDAQ());
    } 
    else if (boardType == kBOARD_MOSAIC) {
      fBoardConfigs.push_back (new TBoardConfigMOSAIC());
    }
    else {
      std::cout << "TConfig: Unknown board type" << std::endl;
    }
  }
  for (int ichip = 0; ichip < (int)chipIds.size(); ichip ++) {
    fChipConfigs.push_back (new TChipConfig(this, chipIds.at(ichip)));
  } 
}


void TConfig::Init (int chipId, TBoardType boardType) {
  if (boardType == kBOARD_DAQ) {
    fDeviceType = TYPE_CHIP;
    fBoardConfigs.push_back (new TBoardConfigDAQ());
  } 
  else if (boardType == kBOARD_MOSAIC) {
    fDeviceType = TYPE_CHIP_MOSAIC;
    fBoardConfigs.push_back (new TBoardConfigMOSAIC());
  }
  else {
    fDeviceType = TYPE_UNKNOWN;
    std::cout << "TConfig: Unknown board type" << std::endl;
  }

  fChipConfigs. push_back (new TChipConfig (this, chipId));
}


// getter functions for chip and board config
TChipConfig *TConfig::GetChipConfigById  (int chipId) {
  for (int i = 0; i < (int)fChipConfigs.size(); i++) {
    if (fChipConfigs.at(i)->GetChipId() == chipId) 
      return fChipConfigs.at(i);
  }
  // throw exception here.
  std::cout << "Chip id " << chipId << " not found" << std::endl;
  return 0;
}


TChipConfig *TConfig::GetChipConfig (int iChip) {
  if ( iChip < (int)fChipConfigs.size()) {
    return fChipConfigs.at(iChip);
  }
  else {
    return 0;
  }
}


TBoardConfig *TConfig::GetBoardConfig (int iBoard){
  if ( iBoard < (int)fBoardConfigs.size()) {
    return fBoardConfigs.at(iBoard);
  }
  else {  // throw exception
    return 0;
  }
}


TDeviceType TConfig::ReadDeviceType (const char *deviceName) {
  TDeviceType type = TYPE_UNKNOWN;
  if (!strcmp (deviceName, "CHIP")) {
    type = TYPE_CHIP;
  }
  else if (!strcmp(deviceName, "TELESCOPE")) {
    type = TYPE_TELESCOPE;
  }
  else if (!strcmp(deviceName, "OBHIC")) {
    type = TYPE_OBHIC;
  }
  else if (!strcmp(deviceName, "IBHIC")) {
    type = TYPE_IBHIC;
  }
  else if (!strcmp(deviceName, "CHIPMOSAIC")) {
    type = TYPE_CHIP_MOSAIC;
  }
  else if (!strcmp(deviceName, "HALFSTAVE")) {
    type = TYPE_HALFSTAVE;
  }
  else {
    std::cout << "Error, unknown setup type found: " << deviceName << std::endl;
    exit (EXIT_FAILURE);
  }
  return type;
}


void TConfig::SetDeviceType (TDeviceType AType, int NChips) {
  std::vector <int> chipIds;

  fDeviceType = AType;
  if (AType == TYPE_CHIP) {
    Init(16, kBOARD_DAQ);
  }
  else if (AType == TYPE_CHIP_MOSAIC) {
    Init(16, kBOARD_MOSAIC);
  }
  else if (AType == TYPE_TELESCOPE) {
    for (int i = 0; i < NChips; i++) {
      chipIds.push_back(16);
    }
    Init(NChips, chipIds, kBOARD_DAQ);
  }
  else if (AType == TYPE_OBHIC) {
    for (int i = 0; i < 15; i++) {
      if (i == 7) continue;
      chipIds.push_back(i + ((DEFAULT_MODULE_ID & 0x7) << 4));
    }
    Init (1, chipIds, kBOARD_MOSAIC);
  }
  else if (AType == TYPE_IBHIC) {
    for (int i = 0; i < 9; i++) {
      chipIds.push_back(i);
    }
    Init (1, chipIds, kBOARD_MOSAIC);    
  }
  else if (AType == TYPE_HALFSTAVE) {
    for (int imod = 0; imod < NChips; imod++) {
      int moduleId = imod + 1;
      for (int i = 0; i < 15; i++) {
        if (i == 7) continue;
        chipIds.push_back(i + ((moduleId & 0x7) << 4));
      }
    }
    Init (2, chipIds, kBOARD_MOSAIC);
  }
}


void TConfig::ReadConfigFile (const char *fName) 
{
  char        Line[1024], Param[50], Rest[50];
  bool        Initialised = false;
  int         NChips      = 0;
  int         NModules    = 0;
  int         Chip;
  TDeviceType type        = TYPE_UNKNOWN;
  FILE       *fp          = fopen (fName, "r");

  if (!fp) {
    std::cout << "WARNING: Config file " << fName << " not found, using default configuration." << std::endl;
    return;
  }

  // first look for the type of setup in order to initialise config structure
  while ((!Initialised) && (fgets(Line, 1023, fp) != NULL)) {
    if ((Line[0] == '\n') || (Line[0] == '#')) continue; 
    ParseLine (Line, Param, Rest, &Chip);
    if (!strcmp(Param,"NCHIPS")){
      sscanf(Rest, "%d", &NChips);
    }
    if (!strcmp(Param,"NMODULES")){
      sscanf(Rest, "%d", &NModules);
    }
    if (!strcmp(Param, "DEVICE")) {
      type = ReadDeviceType (Rest);
    }
    if ((type != TYPE_UNKNOWN) && ((type != TYPE_TELESCOPE) || (NChips > 0)) && ((type != TYPE_HALFSTAVE) || (NModules == 0))) {   // type and nchips has been found (nchips not needed for type chip)
      // SetDeviceType calls the appropriate init method, which in turn calls
      // the constructors for board and chip configs
      if (type == TYPE_HALFSTAVE) SetDeviceType(type, NModules);
      else SetDeviceType(type, NChips);
      Initialised = true;
    }
  }

  fScanConfig = new TScanConfig();

  // now read the rest
  while (fgets(Line, 1023, fp) != NULL) {
    DecodeLine(Line);
  }
}


void TConfig::ParseLine(const char *Line, char *Param, char *Rest, int *Chip) {
  char MyParam[132];
  char *MyParam2;
  if (!strchr(Line, '_')) {
    *Chip = -1;
    sscanf (Line,"%s\t%s",Param, Rest);
  }
  else {
    sscanf (Line,"%s\t%s", MyParam, Rest);
    MyParam2 = strtok(MyParam, "_");
    sprintf(Param, "%s", MyParam2);
    sscanf (strpbrk(Line, "_")+1, "%d", Chip);
  }
}


void TConfig::DecodeLine(const char *Line)
{
  int Chip, Start, ChipStop, BoardStop;
  char Param[128], Rest[896];
  if ((Line[0] == '\n') || (Line[0] == '#')) {   // empty Line or comment
      return;
  }

  ParseLine(Line, Param, Rest, &Chip);

  if (Chip == -1) {
    Start     = 0;
    ChipStop  = fChipConfigs.size();
    BoardStop = fBoardConfigs.size();
  }
  else {
    Start     = Chip;
    ChipStop  = Chip+1;
    BoardStop = Chip+1;
  }

  // Todo: correctly handle the number of readout boards
  // currently only one is written
  // Note: having a config file with parameters for the mosaic board, but a setup with a DAQ board
  // (or vice versa) will issue unknown-parameter warnings... 
  if (fChipConfigs.at(0)->IsParameter(Param)) {
    for (int i = Start; i < ChipStop; i++) {
      fChipConfigs.at(i)->SetParamValue (Param, Rest);
    }
  }
  else if (fBoardConfigs.at(0)->IsParameter(Param)) {
    for (int i = Start; i < BoardStop; i++) {
      fBoardConfigs.at(i)->SetParamValue (Param, Rest);
    }
  }
  else if (fScanConfig->IsParameter(Param)) {
    fScanConfig->SetParamValue (Param, Rest);
  }
  else if ((!strcmp(Param, "ADDRESS")) && (fBoardConfigs.at(0)->GetBoardType() == kBOARD_MOSAIC)) {
    for (int i = Start; i < BoardStop; i++) {
      ((TBoardConfigMOSAIC *)fBoardConfigs.at(i))->SetIPaddress(Rest);
    }
  }
  else {
    std::cout << "Warning: Unknown parameter " << Param << std::endl;
  }


}

// write config to file, has to call same function for all sub-configs (chips and boards)
void TConfig::WriteToFile (string fName) {
    cout << "TConfig::WriteToFile() - filename = " << fName << " , doing nothing (not implemented)" << endl;
}
