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
    int fNTriggers;
    int fNTriggersPerTrain;
    void InitParamMap();

public:
    TScanConfig ();
    ~TScanConfig() {};
    bool SetParamValue (const char *Name, const char *Value);
    int  GetParamValue (const char *Name) const ;
    bool IsParameter   (const char *Name) {return (fSettings.count(Name) > 0);}
    int GetNInj        ()      const { return fNInj;}
    int GetChargeStart ()      const { return fChargeStart; }
    int GetChargeStep  ()      const { return fChargeStep; }
    int GetChargeStop  ()      const { return fChargeStop; }
    int GetNMaskStages ()      const { return fNMaskStages; }
    int GetPixPerRegion()      const { return fPixPerRegion; }
    int GetNTriggers()         const { return fNTriggers; }
    int GetNTriggersPerTrain() const { return fNTriggersPerTrain; }
private:
    #pragma mark - default value for the config
    static const int NINJ;
    static const int CHARGE_START;
    static const int CHARGE_STOP;
    static const int CHARGE_STEP;
    static const int N_MASK_STAGES;
    static const int PIX_PER_REGION;
    static const int N_TRIGGERS;
    static const int N_TRIGGERS_PER_TRAIN;
};



#endif
