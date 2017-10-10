#ifndef DEVICEBUILDER_WITH_DAQ_BOARDS_H
#define DEVICEBUILDER_WITH_DAQ_BOARDS_H

/**
 * \class TDeviceBuilderWithDAQBoards
 *
 * \brief This class adds methods to build a TDevice object with DAQ board(s) readout.
 *
 * \author Andry Rakotozafindrabe
 *
 * This is still an abstract class since it does not define the pure virtual methods
 * of its base class. The inherited classes implement them and provide specific
 * procedures relevant to instantiante the objects in the TDevice class depending
 * on the TDevice type that is wanted in the end, as long as the readout is done
 * with DAQ board(s).
 *
 * \note
 * This class re-use the code from the functions of the (obsolete) USBHelpers.
 */

#include <libusb-1.0/libusb.h>
#include "TDeviceBuilder.h"

class TDeviceBuilderWithDAQBoards : public TDeviceBuilder {

    static struct libusb_context* fContext;

protected:
    #pragma mark - Specific to DAQ board settings
    void AddDAQBoard( libusb_device* device );
    void FindDAQBoards();
    void InitLibUsb();
    bool IsDAQBoard( libusb_device* device );
    void PowerOnDaqBoard( std::shared_ptr<TReadoutBoardDAQ> aDAQBoard );
    
public:
    #pragma mark - Constructors/destructor
    TDeviceBuilderWithDAQBoards();
    virtual ~TDeviceBuilderWithDAQBoards();
    
};

#endif
