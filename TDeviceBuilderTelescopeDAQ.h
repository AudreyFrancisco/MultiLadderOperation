#ifndef DEVICEBUILDER_TELESCOPE_DAQ_H
#define DEVICEBUILDER_TELESCOPE_DAQ_H

/**
* \class TDeviceBuilderTelescopeDAQ
*
* \brief Instantiates objects in TDevice class: chips in OB mode read by DAQ boards
*
* \author Andry Rakotozafindrabe
*
* This class provide specific procedures to instantiante the objects in the TDevice
* class in order to get a suitable configuration for several chips in OB mode, each
* one being read by a DAQ board.
*
* \note
* The code in TDeviceBuilderTelescope::CreateDeviceConfig() and
* TDeviceBuilderTelescope::InitSetup() is inspired from the (obsolete) class TConfig
* and the functions in (obsolete) SetupHelpers.
*/

#include "TDeviceBuilderWithDAQBoards.h"

enum class TDeviceType;

class TDeviceBuilderTelescope : public TDeviceBuilderWithDAQBoards {
    
public:
#pragma mark - Constructors/destructor
    TDeviceBuilderTelescope();
    virtual ~TDeviceBuilderTelescope();
    
#pragma mark - Device creation and initialisation
    void SetDeviceType( const TDeviceType dt );
    void SetNChips( const int number );
    void CreateDeviceConfig();
    void InitSetup();
    
};

#endif
