#ifndef TSCURVE_ANALYSIS_H
#define TSCURVE_ANALYSIS_H

/**
 * \class TSCurveAnalysis
 *
 * \brief Extractor and container of threshold and noise results for each tested chip
 *
 * \author Andry Rakotozafindrabe
 *
 * This class allows to extract and store the threshold and noise from a s-curve issued
 * from charge injection into some (or all) pixels of the matrix. It is based on a code
 * from Markus Keil from ITS team.
 *
 */

#include "TVerbosity.h"
#include "Common.h"

#include <string>

class TCanvas;
class TGraph;
class TGraphErrors;
class TH1F;
class TObjArray;
class TLine;
class TPave;


class TSCurveAnalysis : public TVerbosity {
    
    /// index of the chip for which we collect errors
    common::TChipIndex fIdx;
    
    /// header for the legend
    std::string fHicChipName;
    
    /// number of injections, usually 50, of a fixed known charge in a pixel
    unsigned int fNInj;
    
    /// maximum value, usually 50, of the injected charge in a pixel (in DAC units)
    unsigned int fMaxInjCharge;
    
    ///  if true ( = default) threshold and noise are in units of electrons (if false, DAC units)
    bool fDACtoElectronsConversionIsUsed;

    /// number of electrons per injected charge in DAC units when using the analog pulsing
    static const unsigned int fElectronsPerDAC;

    /// (default = false) used to check if the s-curve for the first pixel was already drawn or not
    bool fIsPixelCurveDrawn;
    
    /// cut on the quality of the fit to the s-curve, usually 5
    float fChisqCut;
    
    /// number of times the fit to the S-curve has a chi2/ndf > chi2 cut
    unsigned int fNChisq;

    /// maximum number of points in the s-curve (at most, 1024 points)
    static const unsigned int fMaxNPoints;
    
    /// array of fMaxNPoints that stores the number of times a given pixel responded to a fixed value of the injected charge (y-axis of the s-curve)
    int* fData;
    
    /// array of fMaxNPoints that stores the consecutive values of the injected charge in DAC units (x-axis of the s-curve)
    int* fX;
    
    /// actual number of points in the s-curve
    int fNPoints;

    /// number of pixels studied in the scan
    int fNPixels;
    
    /// number of pixels for which one could not find a value of the injected charge above which the pixel is always responding
    int fNNostart;
    
    /// value of the fit parameter named "Threshold" extracted from the last s-curve fit
    double fThreshold;
    
    /// value of the fit parameter named "Noise" extracted from the last s-curve fit
    double fNoise;
    
    /// chi2/ndf value obtained for the last s-curve fit
    double fChisq;
    
    /// row of the current pixel
    unsigned int fRow;

    /// column of the current pixel
    unsigned int fColumn;

    /// histo filled with the Chisq from the fit of the response of each analyzed pixel
    TH1F* fHChisq;

    /// histo filled with the threshold from the fit of the response of each analyzed pixel
    TH1F* fHThreshold;

    /// histo filled with the noise from the fit of the response of each analyzed pixel
    TH1F* fHNoise;

    /// canvas for the threshold distribution
    TCanvas* fCnv1;
    
    /// canvas for the noise distribution
    TCanvas* fCnv2;
    
    /// canvas for the response (s-curve) for a single pixel
    TCanvas* fCnv3;
    
    /// canvas for the Chisq distribution
    TCanvas* fCnv4;
    
    /// graph with the response of a single pixel (the first one was chosen as an example).
    TGraph* fgClone;
    
    /// filled yellow rectangle drawn on the graph fgClone to show the value of the noise extracted from the Erf fit (parameter 1 of the fit)
    TPave* fPaveNoise;
    
    /// blue line drawn on the graph fgClone to show the value of the threshold extracted from the Erf fit (parameter 0 of the fit)
    TLine* fLineThreshold;

    /// boolean use to check if everything is ready to be saved to a file (default: false)
    bool fSaveToFileReady;

    
public:
    
#pragma mark - Constructors/destructor

    /// default constructor
    TSCurveAnalysis();
    
    /// constructor that sets the chip index, the number of injections and the maximum charge
    TSCurveAnalysis( const common::TChipIndex aChipIndex,
                     const unsigned int nInjectionsPerCharge,
                     const unsigned int maxInjCharge );
    
    /// destructor
    virtual ~TSCurveAnalysis();
    
#pragma mark - Setters/getters
    
    /// Set the number of injections used for each value of the injected charge
    void SetNInjections( const unsigned int nInj );
    
    /// Set the maximum value of the injected charge (in DAC units)
    void SetMaxInjectedCharge( const unsigned int maxInjCharge );
    
    /// Enable the DAC to electrons conversion of the injected charge in the analog pulsing
    inline void UseDACtoElectronsConversion( const bool value = true )  { fDACtoElectronsConversionIsUsed = value; }
    
    void SetPixelCoordinates( const unsigned int dcol, const unsigned int addr );

    /// Return the number of injections used for each value of the injected charge
    inline unsigned int GetNinjections() const { return fNInj; }
    
    /// Get the maximum value of the injected charge (in DAC units)
    inline unsigned int GetMaxInjectedCharge() const { return fMaxInjCharge; }

#pragma mark - other public functions
    
    ///
    void Init();
    
    /// fill fX and fData arrays with the injected charge and the number of hits for each injection
    void FillPixelData( const unsigned int istep,
                       const unsigned int injectedCharge,
                       const unsigned int nhits );
    
    /// call the fit to the S-curve and fill histograms
    void ProcessPixelData();
    
    /// draw threshold and noise distribution for the tested pixels of the chip
    void DrawDistributions();
    
    /// save the drawing(s) to PDF file(s)
    void SaveToFile( const char *fName );
    
protected:
    
    /// set the hic and chip name for which the hit map will be
    void SetHicChipName();
    
    /// set the base style for any future plot
    void SetBaseStyle();
    
    /// return a string made of the prefix, the hic id and the chip id
    std::string GetName( const std::string prefix ) const;

    /// create canvas
    void PrepareCanvas();

    /// create threshold, noise and chi2 histograms
    void PrepareHistos();
    
    /// Function used to fit the S-curve of each tested pixel to extract threshold and noise
    double Erf( double* xx, double* par);
    
    /// fit the S-curve and extract the value fit parameters (threshold, noise) for a pixel
    bool FitSCurve();

    /// used at the start of the S-curve fitting procedure, as a guess of the threshold value
    float FindStart() const;
    
    /// reset he data in the fData array, and the values of fNPoints, fThreshold, fNoise and fChisq
    void ResetData();
    
    /// return the readiness status of the drawings for file saving
    inline bool IsSaveToFileReady() const { return fSaveToFileReady; }

};
#endif
