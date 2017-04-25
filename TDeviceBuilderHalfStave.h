#ifndef DEVICEBUILDER_HALFSTAVE_H
#define DEVICEBUILDER_HALFSTAVE_H

/// \class TDeviceBuilderHalfStave
/// \brief This class instantiates the objects in the TDevice class for a half stave.
///
/// This class provide specific procedures to instantiante the objects in the TDevice
/// class in order to get a half-stave-like configuration read by a MOSAIC board. When
/// the setup is initialised, a systematic check of the whole system is conducted in
/// order to disable non-working chips in case of device with multiple chips.

#include "TDeviceBuilderWithSlaveChips.h"

class TDeviceBuilderHalfStave : public TDeviceBuilderWithSlaveChips {
    
public:
    #pragma mark - Constructors/destructor
    TDeviceBuilderHalfStave();
    virtual ~TDeviceBuilderHalfStave();
    
    #pragma mark - Device creation and initialisation
    void SetNModules( const int number );
    void CreateDeviceConfig();
    void InitSetup();

};

#endif
