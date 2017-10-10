#ifndef THITMAPDISCORDANT_H
#define THITMAPDISCORDANT_H

/**
 * \class THitMapDiscordant
 *
 * \brief Show the location of any bad on the pixel matrix, and their firing frequency
 *
 * \author Andry Rakotozafindrabe
 *
 * Currently the considered types of bad pixel are:
 * - the dead pixels
 * - the inefficient pixels
 * - the hot pixels.
 *
 * \remark
 * Stuck pixels should (will ?) be added as well.
 *
*/

#include "THitMap.h"
#include "Common.h"

#include <string>
#include <memory>

class TPixHit;

// ROOT
class TCanvas;
class TH2F;
class TLegend;
class TH1F;
class TPad;
class TPaveText;

class THitMapDiscordant : public THitMap {
    
    /// number of dead pixels
    unsigned int fNDeadPixels;
    
    /// number of inefficient pixels
    unsigned int fNInefficientPixels;
    
    /// number of hot pixels
    unsigned int fNHotPixels;
    
    /// width of the canvas for the hit map
    static const int fWidth;
    
    /// height of the canvas for the hit map
    static const int fHeight;
    
    /// fraction of the canvas that will be occupied by the main pad for the hit map
    static const float fRelativeSize;

    /// main pad where the hit map will be drawn
    TPad* fMapPadMain;
    
    /// pad with legend for the hit map
    TPad* fMapPadLegend;
    
    /// the legend for the hit map
    TPaveText* fMapLegend;
    
    /// the canvas dedicated to the 1D histo displaying the firing frequency of bad pixels
    TCanvas* fFireCanvas;

    /// 1D histo filled with bad pixel firing, used to get the vertical scale
    TH1F* fHistoScale;

    /// 1D histo filled the number of times a dead pixel is found
    TH1F* fHistoDead;

    /// 1D histo filled the number of times an inefficient pixel is firing
    TH1F* fHistoInefficient;

    /// 1D histo filled the number of times a hot pixel is firing
    TH1F* fHistoHot;
    
    /// legend for the 1D histo displaying firing frequency distribution of bad pixels
    TLegend* fHistoLegend;

    /// symbol to be used for dead pixel markers (open circle)
    static const int fDeadStyle;
    
    /// symbol to be used for inefficient pixel markers (open triangle down)
    static const int fIneffStyle;
    
    /// symbol to be used for hot pixel markers (open triangle up)
    static const int fHotStyle;

    /// color to be used for dead pixel markers (kBlack)
    static const int fDeadColor;
    
    /// color to be used for inefficient pixel markers (kAzure-3)
    static const int fIneffColor;
    
    /// color to be used for hot pixel markers (kRed)
    static const int fHotColor;
    
    /// size to be used for dead pixel markers (0.4)
    static const float fDeadSize;
    
    /// size to be used for inefficient pixel markers (0.6)
    static const float fIneffSize;
    
    /// size to be used for hot pixel markers (0.6)
    static const float fHotSize;


public:
    
    /// default constuctor
    THitMapDiscordant();
    
    /// constructor that sets the chip index
    THitMapDiscordant( const common::TChipIndex aChipIndex,
                       const unsigned int nInjections  );
    
    /// destructor
    virtual ~THitMapDiscordant();
    
    /// produce a canvas appropriate to draw the hit map
    void BuildCanvas();
    
    /// add a dead pixel to the hit map
    void AddDeadPixel( std::shared_ptr<TPixHit> pix );

    /// add an inefficient pixel to the list
    void AddInefficientPixel( std::shared_ptr<TPixHit> pix, const unsigned int nTimesFired );
    
    /// add a hot pixel to the list
    void AddHotPixel( std::shared_ptr<TPixHit> pix, const unsigned int nTimesFired );

    /// draw all objects that must be drawn by the class
    void Draw();
    
    /// save the drawing(s) to PDF file(s)
    void SaveToFile( const char *fName );
    
private:
    
    /// last step for the hit map, i.e. add legends for each type of bad pixel
    void FinishHitMap();
    
    /// draw the 1D histo with the distribution of the firing frequency for bad pixels
    void DrawFiringFrequencyHisto();

};

#endif
