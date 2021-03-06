#include <math.h> 
#include <string.h>
#include <unistd.h>
#include <thread>

#include "USB.h"
#include "AlpideDictionary.h"
#include "TAlpide.h"
#include "TReadoutBoardDAQ.h"
#include "TBoardDecoder.h"

#pragma mark - constructor/destructor

using namespace std;

//___________________________________________________________________
TReadoutBoardDAQ::TReadoutBoardDAQ()
{ }

// constructor
//___________________________________________________________________
TReadoutBoardDAQ::TReadoutBoardDAQ (libusb_device *ADevice,
                                    shared_ptr<TBoardConfigDAQ> config ) :
    TUSBBoard (ADevice),
    fBoardConfigDAQ( config ),
    fIsTriggerThreadRunning( false ),
    fTrigCnt( 0 ),
    fIsReadDataThreadRunning( false ),
    fEvtCnt( 0 ),
    //fMaxDiffTrigEvtCnt( MAX_DIFF_TRIG_EVT_CNT ),
    fMaxEventBufferSize( MAX_EVT_BUFFSIZE ),
    fNTriggersTotal( 0 ),
    fMaxNTriggersTrain( MAX_NTRIG_TRAIN )
{
  //WriteDelays();

  // write default config to all registers
  WriteCMUModuleConfigRegisters();
  WriteReadoutModuleConfigRegisters();
  WriteADCModuleConfigRegisters();
  WriteTriggerModuleConfigRegisters();
  WriteResetModuleConfigRegisters();
  WriteSoftResetModuleConfigRegisters(); 
  
  //WriteRegister (0x200, 0x1801); 

}

// destructor
//___________________________________________________________________
TReadoutBoardDAQ::~TReadoutBoardDAQ ()
{
  // join threads
  
  //fThreadTrigger.join();
  //fThreadReadData.join();
  //std::cout << "joined threads.." << std::endl;

  std::cout << "Powering off chip" << std::endl;
  PowerOff();

}

#pragma mark - general methods of TReadoutBoard

//___________________________________________________________________
int TReadoutBoardDAQ::ReadRegister (uint16_t address, uint32_t &value)
{
  unsigned char data_buf[DAQBOARD_WORD_SIZE * 2];
  uint32_t      headerword = 0;
  int           err; 

  err = SendWord ((uint32_t)address +  (1 << (DAQBOARD_REG_ADDR_SIZE + DAQBOARD_MODULE_ADDR_SIZE))); // add 1 bit for read access
  if (err < 0) return -1;
  err = ReceiveData (ENDPOINT_READ_REG, data_buf, DAQBOARD_WORD_SIZE * 2); 

  if (err < 0) return -1;

  value = 0;

  for (int i = 0; i < DAQBOARD_WORD_SIZE; i ++) {
    headerword += (data_buf[i                     ] << (8 * i));   // bytes 0 ... 3 are header
    value      += (data_buf[i + DAQBOARD_WORD_SIZE] << (8 * i));   // bytes 4 ... 7 are data
  }
  return 0;
}


int TReadoutBoardDAQ::WriteRegister (uint16_t address, uint32_t value)
{
  //std::cout << "[FPGA] ADDRESS: " << std::hex <<  address << " VALUE " << value << std::dec << std::endl; 

  int err;
  err = SendWord((uint32_t)address);

  if (err < 0) return -1;       // add exceptions

  err = SendWord(value);
  if (err < 0) return -1;
  err = ReadAcknowledge();
  if (err < 0) return -1;

  return 0;
}



int TReadoutBoardDAQ::WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId,
                                         const bool doExecute )
{
    if ( doExecute ) {
        // nothing special to do for DAQ board
    }
    int err;
  uint32_t address32 = (uint32_t) address;
  uint32_t chipId32  = (uint32_t) chipId;
  uint32_t newAddress = (address32 << 16) | (chipId32 << 8) | ((uint32_t)AlpideOpCode::WROP);

  //std::cout << "[CHIP] ADDRESS: " << std::hex <<  newAddress << " VALUE " << value << std::dec << std::endl; 

  //err = WriteRegister (CMU_DATA + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), (uint32_t) value);
  //if(err < 0) return -1;
  //err = WriteRegister (CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), newAddress);
  //if(err < 0) return -1;

    uint32_t command[4];
    //bool err;

    //std::cout << "[ CHIP ] ADDRESS: " << std::hex << address << " (" << newAddress << ") " << " VALUE " << value << std::dec << std::endl;
    command[0] = CMU_DATA + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE);
    command[1] = value;
    command[2] = CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE);
    command[3] = newAddress;
    SendWord((uint32_t)command[0]);
    SendWord((uint32_t)command[1]);
    //if(err==false) return -1;
    err = ReadAcknowledge();
    if (err < 0) return -1;
    SendWord((uint32_t)command[2]);
    SendWord((uint32_t)command[3]);
    //err=ReadAck();
    //if(err==false) return -1;
    err = ReadAcknowledge();
    if (err < 0) return -1;

    return 1;
  //return 0;
}


int TReadoutBoardDAQ::ReadChipRegister (uint16_t address, uint16_t &value, uint8_t chipId,
                                        const bool doExecute )
{
    if ( doExecute ) {
        // nothing special to do for DAQ board
    }

  int           err;
  uint32_t      value32; 
  uint32_t      address32  = (uint32_t) address;
  uint32_t      chipId32   = (uint32_t) chipId;
  uint32_t      newAddress = (address32 << 16) | (chipId32 << 8) | ((uint32_t)AlpideOpCode::RDOP);


  err = WriteRegister(CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), newAddress);
  if (err < 0) return -1;
  err = ReadRegister (CMU_DATA + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), value32);
  if (err < 0) return -1;

  value = (value32>>8) & 0xffff;

  //std::cout << std::hex << value32 << std::dec << std::endl;

  uint8_t received_chipid = value32 & 0xff;
  if (received_chipid!=chipId) {
    std::cout << "WARNING: received chipID (" << (int)received_chipid << ") does not match with configuration in DAQboard (" << (int)chipId << ")" << std::endl;
    return -1;
  }
  else {
    return 0;
  }
}


int TReadoutBoardDAQ::SendOpCode (uint16_t  OpCode)
{
  return WriteRegister (CMU_INSTR + (MODULE_CMU << DAQBOARD_REG_ADDR_SIZE), (int) OpCode);
}



