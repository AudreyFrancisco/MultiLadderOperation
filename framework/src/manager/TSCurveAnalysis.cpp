#include "TSCurveAnalysis.h"
#include "TPixHit.h"
#include <iomanip>
#include <iostream>

// ROOT includes
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TF1.h"
#include "TObjArray.h"
#include "TLine.h"
#include "TPave.h"
#include "Rtypes.h"
#include "TStyle.h"
#include "TROOT.h" // useful for global ROOT pointers (such as gPad)
#include "RtypesCore.h"
#include "TMath.h"

using namespace std;

const unsigned int TSCurveAnalysis::fElectronsPerDAC = 7;
const unsigned int TSCurveAnalysis::fMaxNPoints = 512;

//___________________________________________________________________
TSCurveAnalysis::TSCurveAnalysis() :
TVerbosity(),
fHicChipName( "" ),
fNInj( 50 ),
fMaxInjCharge( 50 ),
fDACtoElectronsConversionIsUsed( true ),
fIsPixelCurveDrawn( false ),
fChisqCut( 5 ),
fNChisq( 0 ),
fData( new int[ TSCurveAnalysis::fMaxNPoints ] ),
fX( new int[ TSCurveAnalysis::fMaxNPoints ] ),
fNPoints( 0 ),
fNPixels( 0 ),
fNNostart( 0 ),
fThreshold( 0 ),
fNoise( 0 ),
fChisq( 0 ),
fRow( 0 ),
fColumn( 0 ),
fHChisq( nullptr ),
fHThreshold( nullptr ),
fHNoise( nullptr ),
fCnv1( nullptr ),
fCnv2( nullptr ),
fCnv3( nullptr ),
fCnv4( nullptr ),
fgClone( nullptr ),
fPaveNoise( nullptr ),
fLineThreshold( nullptr ),
fSaveToFileReady( false )
{
    fIdx.boardIndex = 0;
    fIdx.dataReceiver = 0;
    fIdx.ladderId = 0;
    fIdx.chipId = 0;
    
    SetBaseStyle();
}

//___________________________________________________________________
TSCurveAnalysis::TSCurveAnalysis( const common::TChipIndex aChipIndex,
                                 const unsigned int nInjectionsPerCharge,
                                 const unsigned int maxInjCharge ) :
TVerbosity(),
fHicChipName( "" ),
fNInj( 50 ),
fMaxInjCharge( 50 ),
fDACtoElectronsConversionIsUsed( true ),
fIsPixelCurveDrawn( false ),
fChisqCut( 5 ),
fNChisq( 0 ),
fData( new int[ TSCurveAnalysis::fMaxNPoints ] ),
fX( new int[ TSCurveAnalysis::fMaxNPoints ] ),
fNPoints( 0 ),
fNPixels( 0 ),
fNNostart( 0 ),
fThreshold( 0 ),
fNoise( 0 ),
fChisq( 0 ),
fRow( 0 ),
fColumn( 0 ),
fHChisq( nullptr ),
fHThreshold( nullptr ),
fHNoise( nullptr ),
fCnv1( nullptr ),
fCnv2( nullptr ),
fCnv3( nullptr ),
fCnv4( nullptr ),
fgClone( nullptr ),
fPaveNoise( nullptr ),
fLineThreshold( nullptr ),
fSaveToFileReady( false )
{
    fIdx.boardIndex = aChipIndex.boardIndex;
    fIdx.dataReceiver = aChipIndex.dataReceiver;
    fIdx.ladderId = aChipIndex.ladderId;
    fIdx.chipId = aChipIndex.chipId;
    
    SetNInjections( nInjectionsPerCharge );
    SetMaxInjectedCharge( maxInjCharge );
    
    SetBaseStyle();
}

