#include <iostream>
#include "SetupHelpers.h"
#include "USBHelpers.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"


// Setup definition for outer barrel module with MOSAIC
//    - module ID (3 most significant bits of chip ID) defined by moduleId 
//      (usually 1) 
//    - chips connected to two different control interfaces
//    - masters send data to two different receivers (0 and 1)
//    - receiver number for slaves set to -1 (not connected directly to receiver)
//      (this ensures that a receiver is disabled only if the connected master is disabled)
int initSetupOB(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips) {
  (*boardType)                      = boardMOSAIC;
  TBoardConfigMOSAIC *boardConfig = (TBoardConfigMOSAIC*) config->GetBoardConfig(0);

  boardConfig->SetInvertedData (boardConfig->IsInverted());
  boardConfig->SetSpeedMode    (Mosaic::RCV_RATE_400);

  boards->push_back (new TReadoutBoardMOSAIC(config, boardConfig));

  for (int i = 0; i < config->GetNChips(); i++) {
    TChipConfig *chipConfig = config   ->GetChipConfig(i);
    int          chipId     = chipConfig->GetChipId    ();
    int          control    = chipConfig->GetParamValue("CONTROLINTERFACE");
    int          receiver   = chipConfig->GetParamValue("RECEIVER");

    if (chipId%16!=0) chipConfig->SetParamValue("LINKSPEED", "-1"); // deactivate the DTU/PLL for none master chips

    chips->push_back(new TAlpide(chipConfig));
    chips->at(i) -> SetReadoutBoard(boards->at(0));
    if (i < 7) {              // first master-slave row
      if (receiver < 0) receiver = 9;
      if (control  < 0) control  = 1;
    }
    else {                    // second master-slave row
      if (receiver < 0) receiver = 0;
      if (control  < 0) control  = 0;
    }
    boards->at(0)-> AddChip        (chipId, control, receiver);
  }
  int nWorking = CheckControlInterface(config, boards, boardType, chips);
    std::cout << "initSetupOB() - nWorking = " << nWorking << std::endl;
  sleep(5);
  MakeDaisyChain(config, boards, boardType, chips);
  return 0;
}


// implicit assumptions on the setup in this method
// - chips of master 0 of all modules are connected to 1st mosaic, chips of master 8 to 2nd MOSAIC
int initSetupHalfStave(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips) {
  (*boardType) = boardMOSAIC;
  for (int i = 0; i < config->GetNBoards(); i++) {
    TBoardConfigMOSAIC* boardConfig = (TBoardConfigMOSAIC*) config->GetBoardConfig(i);

    boardConfig->SetInvertedData (false);  //already inverted in the adapter plug ?
    boardConfig->SetSpeedMode    (Mosaic::RCV_RATE_400);

    boards->push_back (new TReadoutBoardMOSAIC(config, boardConfig));
  }

  for (int i = 0; i < config->GetNChips(); i++) {
    TChipConfig* chipConfig = config   ->GetChipConfig(i);
    int          chipId     = chipConfig->GetChipId    ();
    int          mosaic     = (chipId & 0x1000) ? 1:0;

    chips->push_back(new TAlpide(chipConfig));
    chips->at(i) -> SetReadoutBoard(boards->at(mosaic));
    
    // to be checked when final layout of adapter fixed
    int ci  = 0; 
    int rcv = (chipId & 0x7) ? -1 : 9*ci; //FIXME 
    boards->at(mosaic)-> AddChip(chipId, ci, rcv);
  }

  int nWorking = CheckControlInterface(config, boards, boardType, chips);
    std::cout << "initSetupHalfStave() - nWorking = " << nWorking << std::endl;
  sleep(5);
  MakeDaisyChain(config, boards, boardType, chips);
  return 0;
}