int TReadoutBoardDAQ::SetTriggerConfig  (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay)
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();
  spBoardConfigDAQ->SetTriggerEnable(enableTrigger); // enableTrigger? DAQboard trigger disabled only if fBoardConfigDAQ.TriggerMode==0..
  spBoardConfigDAQ->SetPulseEnable(enablePulse); // enablePulse on DAQboard??

  spBoardConfigDAQ->SetTriggerDelay((int32_t)triggerDelay);
  spBoardConfigDAQ->SetStrobeDelay((int32_t)triggerDelay); // equivalent to trigger delay on DAQboard..
  WriteTriggerModuleConfigRegisters();

  spBoardConfigDAQ->SetPulseDelay(pulseDelay); // delay between pulse and strobe/trigger; if fStrobePulseSeq is set correctly (to 2)
  WriteResetModuleConfigRegisters();

  return 0;
}


void TReadoutBoardDAQ::SetTriggerSource (TTriggerSource triggerSource)
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();
    if (triggerSource == TTriggerSource::kTRIG_INT) {
    spBoardConfigDAQ->SetTriggerMode(1);
    WriteTriggerModuleConfigRegisters();
  }
    else if (triggerSource == TTriggerSource::kTRIG_EXT) {
    spBoardConfigDAQ->SetTriggerMode(2);
    WriteTriggerModuleConfigRegisters();
  }
  else {
    std::cerr << "!!! Trigger source not known, doing nothing !!!" << std::endl;
  }
}


// trigger function to be executed in thread fThreadTrigger
void TReadoutBoardDAQ::DAQTrigger() {
  //std::cout << "-> in DAQTrigger function now" << std::endl;
  fMtx.lock();
  fIsTriggerThreadRunning = true;
  fMtx.unlock();
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

  fStatusTrigger = 0;
  unsigned int evtbuffer_size = 0;
  if (spBoardConfigDAQ->GetTriggerEnable() && !spBoardConfigDAQ->GetPulseEnable()) { // TRIGGERING
    std::cout << "Number of triggers: " << fNTriggersTotal << std::endl;
    int nTriggerTrains = fNTriggersTotal/fMaxNTriggersTrain;
    std::cout << " --> " << nTriggerTrains << " trigger trains with " << fMaxNTriggersTrain << " triggers going to be launched" << std::endl;
    int nTriggersLeft  = fNTriggersTotal%fMaxNTriggersTrain; // TODO: nicer solution?
    std::cout << " --> then " << nTriggersLeft << " triggers left to be launched" << std::endl;

    spBoardConfigDAQ->SetNTriggers(fMaxNTriggersTrain);
    WriteTriggerModuleConfigRegisters();

    for (int itrain=0; itrain<nTriggerTrains; itrain++) {
      fMtx.lock();
      evtbuffer_size = fEventBuffer.size();
      fMtx.unlock();
      if (evtbuffer_size < fMaxEventBufferSize) {
        std::cout << "train " << itrain << std::endl;
        
        StartTrigger(); // start trigger train; 
        // sleep for enough time so that stoptrigger sent after last trigger..
        //int sleep_time = spBoardConfigDAQ->GetStrobeDelay()*0.25+375; // TODO: check why this is not working.. but longer a wait time is needed..
        int sleep_time = spBoardConfigDAQ->GetStrobeDelay();
        usleep(sleep_time);
        StopTrigger();

        fTrigCnt += fMaxNTriggersTrain;
      }
      else {
        std::cout << "Maximum event buffer size reached before reaching nTriggers; stop here!" << std::endl;
        std::cout << "    -> Number of triggers performed: " << fTrigCnt << std::endl;
        fStatusTrigger = -1;
        return;
      }
    }
    
    if (nTriggersLeft!=0) { // TODO: nicer solution?
      fMtx.lock();
      evtbuffer_size = fEventBuffer.size();
      fMtx.unlock();
      if (evtbuffer_size < fMaxEventBufferSize) {
        spBoardConfigDAQ->SetNTriggers(nTriggersLeft);
        WriteTriggerModuleConfigRegisters();

        StartTrigger(); // start trigger train; 
        // sleep for enough time so that stoptrigger sent after last trigger..
        //int sleep_time = spBoardConfigDAQ->GetStrobeDelay()*0.025+375; // TODO: check why this is not working.. but longer a wait time is needed..
        int sleep_time = spBoardConfigDAQ->GetStrobeDelay();
        usleep(sleep_time);
        StopTrigger();
        
        fTrigCnt += nTriggersLeft;
      }
      else {
        std::cout << "Maximum event buffer size reached before reaching nTriggers; stop here!" << std::endl;
        std::cout << "    -> Number of triggers performed: " << fTrigCnt << std::endl;
        fStatusTrigger = -1;
        return;
      }
    }
  }
  else if (!spBoardConfigDAQ->GetTriggerEnable() && spBoardConfigDAQ->GetPulseEnable()) { // just PULSING
    //spBoardConfigDAQ->SetNTriggers(100); // TODO: number of triggers to be launched with StartTrigger command? if set trigger src to external not trigger sent at all?
    //WriteTriggerModuleConfigRegisters();

    //StartTrigger(); // TODO: needed so that DAQboard reads events? but does it also send trigger?
    for (fTrigCnt=0; fTrigCnt<fNTriggersTotal; fTrigCnt++) {
      fMtx.lock();
      evtbuffer_size = fEventBuffer.size();
      fMtx.unlock();
      if (evtbuffer_size < fMaxEventBufferSize) {
        //SendOpCode (0x78); // send PULSE to chip
        StartTrigger();
        WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_PULSE, 13); // write anything to pulse register to trigger pulse
        StopTrigger();
        //std::cout << "Pulse " << fTrigCnt << " sent" << std::endl;
      }
      else {
        std::cout << "Maximum event buffer size reached before reaching nTriggers (Pulses); stop here!" << std::endl;
        std::cout << "    -> Number of pulses performed: " << fTrigCnt << std::endl;
        fStatusTrigger = -1;
        return;
      }
    }
    //StopTrigger();
    
  }
  else {
    std::cout << "Pulse and Trigger either both disabled or enabled, please select one of the two!" << std::endl;
    return;
  }

 
  // DEBUG read monitoring registers
  //ReadMonitorRegisters();

  // send EndOfRun command; resets counters in header
  //WriteRegister(0x201, 0x1);

  fMtx.lock();
  fIsTriggerThreadRunning = false;
  fMtx.unlock();
  fStatusTrigger = 1; // exited successfully
}