//___________________________________________________________________
TSCurveAnalysis::~TSCurveAnalysis()
{
    if ( fData ) {
        delete[] fData;
    }
    if ( fX ) {
        delete[] fX;
    }

    // don't delete any other pointer to ROOT object
    // ROOT will take care by itself and delete anything in the Canvas
    if ( fCnv1 ) {
        fCnv1->Clear();
        delete fCnv1;
    }
    if ( fCnv2 ) {
        fCnv2->Clear();
        delete fCnv2;
    }
    if ( fCnv3 ) {
        fCnv3->Clear();
        delete fCnv3;
    }
    if ( fCnv4 ) {
        fCnv4->Clear();
        delete fCnv4;
    }
}

//___________________________________________________________________
void TSCurveAnalysis::SetNInjections( const unsigned int nInj )
{
    if ( nInj == 0 ) {
        cerr << "TSCurveAnalysis::SetNInjections() - zero injection is not valid !" << endl;
        return;
    }
    fNInj = nInj;
}

//___________________________________________________________________
void TSCurveAnalysis::SetMaxInjectedCharge( const unsigned int maxInjCharge )
{
    if ( maxInjCharge == 0 ) {
        cerr << "TSCurveAnalysis::SetMaxInjectedCharge() - zero value is not valid !" << endl;
        return;
    }
    fMaxInjCharge = maxInjCharge;
}

//___________________________________________________________________
void TSCurveAnalysis::SetPixelCoordinates( const unsigned int dcol,
                                           const unsigned int addr )
{
    TPixHit pix;
    pix.SetPixFlag( TPixFlag::kOK );
    pix.SetDoubleColumn( dcol );
    pix.SetAddress( addr );
    if ( !pix.IsPixHitCorrupted() ) {
        fRow = pix.GetRow();
        fColumn = pix.GetColumn();
    } else {
        cerr << "TSCurveAnalysis::SetPixelCoordinates() - bad pixel coordinates !" << endl;
    }
}


//___________________________________________________________________
void TSCurveAnalysis::Init()
{
    SetHicChipName();
    PrepareCanvas();
    PrepareHistos();
}

//___________________________________________________________________
void TSCurveAnalysis::FillPixelData( const unsigned int istep,
                                    const unsigned int injectedCharge,
                                    const unsigned int nhits )
{
    if ( istep < fMaxNPoints ) {
        if ( fDACtoElectronsConversionIsUsed ) {
            fX[istep] = injectedCharge * fElectronsPerDAC;
        } else {
            fX[istep] = injectedCharge;
        }
        fData[istep] = nhits;
        fNPoints++;
    }
}

//___________________________________________________________________
void TSCurveAnalysis::ProcessPixelData()
{
    if ( !fData ) {
        throw runtime_error( "TSCurveAnalysis::ProcessPixelData() - undefined fData array!" );
    }
    if ( !fX ) {
        throw runtime_error( "TSCurveAnalysis::ProcessPixelData() - undefined fX array!" );
    }
    if ( !fHChisq ) {
        throw runtime_error( "TSCurveAnalysis::ProcessPixelData() - undefined fHChisq histo!" );
    }
    if ( !fHThreshold ) {
        throw runtime_error( "TSCurveAnalysis::ProcessPixelData() - undefined fHThreshold histo!" );
    }
    if ( !fHNoise ) {
        throw runtime_error( "TSCurveAnalysis::ProcessPixelData() - undefined fHNoise histo!" );
    }
    bool success = FitSCurve();
    if ( success ) {
        fHChisq->Fill( fChisq );
        if ( fChisq < fChisqCut ) {
            fHThreshold->Fill( fThreshold );
            fHNoise->Fill( fNoise );
        } else {
            fNChisq++;
        }
    } else {
        if ( GetVerboseLevel() > kTERSE ) {
            cerr << "TSCurveAnalysis::ProcessPixelData() - fit failed, (chip "
                 << std::dec << fIdx.chipId << ") row " << fRow << " : column " << fColumn << endl;
        }
    }
    ResetData();
}

