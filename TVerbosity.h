#ifndef VERBOSITY_H
#define VERBOSITY_H

class TVerbosity {

protected:
    int fVerboseLevel;
    
public:
    #pragma mark - Constructors/destructor
    TVerbosity();
    virtual ~TVerbosity();

    #pragma mark - setters
    virtual void SetVerboseLevel( const int level );
    
    #pragma mark - getters
    int GetVerboseLevel() const { return fVerboseLevel; }

};

#endif
