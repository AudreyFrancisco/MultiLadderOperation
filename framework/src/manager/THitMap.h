#ifndef THITMAP_H
#define THITMAP_H

/**
 * \class THitMap
 *
 * \brief Abstract base class to show hits on the pixel matrix
 *
 * \author Andry Rakotozafindrabe
 *
*/

#include "TVerbosity.h"
#include "Common.h"

#include <string>

class TCanvas;
class TH2F;

class THitMap : public TVerbosity {
    
protected:
    
    /// title for the Y axis
    std::string fYaxisTitle;

    /// title for the X axis
    std::string fXaxisTitle;
    
    /// header for the legend
    std::string fHicChipName;
    
    /// number of injections for each pixel
    unsigned int fNInjections;
    
    /// the canvas that will contain all pads
    TCanvas* fMapCanvas;
    
    /// index of the chip for which we collect errors
    common::TChipIndex fIdx;
    
    /// dummy 2D histo used to draw axis of the hit map
    TH2F* fH2Dummy;
    
    /// min for the x-axis of the hit map
    static const float fXMinDummy;
    
    /// max for the x-axis of the hit map
    static const float fXMaxDummy;
    
    /// number of bin in x for the hit map
    static const int fXNbinDummy;
    
    /// min for the y-axis of the hit map
    static const float fYMinDummy;
    
    /// max for the y-axis of the hit map
    static const float fYMaxDummy;
    
    /// number of bin in y for the hit map
    static const int fYNbinDummy;
    
    /// is y-axis inverted for the hit map (default: true i.e. YMin top, YMax bottom )
    bool fYInvertedDummy;
    
    /// boolean used to check if the canvas for the hit map is ready (default: false)
    bool fMapCanvasReady;
    
    /// boolean use to check if everything is ready to be saved to a file (default: false)
    bool fSaveToFileReady;
    
public:
    
    /// default constuctor
    THitMap();
    
    /// constructor that sets the chip index and the number of injections / pixel
    THitMap( const TDeviceType dt,
             const common::TChipIndex aChipIndex, 
             const unsigned int nInjections = 50 );
    
    /// destructor
    virtual ~THitMap();
    
    /// set the number of injections
    void SetNInjections( const unsigned int value );
    
    /// produce a canvas appropriate to draw any object that must be drawn by the class
    virtual void BuildCanvas() = 0;
    
    /// draw all objects that must be drawn by the class
    virtual void Draw() = 0;
    
    /// save the drawing(s) to PDF file(s)
    virtual void SaveToFile( const char *baseFName ) = 0;

protected:
    
    /// set the hic and chip name for which the hit map will be
    void SetHicChipName();
    
    /// set the base style for any future plot
    void SetBaseStyle();
    
    /// return the title of the histogram that includes hic, chip and axis names
    std::string GetHistoTitle( const std::string prefix ) const;

    /// return a string made of the prefix, the hic id and the chip id
    std::string GetName( const std::string prefix ) const;

    /// return the min for the x-axis of the dummy 2D histo
    static float GetMapXMin() { return fXMinDummy; }

    /// return the max for the x-axis of the dummy 2D histo
    static float GetMapXMax() { return fXMaxDummy; }
    
    /// return the number of bin in x for the dummy 2D histo
    static int GetMapXNbin() { return fXNbinDummy; }
    
    /// return the min for the y-axis of the dummy 2D histo
    static float GetMapYMin() { return fYMinDummy; }
    
    /// return the max for the y-axis of the dummy 2D histo
    static float GetMapYMax() { return fYMaxDummy; }
    
    /// return the number of bin in y for the dummy 2D histo
    static int GetMapYNbin() { return fYNbinDummy; }

    /// return the status of the y axis of the dummy 2D histo
    inline bool IsMapYInverted() const { return fYInvertedDummy; }
    
    /// return the readiness status of the canvas to be used to draw the hit map
    inline bool IsCanvasReady() const { return fMapCanvasReady; }
    
    /// return the readiness status of the drawings for file saving
    inline bool IsSaveToFileReady() const { return fSaveToFileReady; }

};

#endif
