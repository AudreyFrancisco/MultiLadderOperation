#ifndef DEVICEBUILDER_OB_H
#define DEVICEBUILDER_OB_H

/**
 * \class TDeviceBuilderOB
 *
 * \brief This class instantiates the objects in the TDevice class for an OB stave.
 *
 * \author Andry Rakotozafindrabe
 *
 * This class provide specific procedures to instantiante the objects in the TDevice
 * class in order to get a OB-like configuration read by a MOSAIC board. When
 * the setup is initialised, a systematic check of the whole system is conducted in
 * order to disable non-working chips in case of device with multiple chips.
 *
 * \note
 * The code in TDeviceBuilderOB::CreateDeviceConfig() and
 * TDeviceBuilderOB::InitSetup() is inspired from the (obsolete) class TConfig
 * and the functions in (obsolete) SetupHelpers.
 */
#include "TDeviceBuilderWithSlaveChips.h"

class TDeviceBuilderOB : public TDeviceBuilderWithSlaveChips {
    
    static const int DEFAULT_MODULE_ID = 1;

public:
    #pragma mark - Constructors/destructor
    TDeviceBuilderOB();
    virtual ~TDeviceBuilderOB();
    
    #pragma mark - Device creation and initialisation
    void SetDeviceType( const TDeviceType dt );
    void CreateDeviceConfig();
    void InitSetup();
    
};

#endif