// readdata function to be executed in thread fThreadReadData
void TReadoutBoardDAQ::DAQReadData() {
  fMtx.lock();
  //std::cout << " -> in DAQReadData now" << std::endl;
  fIsReadDataThreadRunning = true;
  fMtx.unlock();

  const int max_length_buf = 1024*1000;   // length needed at ITHR=10 ~5000!!!  
  const int length_buf = 1024;   // length needed at ITHR=10 ~5000!!!  
  unsigned char data_buf[max_length_buf]; // TODO large enough?   
  int evt_length = 0;       
  int tmp_error = 0;
  
  std::vector <unsigned char> data_evt (max_length_buf);
  //std::copy(my_deque.begin(), my_deque.end(), std::back_inserter(my_vector));

    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

  if (spBoardConfigDAQ->GetPktBasedROEnable() == false) { // event based
    //std::cout << " --> event based readout" << std::endl;
    while (fEvtCnt<fNTriggersTotal) { // no stop-trigger marker with event-based readout
      data_evt.clear();
      evt_length = ReceiveData(ENDPOINT_READ_DATA, data_buf, max_length_buf, &tmp_error);
      //std::cout << "Received " << *length << " bytes" << std::endl;

      if (tmp_error == -7) { // USB timeout
        std::cout << "timeout" << std::endl;
        fStatusReadData = -2;
        return; // TODO: this has to be handled better with
        //return -2;
      }
      else if (evt_length < 1) {
        std::cout << std::endl;
        std::cout << "ERROR, received data returned with " << evt_length << std::endl;
        std::cout << std::endl;
        fStatusReadData = -1;
        return;
      }
      else if (evt_length < (DAQBoardDecoder::GetDAQEventHeaderLength(fFirmwareVersion, spBoardConfigDAQ->GetHeaderType())+DAQBoardDecoder::GetDAQEventTrailerLength()+4)) {
        std::cout << std::endl;
        std::cout << "WARNING, received too small event: " << evt_length 
                  << " instead of expected >= " 
                  << (DAQBoardDecoder::GetDAQEventHeaderLength(fFirmwareVersion, spBoardConfigDAQ->GetHeaderType())+DAQBoardDecoder::GetDAQEventTrailerLength()+4)
                  << std::endl;
        std::cout << std::endl;
      }
      else {
        for (int i=0; i<evt_length; i++) {
            data_evt.push_back(data_buf[i]);
        }
        fMtx.lock();
        fEventBuffer.push_back(data_evt);
        fEvtCnt++;
        fMtx.unlock();

        // DEBUG output
        //fMtx.lock();
        //std::cout << "read evt " << fEvtCnt << std::endl;
        //std::cout << "\t data_evt size: " << data_evt.size() << std::endl;
        //std::cout << "\t EventBuffer length: " << fEventBuffer.size() << std::endl;
        //for (int iByte=0; iByte<data_evt.size(); ++iByte) {
        //  std::cout << std::hex << (int)data_evt[iByte] << std::dec;
        //}
        //std::cout << std::endl;
        //fMtx.unlock();
      }

    }
    
  }
  else if (spBoardConfigDAQ->GetPktBasedROEnable() == true) { // packet based
    //std::cout << " --> packet based readout" << std::endl;
    // each packet may contain more or less than one event. the following code split raw data into events and writes it into fEventBuffer
    evt_length = 0; // no data read so far
    bool foundMagicWord = false;
    const int nMagicWords = 5;
    unsigned char magicWords[nMagicWords][4] = { { 0xbf, 0xbf, 0xbf, 0xbf },   // pALPIDE-2/3 event trailer
                                                 { 0xaf, 0xaf, 0xaf, 0xaf },   // pALPIDE-2/3 event trailer for truncated event
                                                 { 0xfe, 0xeb, 0xfe, 0xeb },   // stop-trigger marker in the packet-based readout mode
                                                 { 0xef, 0xeb, 0xef, 0xeb },   // stop-trigger marker in the packet-based readout mode (inconsistent timestamp and data fifo)
                                                 { 0xfe, 0xab, 0xfe, 0xab } }; // pALPIDE-1 event trailer
    bool timeout = false;
    int packet_length = 0;
    unsigned int length_tmp    = 0;


    while (fEvtCnt<=fNTriggersTotal || fRawBuffer.size()!=0) { // at fEvtCnt==fNTriggersTotal it should find stop-trigger marker
     
      foundMagicWord = false;
      data_evt.clear();
      timeout = false;
      length_tmp = 0;

      do {
        while (length_tmp+4<=fRawBuffer.size() && !foundMagicWord) {// this is executed if fRawBuffer contains data, otherwise it jumps to next if and reads data..

          // DEBUG OUTPUT
          //std::cout << "length_tmp: " << length_tmp << std::endl;
          //std::cout << std::hex << (int)fRawBuffer[length_tmp+0] << (int)fRawBuffer[length_tmp+1] << (int)fRawBuffer[length_tmp+2] << (int)fRawBuffer[length_tmp+3] << std::dec;
          //std::cout << "\t";

          for (int iMagicWord=0; iMagicWord<nMagicWords; ++iMagicWord) {
            if (magicWords[iMagicWord][0] == fRawBuffer[length_tmp+0] &&
                magicWords[iMagicWord][1] == fRawBuffer[length_tmp+1] &&
                magicWords[iMagicWord][2] == fRawBuffer[length_tmp+2] &&
                magicWords[iMagicWord][3] == fRawBuffer[length_tmp+3]) {

              // if found magicword write data/event to fEventBufffer
              foundMagicWord = true;
              switch (iMagicWord) {
                case 1:
                  std::cerr << "Truncated pALPIDE-2/3 event found!" << std::endl;
                  break;
                case 3: 
                  std::cout << "Inconsistent timestamp and data FIFO detected!" << std::endl;
                  break;
                case 2:
                  // DEBUG OUTPUT
                  //for (int iByte=0; iByte<fRawBuffer.size(); ++iByte) {
                  //  std::cout << std::hex << (int)fRawBuffer[iByte] << std::dec;
                  //}
                  //std::cout << std::endl;
                  std::cout << "Stop-trigger marker received." << std::endl;
                  data_evt.clear();
                  //return -3;
                  fStatusReadData = -3;
                  return;
                  break;
              }
            }
          }
          length_tmp += 4;
        }

        if (!timeout && !foundMagicWord) { // read new data packet here if not a magic word found or timeout; this is performed until timeout or full event (magicword) achieved.. or error occurs
          packet_length = ReceiveData(ENDPOINT_READ_DATA, data_buf, length_buf, &tmp_error);
          //std::cout << "packet: " << packet_length << std::endl;

//          if (debug && debug_length) {
//            *debug = new unsigned char[length];
//            memcpy(*debug, data_buf, length);
//            *debug_length = length;
//          }
//          if (error) {
//            *error = tmp_error;
//          }

          if (tmp_error == -7) { // USB timeout
            timeout = true;

//#ifdef MYDEBUG
//          for (int iByte=0; iByte<fRawBuffer.size(); ++iByte) {
//            std::cout << std::hex << (int)fRawBuffer[iByte] << std::dec;
//          }
//          std::cout << std::endl;
//#endif
  
            std::cout << "timeout" << std::endl;
            fStatusReadData = -2;
            return;
            //return -2;
          }
  
          if (packet_length < 1) {
            std::cout << "Error, receive data returned with " << packet_length << std::endl;
            fStatusReadData = -1;
            return;
            //return -1;
          }
          if (packet_length%4!=0) {
            std::cout << "Error, received data was not a multiple of 32 bit! Packet length: " << packet_length << " byte" << std::endl;
            fStatusReadData = -1;
            return;
            //return -1;
          }
  
          for (int i=0; i<packet_length; i++) {
            fRawBuffer.push_back(data_buf[i]);
          }
  
//#if 0
//        std::cout << "USB RAW (length " << length << "): ";
//        for (int j=0; j<length; j++)
//          printf("%02x ", fRawBuffer[j]);
//        std::cout << std::endl;
//#endif

        }
      } while (length_tmp<fRawBuffer.size() && !foundMagicWord);
  
      // arrive here only if 
      if (!foundMagicWord) {
        //return -1; // did not achieve to read a full event
        fStatusReadData = -1;
        return;
      }
  
      evt_length = length_tmp;
  
      if (evt_length > max_length_buf) {
        evt_length = 0;
        std::cerr << "Event to large (" << evt_length << "Byte) to be read with a buffer of " << max_length_buf << "Byte!" << std::endl;
        //return -1;
        fStatusReadData = -1;
        return;
      }
      else if (evt_length < (DAQBoardDecoder::GetDAQEventHeaderLength(fFirmwareVersion, spBoardConfigDAQ->GetHeaderType())+DAQBoardDecoder::GetDAQEventTrailerLength()+4)) {
        std::cout << std::endl;
        std::cout << "WARNING, received too small event: " << evt_length << std::endl;
        std::cout << std::endl;
      }

      fEvtCnt++;
      //std::cout << "------------------------------------------------------" << std::endl;
      //std::cout << "\t evt: " << fEvtCnt << std::endl;
      //std::cout << "\t evt length: " << evt_length << std::endl;
      //std::cout << "\t RawBuffer length: " << fRawBuffer.size() << std::endl;
      for (int i=0; i<evt_length; ++i) {
        data_evt.push_back(fRawBuffer.front());
        fRawBuffer.pop_front();
      }
      fMtx.lock();
      fEventBuffer.push_back(data_evt);
      fMtx.unlock();
      //std::cout << "\t data_evt size: " << data_evt.size() << std::endl;
      //std::cout << "\t EventBuffer length: " << fEventBuffer.size() << std::endl;
      //std::cout << "------------------------------------------------------" << std::endl;


      // DEBUG OUTPUT
      //for (int iByte=0; iByte<data_evt.size(); ++iByte) {
      //  std::cout << std::hex << (int)data_evt[iByte] << std::dec;
      //}
      //std::cout << std::endl;


    } // end while fEvtCnt<fNTriggersTotal
  } // end if PktBasedROEnable

  // check if buffer is empty
  if (fRawBuffer.size() != 0) {
    std::cout << "WARNING: fRawBuffer not empty, but should be at this point!" << std::endl;
    for (unsigned int iByte=0; iByte<fRawBuffer.size(); ++iByte) {
      std::cout << std::hex << (int)fRawBuffer[iByte] << std::dec;
    }
  }

  fMtx.lock();
  fIsReadDataThreadRunning = false;
  fMtx.unlock();

  //return;
  fStatusReadData = 1;  // exited successfully
}





