#include "TScanConfig.h"
#include <iostream>

using namespace std;

const int TScanConfig::NINJ           = 50;
const int TScanConfig::CHARGE_START   = 0;
const int TScanConfig::CHARGE_STOP    = 50;
const int TScanConfig::CHARGE_STEP    = 1;
const int TScanConfig::N_MASK_STAGES  = 3;
const int TScanConfig::PIX_PER_REGION = 32;

//___________________________________________________________________
TScanConfig::TScanConfig()
{
    // dummy values for first tests
    fNInj         = NINJ;
    fChargeStart  = CHARGE_START;
    fChargeStop   = CHARGE_STOP;
    fChargeStep   = CHARGE_STEP;
    fNMaskStages  = N_MASK_STAGES;
    fPixPerRegion = PIX_PER_REGION;
    InitParamMap();
}

//___________________________________________________________________
void TScanConfig::InitParamMap()
{
    fSettings["NINJ"]         = &fNInj;
    fSettings["CHARGESTART"]  = &fChargeStart;
    fSettings["CHARGESTOP"]   = &fChargeStop;
    fSettings["CHARGESTEP"]   = &fChargeStep;
    fSettings["NMASKSTAGES"]  = &fNMaskStages;
    fSettings["PIXPERREGION"] = &fPixPerRegion;
}

//___________________________________________________________________
bool TScanConfig::SetParamValue(const char *Name, const char *Value)
{
    if (fSettings.find (Name) != fSettings.end()) {
        sscanf (Value, "%d", fSettings.find(Name)->second);
        return true;
    }
    cerr << "TScanConfig::SetParamValue() - Unknown parameter `-" << Name << "`" << endl;
    return false;
}

//___________________________________________________________________
int TScanConfig::GetParamValue(const char *Name)
{
    
    if (fSettings.find (Name) != fSettings.end()) {
        return *(fSettings.find(Name)->second);
    }
    cerr << "TScanConfig::GetParamValue() - Unknown parameter `-" << Name << "`" << endl;
    return -1;
}


