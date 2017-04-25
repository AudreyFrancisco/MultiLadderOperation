#include <iostream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include "USB.h"
#include "TDevice.h"
#include "TDeviceBuilderWithDAQBoards.h"
#include "TReadoutBoardDAQ.h"

using namespace std;

#pragma mark - Constructors/destructor

//___________________________________________________________________
TDeviceBuilderWithDAQBoards::TDeviceBuilderWithDAQBoards() : TDeviceBuilder()
{ }


//___________________________________________________________________
TDeviceBuilderWithDAQBoards::~TDeviceBuilderWithDAQBoards()
{ }


#pragma mark - Specific to DAQ board settings

//___________________________________________________________________
void TDeviceBuilderWithDAQBoards::AddDAQBoard( shared_ptr<libusb_device> device )
{
    // note: this should change to use the correct board config according to index or geographical id
    shared_ptr<TBoardConfigDAQ> boardConfig = dynamic_pointer_cast<TBoardConfigDAQ>(fCurrentDevice->GetBoardConfig(0));
    auto readoutBoard = make_shared<TReadoutBoardDAQ>(device, boardConfig);
    try {
        fCurrentDevice->AddBoard(readoutBoard);
    } catch ( std::runtime_error &err ) {
        throw err;
    }
}

//___________________________________________________________________
void TDeviceBuilderWithDAQBoards::FindDAQBoards()
{
    if ( fCurrentDevice->IsSetupFrozen() ) {
        return;
    }
    
    libusb_device **list;
    
    if ( DeviceBuilder::fContext == 0 ) {
        throw runtime_error( "TDeviceBuilderWithDAQBoards::FindDAQBoards() - Error, libusb not initialised." );
    }
    ssize_t cnt = libusb_get_device_list( DeviceBuilder::fContext, &list );
    
    if ( cnt < 0 ) {
        throw runtime_error( "TDeviceBuilderWithDAQBoards::FindDAQBoards() - Error getting device list." );
    }
    
    for ( ssize_t i = 0; i < cnt; i++ ) {
        shared_ptr<libusb_device> device( list[i] );
        if ( IsDAQBoard(device) ) {
            try {
                AddDAQBoard( device );
            } catch ( std::runtime_error &err ) {
                cerr << err.what() << endl;
                libusb_free_device_list(list, 1);
                throw runtime_error( "TDeviceBuilderWithDAQBoards::FindDAQBoards() - Problem adding DAQ board." );
            }
        }
    }
    if ( fVerboseLevel ) {
        cout << "TDeviceBuilderWithDAQBoards::FindDAQBoards() - Found "
             << fCurrentDevice->GetNBoards() << " DAQ boards" << endl;
    }
    libusb_free_device_list(list, 1);
}

//___________________________________________________________________
void TDeviceBuilderWithDAQBoards::InitLibUsb()
{
    int err = libusb_init( &DeviceBuilder::fContext );
    if (err) {
        cerr << "TDeviceBuilderWithDAQBoards::InitLibUsb() - Error " << err << endl;
        throw runtime_error( "TDeviceBuilderWithDAQBoards::InitLibUsb() - Error while trying to init libusb." );
    }
}

//___________________________________________________________________
bool TDeviceBuilderWithDAQBoards::IsDAQBoard( shared_ptr<libusb_device> device )
{
    libusb_device_descriptor desc;
    libusb_get_device_descriptor(device.get(), &desc);
    
    // std::cout << std::hex << "Vendor id " << (int)desc.idVendor << ", Product id " << (int)desc.idProduct << std::dec << std::endl;
    
    if ((desc.idVendor == DAQ_BOARD_VENDOR_ID) && (desc.idProduct == DAQ_BOARD_PRODUCT_ID)) {
        //std::cout << "Serial number " << (int)desc.iSerialNumber << std::endl;
        return true;
    }
    
    return false;
}

//___________________________________________________________________
void TDeviceBuilderWithDAQBoards::PowerOnDaqBoard( shared_ptr<TReadoutBoardDAQ> aDAQBoard )
{
    int overflow;
    
    if ( aDAQBoard->PowerOn(overflow) ) cout << "LDOs are on" << endl;
    else cout << "LDOs are off" << endl;
    cout << "Version = " << std::hex << aDAQBoard->ReadFirmwareVersion()
    << std::dec << endl;
    aDAQBoard->SendOpCode(Alpide::OPCODE_GRST);
    //sleep(1); // sleep necessary after GRST? or PowerOn?
    
    cout << "Analog Current  = " << aDAQBoard->ReadAnalogI()     << endl;
    cout << "Digital Current = " << aDAQBoard->ReadDigitalI()    << endl;
    cout << "Temperature     = " << aDAQBoard->ReadTemperature() << endl;
}