//___________________________________________________________________
void TSCurveAnalysis::DrawDistributions()
{
    if ( !fHChisq ) {
        throw runtime_error( "TSCurveAnalysis::DrawDistributions() - undefined fHChisq histo!" );
    }
    if ( !fHThreshold ) {
        throw runtime_error( "TSCurveAnalysis::DrawDistributions() - undefined fHThreshold histo!" );
    }
    if ( !fHNoise ) {
        throw runtime_error( "TSCurveAnalysis::DrawDistributions() - undefined fHNoise histo!" );
    }
    if ( !fCnv1 ) {
        throw runtime_error( "TSCurveAnalysis::DrawDistributions() - undefined fCnv1 canvas!" );
    }
    if ( !fCnv2 ) {
        throw runtime_error( "TSCurveAnalysis::DrawDistributions() - undefined fCnv2 canvas!" );
    }
    if ( !fCnv4 ) {
        throw runtime_error( "TSCurveAnalysis::DrawDistributions() - undefined fCnv4 canvas!" );
    }

    cout << std::dec << endl;
    cout << "------------------------------- TSCurveAnalysis::DrawDistributions() " << endl;
    if ( fIdx.ladderId ) {
        cout << "Board . receiver . ladder / chip: "
        << std::dec << fIdx.boardIndex << " . "
        << fIdx.dataReceiver << " . " << fIdx.ladderId << " / " << fIdx.chipId << endl;
    } else {
        cout << "Board . receiver / chip: "
        << std::dec << fIdx.boardIndex << " . "
        << fIdx.dataReceiver << " / " << fIdx.chipId << endl;
    }
    cout << "Found                 " << fNPixels << "pixels " << endl;
    cout << "No start point found: " << fNNostart << endl;
    cout << "Chisq cut failed:     " << fNChisq << endl;
    cout << "Chisq cut value:      " << fChisqCut << endl;
    printf("Threshold : %6.3f +/- %6.3f\n",
           fHThreshold->GetMean(), fHThreshold->GetRMS() );
    printf("    Noise : %6.3f +/- %6.3f\n",
           fHNoise->GetMean(), fHNoise->GetRMS() );
    cout << "-------------------------------" << endl << endl;

    fCnv1->cd();
    fHThreshold->SetMarkerColor( kAzure-3 );
    fHThreshold->SetLineColor( kAzure-3 );
    fHThreshold->SetFillColor( kAzure-3 );
    fHThreshold->GetXaxis()->SetRangeUser( 0, 400 );
    fHThreshold->GetYaxis()->SetRangeUser( 0, 1.1*fHThreshold->GetMaximum() );
    fHThreshold->Draw("hist");
    
    fCnv2->cd();
    fHNoise->SetMarkerColor( kAzure-3 );
    fHNoise->SetLineColor( kAzure-3 );
    fHNoise->SetFillColor( kAzure-3 );
    fHNoise->GetXaxis()->SetRangeUser( 0, 30 );
    fHNoise->GetYaxis()->SetRangeUser( 0, 1.1*fHNoise->GetMaximum() );
    fHNoise->Draw("hist");

    fCnv4->cd();
    fHChisq->SetMarkerColor( kAzure-3 );
    fHChisq->SetLineColor( kAzure-3 );
    fHChisq->SetFillColor( kAzure-3 );
    fCnv4->Draw("hist");
    
    fSaveToFileReady = true;
}

//___________________________________________________________________
void TSCurveAnalysis::SaveToFile( const char *fName )
{
    if ( !IsSaveToFileReady() ) {
        throw runtime_error( "TSCurveAnalysis::SaveToFile() - not ready! Please use DrawDistributions() first." );
    }
    cout << "------------------------------- TSCurveAnalysis::SaveToFile() " << endl;
    
    char  fNameChip[100];
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    string suffix( fNameTemp );
    
    string filename = common::GetFileName( fIdx, suffix, "", ".pdf" );
    strcpy( fNameChip, filename.c_str() );
    if ( GetVerboseLevel() > kSILENT ) {
        cout << "TChipErrorCounter::DrawAndSaveToFile() - Chip ID = " << fIdx.chipId ;
        if ( fIdx.ladderId ) {
            cout << " , Ladder ID = " << fIdx.ladderId;
        }
        cout << " to file " << fNameChip << endl;
    }
    
    char fNameOpen[101], fNameClose[101];
    sprintf( fNameOpen,"%s(", fNameChip);
    sprintf( fNameClose,"%s)", fNameChip);
    
    fCnv1->Print( fNameOpen, "Title:Threshold distribution");
    fCnv2->Print( fNameOpen, "Title:Noise distribution");
    fCnv3->Print( fNameOpen, "Title:Response for a single pixel");
    fCnv4->Print( fNameClose, "Title:Chi2/ndf distribution");
    
    cout << "------------------------------- " << endl;
}

