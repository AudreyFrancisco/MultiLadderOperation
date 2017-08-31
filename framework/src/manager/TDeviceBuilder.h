#ifndef DEVICEBUILDER_H
#define DEVICEBUILDER_H

/** 
 * \class TDeviceBuilder
 *
 * \brief This class instantiates the objects in the TDevice class.
 *
 * \author Andry Rakotozafindrabe
 *
 * This is an abstract class composed with aTDevice object. The inherited classes
 * provide specific procedures relevant to instantiante the objects in the TDevice class
 * depending on the TDevice type that is wanted in the end.
 *
 * \note
 * This class re-uses most of the code written in (obsolete) SetupHelpers to
 * check the control interface, and in (obsolete) TConfig class to set the device
 * parameter value.
 */

#include <memory>
#include "TVerbosity.h"

enum class TDeviceType;
class TDevice;

class TDeviceBuilder : public TVerbosity {

protected:
    std::shared_ptr<TDevice> fCurrentDevice;
    
protected:
    void CheckControlInterface();
    void CountEnabledChipsPerBoard();
    void PropagateVerbosityToBoards();
    
public:
    #pragma mark - Constructors/destructor
    TDeviceBuilder();
    virtual ~TDeviceBuilder();

    #pragma mark - Device creation and initialisation
    void CreateDevice();
    virtual void SetDeviceType( const TDeviceType dt );
    virtual void CreateDeviceConfig() = 0;
    void SetDeviceParamValue( const char *Name, const char *Value, int Chip );
    virtual void SetVerboseLevel( const int level );
    virtual void InitSetup() = 0;
    
    #pragma mark - Getters
    std::shared_ptr<TDevice> GetCurrentDevice() { return fCurrentDevice; }

};

#endif