int TReadoutBoardDAQ::Trigger (int nTriggers) // open threads for triggering and reading/writing data to queue..
{
  //std::cout << "in Trigger function now" << std::endl;

  //int wait_counter = 0;
  //while ((fIsTriggerThreadRunning || fIsReadDataThreadRunning) && wait_counter<10) { // check if some other threads still running
  //  std::cout << "Trigger or ReadData thread still active, please wait" << std::endl;
  //  usleep(100000);
  //  wait_counter++;
  //}
  
  fNTriggersTotal = nTriggers;

  fEventBuffer.clear();
  fRawBuffer.clear();
  fTrigCnt = 0;
  fEvtCnt  = 0;

  // launch trigger and readdata in threads:
  //std::cout << "starting threads.." << std::endl;
  if (nTriggers>=0) {
    fThreadTrigger  = std::thread (&TReadoutBoardDAQ::DAQTrigger,   this);
  }
  else {
	fTrigCnt        = -nTriggers;
	fNTriggersTotal = -nTriggers;
  }
  //usleep(10000);
  //sleep(1);
  fThreadReadData = std::thread (&TReadoutBoardDAQ::DAQReadData,  this);
  //fThreadTrigger  = std::thread ([this] { DAQTrigger(); });
  //fThreadReadData = std::thread ([this] { DAQReadData(); });

  if (nTriggers>=0) fThreadTrigger.join();
  fThreadReadData.join();
  //std::cout << "joined threads.." << std::endl;

  return 0;
}