//___________________________________________________________________
void TSCurveAnalysis::SetHicChipName()
{
    if ( fIdx.ladderId ) {
        fHicChipName = "Hic ";
        fHicChipName += std::to_string( fIdx.ladderId );
        fHicChipName += " ";
    }
    fHicChipName += "Chip ";
    fHicChipName += std::to_string( fIdx.chipId );
}

//___________________________________________________________________
void TSCurveAnalysis::SetBaseStyle()
{
    
    gStyle->SetCanvasColor(0);
    gStyle->SetPadColor(0);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetFrameBorderMode(0);
    
    gStyle->SetPadTickX( true );
    gStyle->SetPadTickY( true );
    gStyle->SetPadGridX( false );
    gStyle->SetPadGridY( false );
    
    gStyle->SetLegendFont(42);
}

//___________________________________________________________________
string TSCurveAnalysis::GetName( const string prefix ) const
{
    string name = prefix;
    if ( fIdx.ladderId ) {
        name += "_hic";
        name += std::to_string( fIdx.ladderId );
    }
    name += "_chip";
    name += std::to_string( fIdx.chipId );
    return name;
}

//___________________________________________________________________
void TSCurveAnalysis::PrepareCanvas()
{
    if ( !fCnv1 ) {
        fCnv1 = new TCanvas( GetName("fCnv1").c_str(), "Threshold distribution");
    }
    if ( !fCnv2 ) {
        fCnv2 = new TCanvas( GetName("fCnv2").c_str(), "Noise distribution");
    }
    if (!fCnv3 ) {
        fCnv3 = new TCanvas ( GetName("fCnv3").c_str(), "Response for a single pixel") ;
    }
    if (!fCnv4 ) {
        fCnv4 = new TCanvas ( GetName("fCnv4").c_str(), "Chi2/ndf distribution") ;
    }
}

//___________________________________________________________________
void TSCurveAnalysis::PrepareHistos()
{
    int nbinT = 100; int minT = 0; int maxT = 600; // threshold
    int nbinN = 60; int minN = 0; int maxN = 60; // noise
    
    if ( !fDACtoElectronsConversionIsUsed ) {
        maxT = GetMaxInjectedCharge(); nbinT = GetMaxInjectedCharge();
        maxN = 5; nbinN = 50;
    }
    // threshold distribution
    if ( !fHThreshold ) {
        fHThreshold = new TH1F( GetName("hThresh_").c_str(), fHicChipName.c_str(), nbinT, minT, maxT );
        if ( !fDACtoElectronsConversionIsUsed ) {
            fHThreshold->SetXTitle("Threshold [DAC units]");
        } else {
            fHThreshold->SetXTitle("Threshold [electrons]");
        }
        fHThreshold->SetYTitle("#(pixels)");
    }
    
    // noise distribution
    if ( !fHNoise ) {
        fHNoise = new TH1F( GetName("hNoise_").c_str(), fHicChipName.c_str(), nbinN, minN,  maxN );
        if ( !fDACtoElectronsConversionIsUsed ) {
            fHNoise->SetXTitle("Noise [DAC units]");
        } else {
            fHNoise->SetXTitle("Noise [electrons]");
        }
        fHNoise->SetYTitle("#(pixels)");
    }

    // chi2 distribution
    if ( !fHChisq ) {
        fHChisq = new TH1F( GetName("hChisq_").c_str(), fHicChipName.c_str(), 1000, 0., 100.);
        fHChisq->SetXTitle("Chi2");
        fHChisq->SetYTitle("#(pixels)");
    }
}

