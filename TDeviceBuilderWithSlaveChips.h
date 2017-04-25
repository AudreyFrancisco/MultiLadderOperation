#ifndef DEVICEBUILDER_WITH_SLAVE_CHIPS_H
#define DEVICEBUILDER_WITH_SLAVE_CHIPS_H

/// \class TDeviceBuilderWithSlaveChips
/// \brief This class adds methods to build a TDevice object that contain slave chips.
///
/// This is still an abstract class since it does not define the pure virtual methods
/// of its base class. The inherited classes implement them and provide specific
/// procedures relevant to instantiante the objects in the TDevice class depending
/// on the TDevice type that is wanted in the end (half-stave or OB module).

#include "TDeviceBuilder.h"

class TDeviceBuilderWithSlaveChips : public TDeviceBuilder {

protected:
    #pragma mark - other protected methods
    void EnableSlave( const int mychip );
    void MakeDaisyChain();

public:
    #pragma mark - Constructors/destructor
    TDeviceBuilderWithSlaveChips();
    virtual ~TDeviceBuilderWithSlaveChips();

};

#endif