int TReadoutBoardDAQ::ReadEventData (int &NBytes, unsigned char *Buffer) // provide oldest event in queue and remove it from there
{
  //vector <unsigned char> evt_data = fEventBuffer.front(); 
  fMtx.lock();
  int n_evts_buf = fEventBuffer.size();
  fMtx.unlock();
  
  if (n_evts_buf==0) {
    //std::cout << "No events left in fEventBuffer. Exit." << std::endl;
    return -1;
  }

  fMtx.lock();
  NBytes = fEventBuffer.front().size();
  for (int i=0; i<NBytes; ++i) {
    Buffer[i] = fEventBuffer.front()[i];
    //std::cout << std::hex << (int)fEventBuffer.front()[i] << std::dec;
  }
  //std::cout << std::endl; 
  fEventBuffer.pop_front(); // delete oldest event from deque
  fMtx.unlock();

  //for (int i=0; i<NBytes; ++i) {
  //  std::cout << std::hex << (int)(uint8_t)Buffer[i] << std::dec;
  //}
  //std::cout << std::endl; 

  return 1;
}







//---------------------------------------------------------
// methods only for Cagliari DAQ board
//---------------------------------------------------------



// method to send 32 bit words to DAQ board
// FPGA internal registers have 12-bit addres field and 32-bit data payload
int TReadoutBoardDAQ::SendWord (uint32_t value) 
{
  unsigned char data_buf[DAQBOARD_WORD_SIZE];

  for (int i=0; i<DAQBOARD_WORD_SIZE; i++) {
    data_buf[i] = value & 0xff;
    value >>= 8;
  }

  if (SendData (ENDPOINT_WRITE_REG,data_buf,DAQBOARD_WORD_SIZE) != DAQBOARD_WORD_SIZE)
    return -1;
  return 0;
}


int TReadoutBoardDAQ::ReadAcknowledge() 
{ 
  unsigned char data_buf[2 * DAQBOARD_WORD_SIZE];
  uint32_t      headerword = 0, 
                dataword = 0;
  int           err;

  err=ReceiveData(ENDPOINT_READ_REG, data_buf, 2 * DAQBOARD_WORD_SIZE);

  if (err < 0) return -1;

  for (int i = 0; i < DAQBOARD_WORD_SIZE; i ++) {
    headerword += (data_buf[i                     ] << (8 * i));   // bytes 0 ... 3 are header
    dataword   += (data_buf[i + DAQBOARD_WORD_SIZE] << (8 * i));   // bytes 4 ... 7 are data
  }

  return 0;  
}



int TReadoutBoardDAQ::CurrentToADC (int current)
{
  float Result = (float) current / 100. * 4096. / 3.3;
  //std::cout << "Current to ADC, Result = " << Result << std::endl;
  return (int) Result;
}


float TReadoutBoardDAQ::ADCToSupplyCurrent (int value)
{
  float Result = (float) value * 3.3 / 4096.;   // reference voltage 3.3 V, full range 4096
  Result /= 0.1;    // 0.1 Ohm resistor
  Result *= 10;     // / 100 (gain) * 1000 (conversion to mA);
  return Result;
}


float TReadoutBoardDAQ::ADCToDacmonCurrent (int value)
{
  float Result = (float) value * (1e9 * 3.3);   // reference voltage 3.3 V, conversion to nA
  Result /= (5100 * 4096 * 6);                  // 5.1 kOhm res., ADC-range 4096, amplifier gain 6
  Result /= 10;                                 // gain of monitoring buffer
  return Result;
}


float TReadoutBoardDAQ::ADCToTemperature (int AValue) 
{
  float    Temperature, R;
  float    AVDD = 1.8;
  float    R2   = 5100;
  float    B    = 3900;
  float    T0   = 273.15 + 25;
  float    R0   = 10000;

  float Voltage = (float) AValue;
  Voltage       *= 3.3;
  Voltage       /= (1.8 * 4096);

  R           = (AVDD/Voltage) * R2 - R2;   // Voltage divider between NTC and R2
  Temperature = B / (log (R/R0) + B/T0);

  return Temperature;
}

bool TReadoutBoardDAQ::PowerOn (int &AOverflow) 
{

  // set current limits with voltages off
  WriteCurrentLimits(false, true); 
  // switch on voltages
  WriteCurrentLimits(true, true);

  // do like in old software:
  //std::cout << "Chip voltages off, setting current limits" << std::endl;
  //WriteRegister(0x100, 0x1126ce8b);
  //WriteRegister(0x101, 0xe8b);
  //WriteRegister(0x501, 0x67666664);
  //usleep(50000);

  //std::cout << "Switching chip voltages on" << std::endl;
  //WriteRegister(0x100, 0x1326ce8b);

  sleep(1); // sleep after PowerOn

  return ReadLDOStatus(AOverflow);
}


void TReadoutBoardDAQ::PowerOff () 
{
    // registers set in sequence similar to old software..
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

  spBoardConfigDAQ->SetDataPortSelect(0); // select no dataport
  WriteReadoutModuleConfigRegisters();

  spBoardConfigDAQ->SetDrstTime(0);          // TODO necessary?
  spBoardConfigDAQ->SetClockEnableTime(0);   // TODO necessary?
  spBoardConfigDAQ->SetSignalEnableTime(0);  // TODO necessary?
  spBoardConfigDAQ->SetAutoShutdownTime(1);  // TODO necessary?
  //WriteDelays();
  WriteResetModuleConfigRegisters();

  spBoardConfigDAQ->SetAutoShutdownEnable(1);
  spBoardConfigDAQ->SetLDOEnable(0);
  WriteADCModuleConfigRegisters();

}



void TReadoutBoardDAQ::ReadAllRegisters() {
  for (int i_module=0; i_module<8; ++i_module) {
    for (int i_reg=0; i_reg<8; ++i_reg) {
      uint32_t value = -1;
      uint16_t address = (i_module&0xf)<<8 | (i_reg&0xff);
      ReadRegister(address, value);
      std::cout << i_module << '\t' << i_reg << "\t0x" << std::hex << address << ":\t0x" << value << std::dec << std::endl;
    }
  }

}