//___________________________________________________________________
double TSCurveAnalysis::Erf( double* xx, double* par)
{
    return ( GetNinjections()/2 ) * TMath::Erf((xx[0] - par[0]) / (sqrt(2) *par[1])) + ( GetNinjections()/2 );
}

//___________________________________________________________________
bool TSCurveAnalysis::FitSCurve()
{
    TGraph* g = new TGraph( fNPoints, fX, fData );
    
    // Drawing graph for the first analyzed pixel...
    if ( !fIsPixelCurveDrawn ) {
        fCnv3->cd();
        if ( !fgClone ) {
            fgClone = (TGraph*) g->Clone( GetName("gClone").c_str() );
            fgClone -> SetMarkerStyle(20);
            fgClone->SetTitle("Response for a single pixel");
            if ( !fDACtoElectronsConversionIsUsed ) {
                fgClone->GetXaxis()->SetTitle("Injected charge [DAC units]");
            } else {
                fgClone->GetXaxis()->SetTitle("Injected charge [electrons]");
            }
            fgClone->GetYaxis()->SetTitle("#Hits");
            fgClone->Draw("ape");
        }
    }
    
    TF1* fitfcn = new TF1( "fitfcn", this, &TSCurveAnalysis::Erf, 0, 1500, 2 );
    fitfcn->SetNpx(10000);
    float Start  = FindStart();
    
    if ( Start < 0 ) {
        fNNostart ++;
        return false;
    }
    
    fitfcn->SetParameter(0,Start);
    fitfcn->SetParameter(1,8);
    fitfcn->SetParName(0, "Threshold");
    fitfcn->SetParName(1, "Noise");
    
    //g->SetMarkerStyle(20);
    //g->Draw("AP");
    g->Fit("fitfcn","Q");
    
    fNoise     = fitfcn->GetParameter(1);
    fThreshold = fitfcn->GetParameter(0);
    fChisq     = fitfcn->GetChisquare()/fitfcn->GetNDF();
    
    // Drawing fit and fit parameters for the first analyzed pixel...
    if ( !fIsPixelCurveDrawn ) {
        fCnv3->cd();
        if ( !fPaveNoise ) {
            fPaveNoise = new TPave(fThreshold-fNoise, fgClone->GetHistogram()->GetMaximum(), fThreshold+fNoise, 0 );
            fPaveNoise->SetFillColor(kYellow);
            fPaveNoise->Draw("same");
        }
        if ( !fLineThreshold ) {
            fLineThreshold = new TLine( fThreshold, 0., fThreshold, fgClone->GetHistogram()->GetMaximum() );
            fLineThreshold->SetLineColor(kBlue);
            fLineThreshold->SetLineWidth(2);
            fLineThreshold->Draw("same");
        }
        fgClone->Draw("psame");
        fIsPixelCurveDrawn = true;
    }
    
    if ( fHChisq ) fHChisq->Fill( fChisq );
    
    g->Delete();
    fitfcn->Delete();
    fNPixels++;
    return true;
}

//_______________________________________________________________
float TSCurveAnalysis::FindStart() const
{
    
    float Upper = -1;
    float Lower = -1;
    
    for (int i = 0; i < fNPoints; i ++) {
        if ( fData[i] == (int)GetNinjections() ) {
            Upper = (float) fX[i];
            break;
        }
    }
    if (Upper == -1) return -1;
    for (int i = fNPoints-1; i > 0; i--) {
        if (fData[i] == 0) {
            Lower = (float) fX[i];
            break;
        }
    }
    if ((Lower == -1) || (Upper < Lower)) return -1;
    return (Upper + Lower)/2;
}

//_______________________________________________________________
void TSCurveAnalysis::ResetData()
{
    fThreshold =  0;
    fNoise = 0;
    fChisq = 0;
    fNPoints = 0;
    for ( unsigned int i = 0; i < TSCurveAnalysis::fMaxNPoints; i++ ) {
        fData[i] = 0;
    }
}



