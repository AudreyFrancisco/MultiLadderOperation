#ifndef TSCANCONFIG_H
#define TSCANCONFIG_H

#include <map>
#include <string>

class TScanConfig {

private:
    std::map <std::string, int*> fSettings;
    int fNInj;
    int fChargeStart;
    int fChargeStop;
    int fChargeStep;
    int fNMaskStages;
    int fPixPerRegion;
public:
    TScanConfig ();
    ~TScanConfig() {};
    void InitParamMap  ();
    bool SetParamValue (const char *Name, const char *Value);
    int  GetParamValue (const char *Name) ;
    bool IsParameter   (const char *Name) {return (fSettings.count(Name) > 0);}
    int GetChargeStart () {return fChargeStart;}
    int GetChargeStep  () {return fChargeStep;}
    int GetChargeStop  () {return fChargeStop;}
    int GetNMaskStages () {return fNMaskStages;}
private:
    #pragma mark - default value for the config
    static const int NINJ;
    static const int CHARGE_START;
    static const int CHARGE_STOP;
    static const int CHARGE_STEP;
    static const int N_MASK_STAGES;
    static const int PIX_PER_REGION;
};



#endif
