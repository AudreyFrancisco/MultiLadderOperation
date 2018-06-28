#ifndef THITMAPVIEWALL_H
#define THITMAPVIEWALL_H

/**
 * \class THitMapViewAll
 *
 * \brief Class used to show all hits received on the pixel matrix of a chip
 *
 * \author Andry Rakotozafindrabe
 *
*/

#include "TVerbosity.h"
#include "Common.h"
#include "THitMap.h"
#include <memory>
#include <map>


class TScanHisto;
class TH2F;

class THitMapView : public THitMap {

    /// container of the list of hit pixels per chip for all chips
    std::shared_ptr<TScanHisto> fScanHisto;

    /// 2D histo to be filled with the hit map of the current chip
    TH2F* fHitMap;

    /// a boolean that will be true if there is any pixel hit on that chip
    bool fHasData;

    /// the current chip scrutinzed by a given instance of this class
    common::TChipIndex fChipIndex;


public: 

    /// default constructor
    THitMapView();

    /// constructor that sets the chip index and the container of the list of hit pixels
    THitMapView( std::shared_ptr<TScanHisto> aScanHisto, 
                    const common::TChipIndex aChipIndex );

    /// destructor
    virtual ~THitMapView();

    /// produce a canvas appropriate to draw the hit map
    void BuildCanvas();

    /// draw all objects that must be drawn by the class
    void Draw();

    /// transfer the list of pixel hits to the TH2F the chip
    void FillHitMap();  

    /// return the current chip index
    common::TChipIndex GetChipIndex() const { return fChipIndex; }

    /// return the TH2F* hit map for the chip
    TH2F* GetHitMap() { return fHitMap; }

    /// return true if the chip has some hit
    bool HasData() const { return fHasData; }

    /// write the list of hit pixels to a file
    void WriteHitsToFile( const char *fName, const bool Recreate );

    /// save the drawing(s) to PDF file(s) and save the TH2F to a root file
    void SaveToFile( const char *fName );

};

#endif
