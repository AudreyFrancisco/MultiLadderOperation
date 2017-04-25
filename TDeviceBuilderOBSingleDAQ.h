#ifndef DEVICEBUILDER_OB_SINGLE_DAQ_H
#define DEVICEBUILDER_OB_SINGLE_DAQ_H

/// \class TDeviceBuilderOBSingleDAQ
/// \brief Instantiates objects in TDevice class: single chip in OB mode read by DAQ board
///
/// This class provide specific procedures to instantiante the objects in the TDevice
/// class in order to get a suitable configuration for a single chip in OB mode read by
/// a DAQ board.

#include "TDeviceBuilderWithDAQBoards.h"

class TDeviceBuilderOBSingleDAQ : public TDeviceBuilderWithDAQBoards {
    
public:
#pragma mark - Constructors/destructor
    TDeviceBuilderOBSingleDAQ();
    virtual ~TDeviceBuilderOBSingleDAQ();
    
#pragma mark - Device creation and initialisation
    void SetDeviceType( const TDeviceType dt );
    void CreateDeviceConfig();
    void InitSetup();
    
};

#endif
