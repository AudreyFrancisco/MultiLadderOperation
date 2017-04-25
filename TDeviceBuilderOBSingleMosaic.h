#ifndef DEVICEBUILDER_OB_SINGLE_MOSAIC_H
#define DEVICEBUILDER_OB_SINGLE_MOSAIC_H

/// \class TDeviceBuilderOBSingleMosaic
/// \brief Instantiates objects in TDevice class: single chip in OB mode read by MOSAIC
///
/// This class provide specific procedures to instantiante the objects in the TDevice
/// class in order to get a suitable configuration for a single chip in OB mode read by
/// a MOSAIC board.

#include "TDeviceBuilder.h"

class TDeviceBuilderOBSingleMosaic : public TDeviceBuilder {
    
public:
#pragma mark - Constructors/destructor
    TDeviceBuilderOBSingleMosaic();
    virtual ~TDeviceBuilderOBSingleMosaic();
    
#pragma mark - Device creation and initialisation
    void SetDeviceType( const TDeviceType dt );
    void CreateDeviceConfig();
    void InitSetup();
    
};

#endif
