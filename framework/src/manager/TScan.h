#ifndef TSCAN_H
#define TSCAN_H

#include <deque>
#include <memory>

class TDevice;
class TScanConfig;
class TScanHisto;
class THisto;

class TScan {
    
private:
    static const int MAXLOOPLEVEL = 3;
    static const int MAXBOARDS    = 2;
    
protected:
    std::weak_ptr<TScanConfig> fScanConfig;
    std::weak_ptr<TDevice> fDevice;
    std::unique_ptr<TScanHisto> fHisto;
    std::deque<TScanHisto> *fHistoQue;
    int fStart[MAXLOOPLEVEL];
    int fStop[MAXLOOPLEVEL];
    int fStep[MAXLOOPLEVEL];
    int fValue[MAXLOOPLEVEL];
    
    virtual std::shared_ptr<THisto> CreateHisto() = 0;
    
public:
    TScan();
    TScan( std::shared_ptr<TScanConfig> aConfig,
          std::shared_ptr<TDevice> aDevice,
          std::deque<TScanHisto> *histoQue );
    virtual ~TScan() {};
    
    virtual void Init            ()              = 0;
    virtual void Terminate       ()              = 0;
    virtual void LoopStart       ( const int loopIndex ) = 0;
    virtual void LoopEnd         ( const int loopIndex ) = 0;
    virtual void PrepareStep     ( const int loopIndex ) = 0;
    virtual void Execute         ()              = 0;
    bool         Loop            ( const int loopIndex );
    void         Next            ( const int loopIndex );
    void         CreateScanHisto ();
};



class TMaskScan : public TScan {
    
protected:
    int  fPixPerStage;
    int  fRow;
    void ConfigureMaskStage( const int ichip, const int istage );

public:
    TMaskScan();
    TMaskScan( std::shared_ptr<TScanConfig> aConfig,
              std::shared_ptr<TDevice> aDevice,
              std::deque<TScanHisto> *histoQue );
    virtual ~TMaskScan() {};
};

#endif
