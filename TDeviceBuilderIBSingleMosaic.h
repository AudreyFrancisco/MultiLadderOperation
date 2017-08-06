#ifndef DEVICEBUILDER_IB_SINGLE_MOSAIC_H
#define DEVICEBUILDER_IB_SINGLE_MOSAIC_H

/**
 * \class TDeviceBuilderIBSingleMosaic
 *
 * \brief Instantiates objects in TDevice class: single chip in IB mode read by MOSAIC
 *
 * \author Andry Rakotozafindrabe
 *
 * This class provide specific procedures to instantiante the objects in the TDevice
 * class in order to get a suitable configuration for a single chip in IB mode read by
 * a MOSAIC board.
 *
 * \note
 * The code in TDeviceBuilderIBSingleMosaic::CreateDeviceConfig() and
 * TDeviceBuilderIBSingleMosaic::InitSetup() is inspired from the (obsolete) class TConfig
 * and the functions in (obsolete) SetupHelpers.
 */

#include "TDeviceBuilder.h"

class TDeviceBuilderIBSingleMosaic : public TDeviceBuilder {
    
public:
    #pragma mark - Constructors/destructor
    TDeviceBuilderIBSingleMosaic();
    virtual ~TDeviceBuilderIBSingleMosaic();
    
    #pragma mark - Device creation and initialisation
    void SetDeviceType( const TDeviceType dt );
    void CreateDeviceConfig();
    void InitSetup();
    
};

#endif
