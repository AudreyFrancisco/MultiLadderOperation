#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include <cstdint>
#include <string>
#include <map>

enum class TTriggerSource { kTRIG_INT, kTRIG_EXT };
enum class TBoardType { kBOARD_DAQ, kBOARD_MOSAIC, kBOARD_UNKNOWN };

class TBoardConfig {
 
protected:
    std::map <std::string, int*> fSettings;
    bool           fTriggerEnable;
    bool           fPulseEnable;
    int            fNTriggers;
    int            fTriggerDelay;
    int            fPulseDelay;
    TTriggerSource fTriggerSource;
    TBoardType     fBoardType;
    static const int fPULSEDELAY  = 1000;
    static const int fSTROBEDELAY = 20;

public:
    TBoardConfig();
    virtual ~TBoardConfig(){}
    
    virtual void InitParamMap();
    bool SetParamValue(const char *Name, const char *Value);
    bool SetParamValue(const char *Name, const int Value);
    int  GetParamValue(const char *Name) ;
    bool IsParameter(const char *Name) {return (fSettings.count(Name) > 0);};
    
    virtual TBoardType GetBoardType() {return fBoardType;};
    
    bool           GetTriggerEnable() {return fTriggerEnable;};
    bool           GetPulseEnable  () {return fPulseEnable;};
    int            GetNTriggers    () {return fNTriggers;};
    int            GetTriggerDelay () {return fTriggerDelay;};
    int            GetPulseDelay   () {return fPulseDelay;};
    TTriggerSource GetTriggerSource() {return fTriggerSource;};
    
    void SetTriggerEnable(bool trigEnable)  {fTriggerEnable = trigEnable;};
    void SetPulseEnable  (bool pulseEnable) {fPulseEnable   = pulseEnable;};
    void SetNTriggers    (int  nTriggers)   {fNTriggers     = nTriggers;};
    void SetTriggerDelay (int  trigDelay)   {fTriggerDelay  = trigDelay;};   // obsolete
    void SetPulseDelay   (int  pulseDelay)  {fPulseDelay    = pulseDelay;};  // obsolete
    void SetTriggerSource(TTriggerSource trigSource)  {fTriggerSource = trigSource;};

};


#endif   /* BOARDCONFIG_H */
