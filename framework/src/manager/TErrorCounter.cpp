#include "TErrorCounter.h"
#include <iostream>

using namespace std;

//___________________________________________________________________
TErrorCounter::TErrorCounter() :
fN8b10b( 0 ),
fNCorruptEvent( 0 ),
fNPrioEncoder( 0 ),
fNTimeout( 0 )
{
    
}

//___________________________________________________________________
TErrorCounter::~TErrorCounter()
{
    
}

//___________________________________________________________________
void TErrorCounter::Dump()
{
    cout << "------------------------------- TErrorCounter::Dump() " << endl;
    cout << "Number of 8b10b decoder errors: " << std::dec << fN8b10b << endl;
    cout << "Number of corrupted events: " << fNCorruptEvent << endl;
    cout << "Number of priority encoder errors: " << fNPrioEncoder << endl;
    cout << "Number of timeout: " << fNTimeout << endl;
    cout << "-------------------------------" << endl;
}
