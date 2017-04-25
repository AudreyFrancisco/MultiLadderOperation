#ifndef DEVICEBUILDER_H
#define DEVICEBUILDER_H

/// \class TDeviceBuilder
/// \brief This class instantiates the objects in the TDevice class.
///
/// This is an abstract class composed with aTDevice object. The inherited classes
/// provide specific procedures relevant to instantiante the objects in the TDevice class
/// depending on the TDevice type that is wanted in the end.

#include <memory>
#include "TVerbosity.h"

enum class TDeviceType;
class TDevice;

class TDeviceBuilder : public TVerbosity {

protected:
    std::shared_ptr<TDevice> fCurrentDevice;
    
protected:
    void CheckControlInterface();

public:
    #pragma mark - Constructors/destructor
    TDeviceBuilder();
    virtual ~TDeviceBuilder();

    #pragma mark - Device creation and initialisation
    void CreateDevice();
    virtual void SetDeviceType( const TDeviceType dt );
    virtual void CreateDeviceConfig() = 0;
    void SetDeviceParamValue( const char *Name, const char *Value, int Chip );
    virtual void InitSetup() = 0;
    
    #pragma mark - Propagate verbosity down to the TDevice
    void SetVerboseLevel( const int level );

    #pragma mark - Getters
    std::shared_ptr<TDevice> GetCurrentDevice() { return fCurrentDevice; }

};

#endif
