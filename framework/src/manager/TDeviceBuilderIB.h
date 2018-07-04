#ifndef DEVICEBUILDER_IB_H
#define DEVICEBUILDER_IB_H

/**
 * \class TDeviceBuilderIB
 *
 * \brief This class instantiates the objects in the TDevice class for an IB stave.
 *
 * \author Andry Rakotozafindrabe
 *
 * This class provide specific procedures to instantiante the objects in the TDevice
 * class in order to get a IB-like configuration read by a MOSAIC board. When
 * the setup is initialised, a systematic check of the whole system is conducted in
 * order to disable non-working chips in case of device with multiple chips.
 *
 * \note
 * The code in TDeviceBuilderIB::CreateDeviceConfig() and
 * TDeviceBuilderIB::InitSetup() is inspired from the (obsolete) class TConfig
 * and the functions in (obsolete) SetupHelpers.
 */

#include "TDeviceBuilder.h"

class TDeviceBuilderIB : public TDeviceBuilder {
    
public:
    #pragma mark - Constructors/destructor
    TDeviceBuilderIB();
    virtual ~TDeviceBuilderIB();
    
    #pragma mark - Device creation and initialisation
    void SetDeviceType( const TDeviceType dt );
    virtual void SetVerboseLevel( const int level );
    void CreateDeviceConfig();
    void InitSetup();
    
};

#endif