// Make the daisy chain for OB readout, based on enabled chips
// i.e. to be called after CheckControlInterface
void MakeDaisyChain(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips) {
  for (unsigned int i = 0; i < chips->size(); i++) {
    if (!chips->at(i)->GetConfig()->IsEnabled()) continue;
    int chipId   = chips->at(i)->GetConfig()->GetChipId();
    int previous = -1;
    if (!(chipId & 0x7)) {   // Master, has initial token, previous chip is last enabled chip in row
      chips->at(i)->GetConfig()->SetInitialToken(true);
      for (int iprev = chipId + 6; (iprev > chipId) && (previous == -1); iprev--) { 
        if (config->GetChipConfigById(iprev)->IsEnabled()) {
          previous = iprev; 
	}
      }
      if (previous == -1) {
        chips->at(i)->GetConfig()->SetPreviousId(chipId);
      }
      else {
         chips->at(i)->GetConfig()->SetPreviousId(previous);
      }
    }
    else {    // Slave, does not have token, previous chip is last enabled chip before
      chips->at(i)->GetConfig()->SetInitialToken(false);
      for (int iprev = chipId - 1; (iprev >= (chipId & 0x78)) && (previous == -1); iprev--) {
        if (config->GetChipConfigById(iprev)->IsEnabled()) {
          previous = iprev; 
	}
      }
      if (previous == -1) {
        chips->at(i)->GetConfig()->SetPreviousId(chipId);
      }
      else {
         chips->at(i)->GetConfig()->SetPreviousId(previous);
      }
    }
    std::cout << "Chip Id " << chipId << ", token = " << (bool) chips->at(i)->GetConfig()->GetInitialToken() << ", previous = " << chips->at(i)->GetConfig()->GetPreviousId() << std::endl;
  }
}


// Try to communicate with all chips, disable chips that are not answering
int CheckControlInterface(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips) {
  uint16_t WriteValue = 10;
  uint16_t Value;
  int      nWorking = 0;

  std::cout << std::endl << "Before starting actual test:" << std::endl << "Checking the control interfaces of all chips by doing a single register readback test" << std::endl;

  for (unsigned int i = 0; i < chips->size(); i++) {
    if (!chips->at(i)->GetConfig()->IsEnabled()) continue;
    chips->at(i)->WriteRegister (0x60d, WriteValue);
    try {
      chips->at(i)->ReadRegister (0x60d, Value);
      if (WriteValue == Value) {
        std::cout << "  Chip ID " << chips->at(i)->GetConfig()->GetChipId() << ", readback correct." << std::endl;
        nWorking ++;   
      }
      else {
	std::cout << "  Chip ID " << chips->at(i)->GetConfig()->GetChipId() << ", wrong readback value (" << Value << " instead of " << WriteValue << "), disabling." << std::endl;
        chips->at(i)->GetConfig()->SetEnable(false);
      }
    }
      catch (std::exception &e) {
      std::cout << "  Chip ID " << chips->at(i)->GetConfig()->GetChipId() << ", not answering, disabling." << std::endl;
      chips->at(i)->GetConfig()->SetEnable(false);
    }
    
  }
  std::cout << "Found " << nWorking << " working chips." << std::endl << std::endl;
  return nWorking;
}


// Setup definition for inner barrel stave with MOSAIC
//    - all chips connected to same control interface
//    - each chip has its own receiver, mapping defined in RCVMAP
int initSetupIB(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips) {
  int RCVMAP []                   = { 3, 5, 7, 8, 6, 4, 2, 1, 0 };
  (*boardType)                      = boardMOSAIC;
  TBoardConfigMOSAIC *boardConfig = (TBoardConfigMOSAIC*) config->GetBoardConfig(0);

  boardConfig->SetInvertedData (false);

  Mosaic::TReceiverSpeed speed; 

  switch (config->GetChipConfig(0)->GetParamValue("LINKSPEED")) {
  case 400: 
    speed = Mosaic::RCV_RATE_400;
    break;
  case 600: 
    speed = Mosaic::RCV_RATE_600;
    break;
  case 1200: 
    speed = Mosaic::RCV_RATE_1200;
    break;
  default: 
    std::cout << "Warning: invalid link speed, using 1200" << std::endl;
    speed = Mosaic::RCV_RATE_1200;
    break;
  }
  std::cout << "Speed mode = " << speed << std::endl;
  boardConfig->SetSpeedMode    (speed);

  boards->push_back (new TReadoutBoardMOSAIC(config, boardConfig));

  for (int i = 0; i < config->GetNChips(); i++) {
    TChipConfig *chipConfig = config->GetChipConfig(i);
    int          control    = chipConfig->GetParamValue("CONTROLINTERFACE");
    int          receiver   = chipConfig->GetParamValue("RECEIVER");
    chips->push_back(new TAlpide(chipConfig));
    chips->at(i) -> SetReadoutBoard(boards->at(0));

    if (control  < 0) control  = 0;
    if (receiver < 0) receiver = RCVMAP[i];

    boards->at(0)-> AddChip        (chipConfig->GetChipId(), control, receiver);
  }

  int nWorking = CheckControlInterface(config, boards, boardType, chips);
    std::cout << "initSetupIB() - nWorking = " << nWorking << std::endl;
  return 0;
}


