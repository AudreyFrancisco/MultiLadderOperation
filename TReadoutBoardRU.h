#ifndef TREADOUTBOARDRU_H
#define TREADOUTBOARDRU_H

#include "TReadoutBoard.h"
#include "TConfig.h"
#include "TBoardConfigRU.h"
#include "USB.h"

#include <memory>

#include "ReadoutUnitSrc/UsbDev.hpp"

class TReadoutBoardRU : public TReadoutBoard {
private:
  static const int VID = 0x04B4;
  static const int PID = 0x0008;
  static const int INTERFACE_NUMBER = 2;
  static const uint8_t EP_CTL_OUT = 3;
  static const uint8_t EP_CTL_IN = 3;
  static const uint8_t EP_DATA0_IN = 4;
  static const uint8_t EP_DATA1_IN = 5;

    static const size_t USB_TIMEOUT = 1000;
    static const int MAX_RETRIES_READ = 5;

    // Module Addresses
    static const uint16_t MODULE_DCTRL = 4;

  std::shared_ptr<UsbDev> m_usb;


    struct ReadResult {
        uint16_t address;
        uint16_t data;
        bool error;
    };

    void registeredWrite(uint16_t module, uint16_t address, uint16_t data);
    void registeredRead(uint16_t module, uint16_t address);
    bool flush();
    void readFromPort(uint8_t port, size_t size, UsbDev::DataBuffer &buffer);
    std::vector<ReadResult> readResults();

    UsbDev::DataBuffer m_buffer;
    uint32_t m_readBytes;

    bool m_logging;

public:
  TReadoutBoardRU(libusb_device *ADevice, TBoardConfigRU *config);

  virtual int WriteChipRegister(uint16_t Address, uint16_t Value,
                                uint8_t chipId = 0);
  virtual int ReadRegister(uint16_t Address, uint32_t &Value);
  virtual int WriteRegister(uint16_t Address, uint32_t Value);
  virtual int ReadChipRegister(uint16_t Address, uint16_t &Value,
                               uint8_t chipID = 0);
  virtual int SendOpCode(uint16_t OpCode);
  virtual int SendOpCode(uint16_t OpCode, uint8_t chipId);

  virtual int SetTriggerConfig(bool enablePulse, bool enableTrigger,
                               int triggerDelay, int pulseDelay);
  virtual void SetTriggerSource(TTriggerSource triggerSource);
  virtual int Trigger(int nTriggers);
  virtual int ReadEventData(int &NBytes, unsigned char *Buffer);
};

#endif // TREADOUTBOARDRU_H
