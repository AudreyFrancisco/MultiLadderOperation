#include "TBoardConfig.h"
#include <stdio.h>

using namespace std;

//___________________________________________________________________
TBoardConfig::TBoardConfig()
{
  fTriggerDelay = fSTROBEDELAY;
  fPulseDelay   = fPULSEDELAY;
}


//___________________________________________________________________
void TBoardConfig::InitParamMap()
{
  fSettings["STROBEDELAYBOARD"] = &fTriggerDelay;
  fSettings["PULSEDELAY"]       = &fPulseDelay;
}


//___________________________________________________________________
bool TBoardConfig::SetParamValue (const char *Name, const char *Value)
{
  if (fSettings.find (Name) != fSettings.end()) {
    sscanf (Value, "%d", fSettings.find(Name)->second);
    return true;
  }
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