void TReadoutBoardDAQ::DumpConfig(const char *fName, bool writeFile, char *config) {
  config[0] = '\0';
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();
  if (writeFile) {
    FILE *fp = fopen(fName, "w");
    fprintf(fp, "FIRMWARE  %i\n", ReadFirmwareVersion());
    fprintf(fp, "TRIGGERDELAY  %i\n", spBoardConfigDAQ->GetTriggerDelay()); // same as StrobeDelay on DAQboard
    fprintf(fp, "PULSEDELAY  %i\n", spBoardConfigDAQ->GetPulseDelay());
    fclose(fp);
  }
  
  sprintf(config, "FIRMWARE  0x%x\n", ReadFirmwareVersion());
  sprintf(config, "%sTRIGGERDELAY  %i\n", config, spBoardConfigDAQ->GetTriggerDelay());
  sprintf(config, "%sPULSEDELAY  %i\n", config, spBoardConfigDAQ->GetPulseDelay());
}


//---------------------------------------------------------
// methods related to data readout
//---------------------------------------------------------
int TReadoutBoardDAQ::GetEventBufferLength() {
  int buffer_length = 0;
  fMtx.lock();
  buffer_length = fEventBuffer.size();
  fMtx.unlock();
  
  return buffer_length;
}



//---------------------------------------------------------
// methods module by module
//---------------------------------------------------------

// ADC Module
//----------------------------------------------------------------------------


void TReadoutBoardDAQ::WriteADCModuleConfigRegisters() 
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();
    int limitDigital = CurrentToADC (spBoardConfigDAQ->GetCurrentLimitDigital());
  int limitIo      = CurrentToADC (spBoardConfigDAQ->GetCurrentLimitIo());
  int limitAnalogue= CurrentToADC (spBoardConfigDAQ->GetCurrentLimitAnalogue());

  // ADC config reg 0
  uint32_t config0 = 0;
  config0 |= ( limitDigital                              & 0xfff);
  config0 |= ((limitIo                                   & 0xfff) << 12);
  config0 |= ((spBoardConfigDAQ->GetAutoShutdownEnable() ?1:0)     << 24);
  config0 |= ((spBoardConfigDAQ->GetLDOEnable()          ?1:0)     << 25);
  config0 |= ((spBoardConfigDAQ->GetADCEnable()          ?1:0)     << 26);
  config0 |= ((spBoardConfigDAQ->GetADCSelfStop()        ?1:0)     << 27);
  config0 |= ((spBoardConfigDAQ->GetDisableTstmpReset()  ?1:0)     << 28);
  config0 |= ((spBoardConfigDAQ->GetPktBasedROEnableADC()?1:0)     << 29);
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG0, config0);

  // ADC config reg 1
  uint32_t config1 = 0;
  config1 |= ( limitAnalogue                              & 0xfff);
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG1, config1);

  // ADC config reg 2
  uint32_t config2 = 0;
  config2 |= ( spBoardConfigDAQ->GetAutoShutOffDelay()     & 0xfffff);
  config2 |= ( spBoardConfigDAQ->GetADCDownSamplingFact()  & 0xfff  << 20);
  WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG2, config2);

}


void TReadoutBoardDAQ::WriteCurrentLimits (bool ALDOEnable, bool AAutoshutdown) 
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

  //int limitDigital = CurrentToADC (spBoardConfigDAQ->GetCurrentLimitDigital());
  //int limitIo      = CurrentToADC (spBoardConfigDAQ->GetCurrentLimitIo());
  //int limitAnalog  = CurrentToADC (spBoardConfigDAQ->GetCurrentLimitAnalogue());

  spBoardConfigDAQ->SetAutoShutdownEnable(AAutoshutdown);  // keep track of settings in BoardConfig..
  spBoardConfigDAQ->SetLDOEnable(ALDOEnable);              // keep track of settings in BoardConfig..

  //uint32_t config0 = (((int) limitDigital) & 0xfff) | ((((int) limitIo) & 0xfff) << 12);
  //config0 |= ((AAutoshutdown?1:0) << 24);
  //config0 |= ((ALDOEnable       ?1:0) << 25);   
  //WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG0, config0);
  //uint32_t config1 = ((int) limitAnalog) & 0xfff;
  //WriteRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_CONFIG1, config1);

  WriteADCModuleConfigRegisters();
}


bool TReadoutBoardDAQ::ReadLDOStatus(int &AOverflow) 
{
  uint32_t ReadValue;
  bool     err, reg0, reg1, reg2;

  err  = ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  reg0 = ((ReadValue & 0x1000000) != 0); // LDO off if bit==0, on if bit==1
  err  = ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  reg1 = ((ReadValue & 0x1000000) != 0);
  err  = ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  reg2 = ((ReadValue & 0x1000000) != 0);

  err = ReadRegister((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_OVERFLOW, ReadValue);
  AOverflow = (int) ReadValue;

    if (! (reg0 & reg1 & reg2)) {
    std::cout << "GetLDOStatus, LDO status = " << reg0 << ", " << reg1 << ", " << reg2 << std::endl;
    }
    if ( err ) {
        // nothing to do
        // (this block is only intended to remove annoying warning at compilation
    }

  return ( reg0 & reg1 & reg2);
}


void TReadoutBoardDAQ::DecodeOverflow  (int AOverflow) {
  if (AOverflow & 0x1) {
    std::cout << "Overflow in digital current" << std::endl;
  }
  if (AOverflow & 0x2) {
    std::cout << "Overflow in digital I/O current" << std::endl;
  }
  if (AOverflow & 0x4) {
    std::cout << "Overflow in analogue current" << std::endl;
  }
}


float TReadoutBoardDAQ::ReadAnalogI() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToSupplyCurrent(Value);
}

float TReadoutBoardDAQ::ReadDigitalI() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  return ADCToSupplyCurrent(Value);
}

float TReadoutBoardDAQ::ReadIoI() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA2, ReadValue);
  int Value = (ReadValue) & 0xfff;

  return ADCToSupplyCurrent(Value);
}

float TReadoutBoardDAQ::ReadMonV() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  int Value = (ReadValue >> 12) & 0xfff;

  float Voltage = (float) Value;
  Voltage *= 3.3;
  Voltage /= (1.8 * 4096);
  return Voltage;
}


float TReadoutBoardDAQ::ReadMonI() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA1, ReadValue);
  int Value = (ReadValue) & 0xfff;

  return ADCToDacmonCurrent(Value);
}


float TReadoutBoardDAQ::ReadTemperature() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ADC << DAQBOARD_REG_ADDR_SIZE) + ADC_DATA0, ReadValue);
  //printf("NTC ADC: 0x%08X\n",Reading);
  int Value = (ReadValue) & 0xfff;

  return ADCToTemperature (Value);
}

