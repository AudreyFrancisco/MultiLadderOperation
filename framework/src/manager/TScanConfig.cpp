#include "TScanConfig.h"
#include <iostream>

using namespace std;

const int TScanConfig::NINJ           = 50; // number of injections in digital/threshold scans
const int TScanConfig::CHARGE_START   = 0;
const int TScanConfig::CHARGE_STOP    = 50;
const int TScanConfig::CHARGE_STEP    = 1;
const int TScanConfig::N_MASK_STAGES  = 3;
const int TScanConfig::PIX_PER_REGION = 32;
const int TScanConfig::N_TRIGGERS = 1000000;
const int TScanConfig::N_TRIGGERS_PER_TRAIN = 100;

//___________________________________________________________________
TScanConfig::TScanConfig()
{
    // default values
    fNInj              = NINJ;
    fChargeStart       = CHARGE_START;
    fChargeStop        = CHARGE_STOP;
    fChargeStep        = CHARGE_STEP;
    fNMaskStages       = N_MASK_STAGES;
    fPixPerRegion      = PIX_PER_REGION;
    fNTriggers         = N_TRIGGERS;
    fNTriggersPerTrain = N_TRIGGERS_PER_TRAIN;
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
    fSettings["NTRIGGERS"]    = &fNTriggers;
    fSettings["NTRGPERTRAIN"] = &fNTriggersPerTrain;
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
int TScanConfig::GetParamValue(const char *Name) const
{
    
    if (fSettings.find (Name) != fSettings.end()) {
        return *(fSettings.find(Name)->second);
    }
    cerr << "TScanConfig::GetParamValue() - Unknown parameter `-" << Name << "`" << endl;
    return -1;
}