int initSetupSingleMosaic(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips) {
  TChipConfig        *chipConfig  = config->GetChipConfig(0);
  (*boardType)                      = boardMOSAIC;
  TBoardConfigMOSAIC *boardConfig = (TBoardConfigMOSAIC*) config->GetBoardConfig(0);
  int                 control     = chipConfig->GetParamValue("CONTROLINTERFACE");
  int                 receiver    = chipConfig->GetParamValue("RECEIVER");

  if (receiver < 0) receiver = 3;   // HSData is connected to pins for first chip on a stave
  if (control  < 0) control  = 0; 

  boardConfig->SetInvertedData (false);
  boardConfig->SetSpeedMode    (Mosaic::RCV_RATE_400);

  boards->push_back (new TReadoutBoardMOSAIC(config, boardConfig));

  chips-> push_back(new TAlpide(chipConfig));
  chips-> at(0) -> SetReadoutBoard(boards->at(0));
  boards->at(0) -> AddChip        (chipConfig->GetChipId(), control, receiver);
  return 0;
}


int initSetupSingle(TConfig* config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips) {
  TReadoutBoardDAQ  *myDAQBoard = 0;
  TChipConfig       *chipConfig = config->GetChipConfig(0);
  chipConfig->SetParamValue("LINKSPEED", "-1");
  (*boardType)                    = boardDAQ;
  // values for control interface and receiver currently ignored for DAQ board
  //int               control     = chipConfig->GetParamValue("CONTROLINTERFACE");
  //int               receiver    = chipConfig->GetParamValue("RECEIVER");
  
  InitLibUsb(); 
  //  The following code searches the USB bus for DAQ boards, creates them and adds them to the readout board vector: 
  //  TReadoutBoard *readoutBoard = new TReadoutBoardDAQ(device, config);
  //  board.push_back (readoutBoard);
  FindDAQBoards (config, boards);
  std::cout << "Found " << boards->size() << " DAQ boards" << std::endl;
  myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (boards->at(0));

  if (boards->size() != 1) {
    std::cout << "Error in creating readout board object" << std::endl;
    return -1;
  }

  // for Cagliari DAQ board disable DDR and Manchester encoding
  chipConfig->SetEnableDdr         (false);
  chipConfig->SetDisableManchester (true);

  // create chip object and connections with readout board
  chips-> push_back(new TAlpide (chipConfig));
  chips-> at(0) -> SetReadoutBoard (boards->at(0));

  boards->at(0) -> AddChip         (chipConfig->GetChipId(), 0, 0);

  powerOn(myDAQBoard);

  return 0;
}


int powerOn (TReadoutBoardDAQ *aDAQBoard) {
  int overflow;

  if (aDAQBoard -> PowerOn (overflow)) std::cout << "LDOs are on" << std::endl;
  else std::cout << "LDOs are off" << std::endl;
  std::cout << "Version = " << std::hex << aDAQBoard->ReadFirmwareVersion() << std::dec << std::endl;
  aDAQBoard -> SendOpCode (Alpide::OPCODE_GRST);
  //sleep(1); // sleep necessary after GRST? or PowerOn?

  std::cout << "Analog Current  = " << aDAQBoard-> ReadAnalogI()     << std::endl;
  std::cout << "Digital Current = " << aDAQBoard-> ReadDigitalI()    << std::endl;
  std::cout << "Temperature     = " << aDAQBoard-> ReadTemperature() << std::endl;

  return 0;
}


int initSetup(TConfig*& config, std::vector <TReadoutBoard *> * boards, TBoardType* boardType, std::vector <TAlpide *> * chips, const char *configFileName) {
  config = new TConfig (configFileName);

  switch (config->GetDeviceType())
    {
    case TYPE_CHIP: 
      initSetupSingle(config, boards, boardType, chips);
      break;
    case TYPE_IBHIC:
      initSetupIB(config, boards, boardType, chips);
      break;
    case TYPE_OBHIC:
      initSetupOB(config, boards, boardType, chips);
      break;
    case TYPE_CHIP_MOSAIC: 
      initSetupSingleMosaic(config, boards, boardType, chips);
      break;
    default: 
      std::cout << "Unknown setup type, doing nothing" << std::endl;
      return -1;
    }
  return 0;
}