bool TReadoutBoardDAQ::ReadMonitorRegisters(){
    ReadMonitorReadoutRegister();
    ReadMonitorTriggerRegister();
    return true; // added Caterina
}

bool TReadoutBoardDAQ::ReadMonitorReadoutRegister(){
    uint32_t value;
    int addr;
    addr = READOUT_MONITOR1 + (MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE);
    ReadRegister(addr, value);
    std::cout << "READOUT_MONITOR1 (0x" << std::hex << addr << "): 0x" << value << std::endl;
    std::cout << " READOUT SM 2: 0x"          << (value & 0x7)         << std::endl;
    std::cout << " EOT SM: 0x"                << ((value >> 3) & 0x7)  << std::endl;
    std::cout << " EOT COUNTER: 0x"           << ((value >> 6) & 0xff) << std::endl;
    std::cout << " TIMESTAMP FIFO EMPTY: "    << ((value >> 14) & 0x1) << std::endl;
    std::cout << " PACKET BASED FLAG: "       << ((value >> 15) & 0x1) << std::endl;
    std::cout << " FIFO 33 BIT EMPTY: "       << ((value >> 16) & 0x1) << std::endl;
    std::cout << " FIFO 32 BIT ALMOST FULL: " << ((value >> 17) & 0x1) << std::endl;
    std::cout << " SM READOUT 1: 0x"          << ((value >> 18) & 0x7) << std::endl;
    std::cout << " FIFO 9 BIT EMPTY: "        << ((value >> 21) & 0x1) << std::endl;
    std::cout << " SM READOUT 3: "            << ((value >> 22) & 0x1) << std::endl;
    std::cout << " FIFO 33 BIT FULL: "        << ((value >> 23) & 0x1) << std::endl;
    std::cout << " SM CTRL WORD DECODER: 0x"  << ((value >> 24) & 0x7) << std::endl;
    std::cout << " FIFO 32 BIT EMPTY: "       << ((value >> 27) & 0x1) << std::dec << std::endl;
    return true;
}

bool TReadoutBoardDAQ::ReadMonitorTriggerRegister(){
    uint32_t value;
    int addr;
    addr = TRIG_MONITOR1 + (MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE);
    WriteRegister(addr,0xaaa);
    ReadRegister(addr, value);
    std::cout <<"TRIG_MONITOR1 (0x" << std::hex << addr <<"): 0x" << value << std::endl;
    std::cout <<" TRIGGER SM: 0x"            << (value & 0x7) << std::endl;
    std::cout <<" BUSY SM: 0x"               << ((value >> 3) & 0x3) << std::endl;
    std::cout <<" STOP TRIGGER COUNTER: 0x"  << ((value >> 5) & 0xff) << std::endl;
    std::cout <<" START TRIGGER COUNTER: 0x" << ((value >> 13) & 0xff) << std::endl;
    std::cout <<" BUSY IN: "                 << ((value >> 21) & 0x1) << std::endl;
    std::cout <<" BUSY PALPIDE: "            << ((value >> 22) & 0x1) << std::endl;
    std::cout <<" CONTROL WORD BUSY: "       << ((value >> 23) & 0x1) << std::endl;
    std::cout <<" BUSY EVENT BUILDER: "      << ((value >> 24) & 0x1) << std::endl;
    std::cout <<" BUSY OVERRIDE FLAG: "      << ((value >> 25) & 0x1) << std::endl;
    std::cout <<" BUSY : "                   << ((value >> 26) & 0x1) << std::dec << std::endl;
    return true;
}








// READOUT Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteReadoutModuleConfigRegisters() 
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

  // Event builder config reg 0
  uint32_t config = 0;
  config |= ( spBoardConfigDAQ->GetMaxDiffTriggers()       & 0xf);
  config |= ((spBoardConfigDAQ->GetSamplingEdgeSelect()?1:0)         <<  4);
  config |= ((spBoardConfigDAQ->GetPktBasedROEnable()?  1:0)         <<  5);
  config |= ((spBoardConfigDAQ->GetDDREnable()?         1:0)         <<  6);
  config |= ((spBoardConfigDAQ->GetDataPortSelect()           & 0x3) <<  7);
  config |= ((spBoardConfigDAQ->GetFPGAEmulationMode()        & 0x3) <<  9);
  config |= ((spBoardConfigDAQ->GetHeaderType()?        1:0)         << 11);
  config |= ((spBoardConfigDAQ->GetParamValue("BOARDVERSION") & 0x1) << 12);

  //std::cout << "FPGAEmulationMode: " << spBoardConfigDAQ->GetFPGAEmulationMode() << std::endl;

  WriteRegister ((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_EVENTBUILDER_CONFIG, config);

}

