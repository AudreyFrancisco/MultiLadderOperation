#ifndef DEVICEBUILDER_MFT_LADDER_H
#define DEVICEBUILDER_MFT_LADDER_H

/**
 * \class TDeviceBuilderMFTLadder
 *
 * \brief This class instantiates the objects in the TDevice class for an MFT ladder.
 *
 * \author Andry Rakotozafindrabe
 *
 * This class provide specific procedures to instantiante the objects in the TDevice
 * class in order to get an MFT-ladder-like configuration read by a MOSAIC board. When
 * the setup is initialised, a systematic check of the whole system is conducted in
 * order to disable non-working chips in case of device with multiple chips.
 *
 */

#include "TDeviceBuilder.h"

class TDeviceBuilderMFTLadder : public TDeviceBuilder {
    
public:
#pragma mark - Constructors/destructor
    TDeviceBuilderMFTLadder();
    virtual ~TDeviceBuilderMFTLadder();
    
#pragma mark - Device creation and initialisation
    void SetDeviceType( const TDeviceType dt );
    virtual void SetVerboseLevel( const int level );
    void SetDeviceId( const unsigned int number );
    void CreateDeviceConfig();
    void InitSetup();
    
};

#endif
