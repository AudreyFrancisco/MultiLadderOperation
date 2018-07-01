#ifndef STOREPIXHIT_H
#define STOREPIXHIT_H

/**
 * \class TStorePixHit
 *
 * \brief Container with ROOT TTree of hit pixels for all chips read by a given board
 *
 * \author Andry Rakotozafindrabe
 *
 * For each hit pixel, the following information is stored:
 * chip Id, row, column, bunch crossing counter (from chip), trigger 
 * counter (from the readout board). 
 * 
 * Only valid hits (i.e. with no bad flag) are stored. See TPixHit.h for 
 * the list of bad flags.
 * 
 */

#include "TVerbosity.h"
#include <memory>
#include <stdint.h>
#include <string>
#include "Common.h"

class TPixHit;
class TTree;
class TFile;

class TStorePixHit : public TVerbosity {

public:

   /// struct to easily feed the TTree
    typedef struct {
        unsigned int boardIndex;
        unsigned int dataReceiver;
        unsigned int deviceType;
        unsigned int deviceId;
        unsigned int chipId;
        unsigned int row;
        unsigned int col;
        unsigned int bunchNum;
        uint32_t trgNum;
        uint64_t trgTime;
    } TDataSummary;

private:

    /// the ROOT TTree container
    TTree* fTree;

    /// the output file
    TFile* fFile;

    /// the data to be stored in the TTree for each hit pixel
    TDataSummary fData;

    /// name of the output file that will store the TTree
    std::string fOutFileName;

    /// name of the TTree
    std::string fTreeName;

    /// title of the TTree
    std::string fTreeTitle;

public:

    /// default constructor
    TStorePixHit();

    /// default destructor
    virtual ~TStorePixHit();

    /// Set the name of the output ROOT TTree file, the TTree name and title
    void SetNames( const char *baseFName, const common::TChipIndex aChipIndex );

    /// initialization 
    void Init();

    /// fill the ROOT TTree with the information from a hit pixel
    void Fill( std::shared_ptr<TPixHit> hit );

    /// use this method when the job is over
    void Terminate();

private:

    /// set the fields in the data based on the information from a pixel hit
    void SetDataSummary( std::shared_ptr<TPixHit> hit );

    /// set the output ROOT filename by adding  board + device id to a prefix
    void SetFileName( std::string prefix, const common::TChipIndex aChipIndex );
    
    /// construct the name and title of the TTree with board + device id
    void SetTreeNameAndTitle( const common::TChipIndex aChipIndex );

};

#endif