#include <stdlib.h>
#include "TVerbosity.h"

#pragma mark - Constructors/destructor

//___________________________________________________________________
TVerbosity::TVerbosity() :
    fVerboseLevel( kTERSE )
{ }

//___________________________________________________________________
TVerbosity::~TVerbosity()
{ }

//___________________________________________________________________
void TVerbosity::SetVerboseLevel( const int level )
{
    fVerboseLevel = level;
}
