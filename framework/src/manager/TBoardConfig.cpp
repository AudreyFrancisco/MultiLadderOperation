#include "TBoardConfig.h"
#include <stdio.h>
#include <iostream>

using namespace std;

//___________________________________________________________________
TBoardConfig::TBoardConfig()
{
  fTriggerDelay  = fSTROBEDELAY;
  fPulseDelay    = fPULSEDELAY;
  fTriggerSource = (int)TTriggerSource::kTRIG_INT;
}


//___________________________________________________________________
void TBoardConfig::InitParamMap()
{
  fSettings["STROBEDELAYBOARD"] = &fTriggerDelay;
  fSettings["PULSEDELAY"]       = &fPulseDelay;
  fSettings["TRIGGERSOURCE"]    = &fTriggerSource;
}


//___________________________________________________________________
bool TBoardConfig::SetParamValue (const char *Name, const char *Value)
{
    if (fSettings.find (Name) != fSettings.end()) {
        sscanf (Value, "%d", fSettings.find(Name)->second);
        return true;
    }
    cerr << "TBoardConfig::SetParamValue() - Unknown parameter `-" << Name << "`" << endl;
    return false;
}

//___________________________________________________________________
bool TBoardConfig::SetParamValue(const char *Name, const int Value)
{
    if (fSettings.find (Name) != fSettings.end()) {
        *(fSettings.find(Name)->second) = Value;
        return true;
    }
    cerr << "TBoardConfig::SetParamValue() - Unknown parameter `-" << Name << "`" << endl;
    return false;
}

//___________________________________________________________________
int TBoardConfig::GetParamValue (const char *Name)
{
  if (fSettings.find (Name) != fSettings.end()) {
    return *(fSettings.find(Name)->second);
  }
  return -1;
}