bool TReadoutBoardDAQ::ResyncSerialPort ()
{
  return WriteRegister((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_RESYNC, 0x0);
}

bool TReadoutBoardDAQ::WriteSlaveDataEmulatorReg(uint32_t AWord) {
  AWord &= 0xffffffff;
  return WriteRegister((MODULE_READOUT << DAQBOARD_REG_ADDR_SIZE) + READOUT_SLAVE_DATA_EMULATOR, AWord);
}


// TRIGGER Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteTriggerModuleConfigRegisters() 
{
  bool err;
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

    //  busy config reg
  uint32_t config0 = 0;
  config0 |= spBoardConfigDAQ->GetBusyDuration();
  WriteRegister ((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_DURATION, config0);

  // trigger conif reg
  uint32_t config1 = 0;
  config1 |= ( spBoardConfigDAQ->GetNTriggers()          & 0xffff);
  config1 |= ((spBoardConfigDAQ->GetTriggerMode()        &  0x7)  << 16);
  config1 |= ((spBoardConfigDAQ->GetStrobeDuration()     & 0xff)  << 19);
  config1 |= ((spBoardConfigDAQ->GetBusyConfig()         &  0x7)  << 27);
  //std::cout << "config1: " << std::hex << config1 << std::dec << std::endl;
  WriteRegister ((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_TRIGGER_CONFIG, config1);

  //  strobe delay config reg
  //std::cout << spBoardConfigDAQ->GetStrobeDelay() << std::endl;
  uint32_t config2 = 0;
  config2 |= spBoardConfigDAQ->GetStrobeDelay();
  //std::cout << "config2: " << std::hex << config2 << std::dec << std::endl;
  err = WriteRegister ((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_DELAY, config2);
  //std::cout << err << std::endl;
  //  busy override config reg
  uint32_t config3 = 0;
  config3 |= (spBoardConfigDAQ->GetBusyOverride()?1:0);
  //std::cout << "config3: " << std::hex << config3 << std::dec << std::endl;
  WriteRegister ((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_OVERRIDE, config3);
    if ( err ) {
        // nothing to do
        // (this block is only intended to remove annoying warning at compilation
    }

}



bool TReadoutBoardDAQ::StartTrigger()
{
  return WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_START, 13);
}


bool TReadoutBoardDAQ::StopTrigger ()
{
  return WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_STOP, 13);
}


bool TReadoutBoardDAQ::WriteBusyOverrideReg(bool ABusyOverride)
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();
    spBoardConfigDAQ->SetBusyOverride(ABusyOverride);
  bool err;
  err = WriteRegister((MODULE_TRIGGER << DAQBOARD_REG_ADDR_SIZE) + TRIG_BUSY_OVERRIDE, ABusyOverride);
  if (!err) return false;

  return err;
}



// CMU Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteCMUModuleConfigRegisters () 
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

    //  CMU config reg
  uint32_t config = 0;
  config |= ( spBoardConfigDAQ->GetManchesterDisable()     ?1:0);
  config |= ((spBoardConfigDAQ->GetSamplingEdgeSelectCMU() ?1:0)       << 1);
  config |= ((spBoardConfigDAQ->GetInvertCMUBus()          ?1:0)       << 2);
  config |= ((spBoardConfigDAQ->GetChipMaster()            ?1:0)       << 3);
  WriteRegister ((MODULE_CMU << DAQBOARD_REG_ADDR_SIZE) + CMU_CONFIG, config);
}



// RESET Module
//----------------------------------------------------------------------------

void TReadoutBoardDAQ::WriteResetModuleConfigRegisters () 
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();
  //  PULSE DRST PRST duration reg
  uint32_t config0 = 0;
  config0 |= ( spBoardConfigDAQ->GetPRSTDuration()        & 0xff );
  config0 |= ((spBoardConfigDAQ->GetDRSTDuration()        & 0xff)       << 8);
  config0 |= ((spBoardConfigDAQ->GetPULSEDuration()       & 0xffff)     << 16);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DURATION, config0);

  // power up sequencer delay register
  uint32_t config1 = ((spBoardConfigDAQ->GetDrstTime()        & 0xff) << 24)
    | ((spBoardConfigDAQ->GetSignalEnableTime() & 0xff) << 16)
    | ((spBoardConfigDAQ->GetClockEnableTime()  & 0xff) << 8)
    | ( spBoardConfigDAQ->GetAutoShutdownTime() & 0xff);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DELAYS, config1);  

  // PULSE STROBE delay sequence reg
  uint32_t config2 = 0;
  //std::cout << "PulseDelay: " << spBoardConfigDAQ->GetPulseDelay() << std::endl;
  config2 |= ( spBoardConfigDAQ->GetPulseDelay()          & 0xffff );
  config2 |= ((spBoardConfigDAQ->GetStrobePulseSeq()      & 0x3)     << 16);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_PULSE_DELAY, config2);

  // Power On Reset disable reg
  uint32_t config3 = 0;
  config3 |= ( spBoardConfigDAQ->GetPORDisable()     ?1:0);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_POR_DISABLE, config3);
}


void TReadoutBoardDAQ::WriteDelays () 
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

  uint32_t delays = ((spBoardConfigDAQ->GetDrstTime()         & 0xff) << 24)
    | ((spBoardConfigDAQ->GetSignalEnableTime() & 0xff) << 16)
    | ((spBoardConfigDAQ->GetClockEnableTime()  & 0xff) << 8)
    | ( spBoardConfigDAQ->GetAutoShutdownTime() & 0xff);
  WriteRegister ((MODULE_RESET << DAQBOARD_REG_ADDR_SIZE) + RESET_DELAYS, delays);  
}



// ID Module
//----------------------------------------------------------------------------

int TReadoutBoardDAQ::ReadBoardAddress() 
{
  uint32_t ReadValue;
  ReadRegister ((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_ADDRESS, ReadValue);
  int BoardAddress = ReadValue & 0xff;

  return BoardAddress;

}


uint32_t TReadoutBoardDAQ::ReadFirmwareVersion() 
{
  ReadRegister ((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, fFirmwareVersion);

  return fFirmwareVersion;
}


uint32_t TReadoutBoardDAQ::ReadFirmwareDate() 
{
  if (fFirmwareVersion==0) { 
    ReadRegister ((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, fFirmwareVersion);
  }

  return (fFirmwareVersion & 0xffffff);
}


int TReadoutBoardDAQ::ReadFirmwareChipVersion() 
{
  if (fFirmwareVersion==0) { 
    ReadRegister ((MODULE_ID << DAQBOARD_REG_ADDR_SIZE) + ID_FIRMWARE, fFirmwareVersion);
  }
  return ((fFirmwareVersion & 0xf0000000) >> 28);
}



// SOFTRESET Module
//----------------------------------------------------------------------------
void TReadoutBoardDAQ::WriteSoftResetModuleConfigRegisters () 
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

    //  PULSE DRST PRST duration reg
  uint32_t config = 0;
  config |= ( spBoardConfigDAQ->GetSoftResetDuration()        & 0xff );
  WriteRegister ((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_DURATION, config);

}


bool TReadoutBoardDAQ::ResetBoardFPGA (int ADuration)
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

  spBoardConfigDAQ->SetSoftResetDuration(ADuration); // keep track of latest config in TBoardConfigDAQ
  bool err;
  err = WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_DURATION, ADuration);
  if (!err) return false;
  return WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_FPGA_RESET, 13);
}


bool TReadoutBoardDAQ::ResetBoardFX3 (int ADuration)
{
    shared_ptr<TBoardConfigDAQ> spBoardConfigDAQ = fBoardConfigDAQ.lock();

  spBoardConfigDAQ->SetSoftResetDuration(ADuration); // keep track of latest config in TBoardConfigDAQ
  bool err;
  err = WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_DURATION, ADuration);
  if (!err) return false;
  return WriteRegister((MODULE_SOFTRESET << DAQBOARD_REG_ADDR_SIZE) + SOFTRESET_FX3_RESET, 13);
}








