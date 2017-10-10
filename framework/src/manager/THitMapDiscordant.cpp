#include "THitMapDiscordant.h"
#include "TPixHit.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

// ROOT includes
#include "TStyle.h"
#include "TROOT.h" // useful for global ROOT pointers (such as gPad)
#include "TGaxis.h"
#include "TH2F.h"
#include "TMarker.h"
#include "Rtypes.h"
#include "TPDF.h"
#include "TLine.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TH1F.h"
#include "TPaveText.h"
#include "RtypesCore.h"
#include "Rtypes.h"

using namespace std;

const int THitMapDiscordant::fWidth = 1000;
const int THitMapDiscordant::fHeight = 475;
const float THitMapDiscordant::fRelativeSize = 0.8;

const int THitMapDiscordant::fDeadStyle = 24;
const int THitMapDiscordant::fIneffStyle = 32;
const int THitMapDiscordant::fHotStyle = 26;

const int THitMapDiscordant::fDeadColor = kBlack;
const int THitMapDiscordant::fIneffColor = kAzure-3;
const int THitMapDiscordant::fHotColor = kRed;

const float THitMapDiscordant::fDeadSize = 0.4;
const float THitMapDiscordant::fIneffSize = 0.6;
const float THitMapDiscordant::fHotSize = 0.6;

//___________________________________________________________________
THitMapDiscordant::THitMapDiscordant() :
THitMap(),
fNDeadPixels( 0 ),
fNInefficientPixels( 0 ),
fNHotPixels( 0 ),
fMapPadMain( nullptr ),
fMapPadLegend( nullptr ),
fMapLegend( nullptr ),
fFireCanvas( nullptr ),
fHistoScale( nullptr ),
fHistoDead( nullptr ),
fHistoInefficient( nullptr ),
fHistoHot( nullptr ),
fHistoLegend( nullptr )
{
    fFireCanvas = new TCanvas( "cvF" );
    fHistoScale = new TH1F( "hscale", "Discordant pixels; Firing frequency per pixel; Yield", (int)(1.5*fNInjections), -3.5, (1.5*fNInjections)+3.5 );
    fHistoDead = new TH1F( "hdead", "", (int)(1.5*fNInjections), -3.5, (1.5*fNInjections)+3.5 );
    fHistoInefficient = new TH1F( "hineff", "", (int)(1.5*fNInjections), -3.5, (1.5*fNInjections)+3.5 );
    fHistoHot =  new TH1F( "hhot", "", (int)(1.5*fNInjections), -3.5, (1.5*fNInjections)+3.5 );
    
    fHistoScale->SetStats( kFALSE );
    fHistoDead->SetStats( kFALSE );
    fHistoInefficient->SetStats( kFALSE );
    fHistoHot->SetStats( kFALSE );
    
    fHistoScale->SetBit( kCanDelete );
    fHistoDead->SetBit( kCanDelete );
    fHistoInefficient->SetBit( kCanDelete );
    fHistoHot->SetBit( kCanDelete );
}

//___________________________________________________________________
THitMapDiscordant::THitMapDiscordant( const common::TChipIndex aChipIndex,
                                      const unsigned int nInjections ) :
THitMap( aChipIndex, nInjections ),
fNDeadPixels( 0 ),
fNInefficientPixels( 0 ),
fNHotPixels( 0 ),
fMapPadMain( nullptr ),
fMapPadLegend( nullptr ),
fMapLegend( nullptr ),
fFireCanvas( nullptr ),
fHistoScale( nullptr ),
fHistoDead( nullptr ),
fHistoInefficient( nullptr ),
fHistoHot( nullptr ),
fHistoLegend( nullptr )
{
    fFireCanvas = new TCanvas( "cvF" );
    string titleS = fHicChipName + "; Firing frequency per pixel; Yield";
    fHistoScale = new TH1F( "hscale", titleS.c_str(), (int)(1.5*fNInjections), -3.5, (1.5*fNInjections)+3.5 );
    fHistoDead = new TH1F( "hdead", "", (int)(1.5*fNInjections), -3.5, (1.5*fNInjections)+3.5 );
    fHistoInefficient = new TH1F( "hineff", "", (int)(1.5*fNInjections), -3.5, (1.5*fNInjections)+3.5 );
    fHistoHot =  new TH1F( "hhot", "", (int)(1.5*fNInjections), -3.5, (1.5*fNInjections)+3.5 );
    
    string nameS = GetName( "hscale" );
    fHistoScale->SetName( nameS.c_str() );

    string nameD = GetName( "hdead" );
    fHistoDead->SetName( nameD.c_str() );

    string nameI = GetName( "hineff" );
    fHistoInefficient->SetName( nameI.c_str() );

    string nameH = GetName( "hhot" );
    fHistoHot->SetName( nameH.c_str() );
    
    string nameCv = GetName( "fFireCanvas" );
    fFireCanvas->SetName( nameCv.c_str() );

    fHistoScale->SetStats( kFALSE );
    fHistoDead->SetStats( kFALSE );
    fHistoInefficient->SetStats( kFALSE );
    fHistoHot->SetStats( kFALSE );
    
    fHistoScale->SetBit( kCanDelete );
    fHistoDead->SetBit( kCanDelete );
    fHistoInefficient->SetBit( kCanDelete );
    fHistoHot->SetBit( kCanDelete );
    
    fHistoDead->SetFillColor( fDeadColor );
    fHistoInefficient->SetFillColor( fIneffColor );
    fHistoHot->SetFillColor( fHotColor );

    fHistoDead->SetLineColor( fDeadColor );
    fHistoInefficient->SetLineColor( fIneffColor );
    fHistoHot->SetLineColor( fHotColor );

}

//___________________________________________________________________
THitMapDiscordant::~THitMapDiscordant()
{
    // don't delete any other pointer to ROOT object
    // ROOT will take care by itself and delete anything in the Canvas
    fFireCanvas->Clear();
    delete fFireCanvas;
}

//___________________________________________________________________
void THitMapDiscordant::BuildCanvas()
{
    if ( IsCanvasReady() ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "THitMapDiscordant::BuildCanvas() - canvas already built, nothing to be done." << endl;
        }
        return;
    }
    
    //--- hit map
    
    // canvas
    
    fMapCanvas->SetWindowSize( fWidth, fHeight );
    fMapCanvas->SetFillColor( kWhite );
    fMapCanvas->SetFillStyle( kFSolid );
    fMapCanvas->SetTopMargin( 0.08 );
    fMapCanvas->SetBottomMargin( 0.02 );
    fMapCanvas->SetLeftMargin( 0.02 );
    fMapCanvas->SetRightMargin( 0.04 );
    fMapCanvas->cd();
    
    // main pad
    
    fMapPadMain = new TPad( "fMapPadMain", "", 0, 0, fRelativeSize, 1 );
    string mainpadname = GetName( "fMapPadMain" );
    fMapPadMain->SetName( mainpadname.c_str() );
    fMapPadMain->SetBorderSize( 0 );
    fMapPadMain->SetFillColor( kWhite );
    fMapPadMain->SetTopMargin( 0.13 );
    fMapPadMain->SetBottomMargin( 0.13 );
    fMapPadMain->SetLeftMargin( 0.1 );
    fMapPadMain->SetRightMargin( 0.05 );
    fMapPadMain->Draw();
    fMapPadMain->cd();
    
    // dummy histo (into the main pad)
    
    fH2Dummy->SetStats( kFALSE );
    (fH2Dummy->GetXaxis())->SetNdivisions( 506 );
    (fH2Dummy->GetXaxis())->SetLabelFont( 42 );
    (fH2Dummy->GetXaxis())->SetLabelSize( 0.04 );
    (fH2Dummy->GetXaxis())->SetTitleSize( 0.04 );
    (fH2Dummy->GetXaxis())->SetTitleOffset( 1.3 );
    (fH2Dummy->GetYaxis())->SetTitleSize( 0.04 );
    (fH2Dummy->GetYaxis())->SetTitleOffset( 1.15 );
    fH2Dummy->Draw( "AXIS" );
    
    if ( IsMapYInverted() ) {
        (fH2Dummy->GetYaxis())->SetLabelOffset(999);
        (fH2Dummy->GetYaxis())->SetTickLength(0);
        gPad->Update();
        TGaxis* newaxis = new TGaxis( fMapPadMain->GetUxmin(),
                        fMapPadMain->GetUymax(),
                        fMapPadMain->GetUxmin()-0.001,
                        fMapPadMain->GetUymin(),
                        (fH2Dummy->GetYaxis())->GetXmin(),
                        (fH2Dummy->GetYaxis())->GetXmax(),
                        506, "+" );
        newaxis->SetLabelOffset(-0.03);
        newaxis->SetLabelFont( 42 );
        newaxis->SetLabelSize( 0.04 );
        newaxis->Draw();
        
    } else {
        (fH2Dummy->GetYaxis())->SetNdivisions( 506 );
        (fH2Dummy->GetYaxis())->SetLabelFont( 42 );
        gPad->Update();
    }
    
    TLine vline;
    vline.SetLineStyle( 3 );
    vline.SetLineColor( kGray );
    vline.SetLineWidth( 1 );
    for ( unsigned int region = 1; region < 32; region++ ) {
        vline.DrawLine( region*(common::MAX_REGION+1), (fH2Dummy->GetYaxis())->GetXmin(),
                        region*(common::MAX_REGION+1), (fH2Dummy->GetYaxis())->GetXmax() );
    }
    gPad->Update();

    
    fMapCanvas->cd();
    
    // pad for the legend
    
    fMapPadLegend = new TPad( "fMapPadLegend", "", fRelativeSize, 0, 1, 1 );
    string legpadname = GetName( "fMapPadLegend" );
    fMapPadLegend->SetName( legpadname.c_str() );
    fMapPadLegend->SetBorderSize( 0 );
    fMapPadLegend->SetBorderMode( 0 );
    fMapPadLegend->SetFillColor( kWhite );
    fMapPadLegend->SetFillStyle( 4000 );
    fMapPadLegend->SetTopMargin( 0.079 );
    fMapPadLegend->SetBottomMargin( 0.09 );
    fMapPadLegend->SetLeftMargin( 0.001 );
    fMapPadLegend->SetRightMargin( 0.02 );
    fMapPadLegend->Draw();
    fMapPadLegend->cd();
    
    // add legend

    fMapLegend = new TPaveText ( 0.0236, 0.14, 0.97, 0.45, "NDC" );
    fMapLegend->SetBorderSize(0);
    fMapLegend->SetFillColor( kWhite );
    fMapLegend->SetTextSize( 0.1 );
    fMapLegend->SetTextSize( 0.1 );
    fMapLegend->SetTextFont(42);
    
    //--- firing frequency distribution of bad pixels

    fFireCanvas->cd();

    fHistoLegend = new TLegend( 0.21, 0.65, 0.42, 0.90 );
    fHistoLegend->SetBorderSize(0);
    fHistoLegend->SetTextSize( 0.03 );
    fHistoLegend->SetLineColor( kBlack );
    fHistoLegend->SetLineStyle( 1 );
    fHistoLegend->SetLineWidth( 1 );
    fHistoLegend->SetFillColor( kWhite );
    fHistoLegend->SetFillStyle( kFSolid );
    
    fMapCanvasReady = true;

    return;
}

//___________________________________________________________________
void THitMapDiscordant::AddDeadPixel( shared_ptr<TPixHit> pix )
{
    if ( !IsCanvasReady() ) {
        throw runtime_error( "THitMapDiscordant::AddDeadPixel() - canvas not ready!" );
    }

    fNDeadPixels++;
    
    fMapPadMain->cd();
    
    TMarker* marker = new TMarker();
    marker->SetMarkerStyle( fDeadStyle );
    marker->SetMarkerSize( fDeadSize );
    marker->SetMarkerColor( fDeadColor );
    float ym = pix->GetRow();
    if ( IsMapYInverted() ) {
        ym = fYMaxDummy - pix->GetRow();
    }
    float xm = pix->GetColumn();
    marker->DrawMarker( xm, ym );

    fHistoScale->Fill(0);
    fHistoDead->Fill(0);

    delete marker;
}

//___________________________________________________________________
void THitMapDiscordant::AddInefficientPixel( shared_ptr<TPixHit> pix,
                                             const unsigned int nTimesFired )
{
    if ( !IsCanvasReady() ) {
        throw runtime_error( "THitMapDiscordant::AddInefficientPixel() - canvas not ready!" );
    }

    fNInefficientPixels++;
    
    fMapPadMain->cd();
    
    TMarker* marker = new TMarker();
    marker->SetMarkerStyle( fIneffStyle );
    marker->SetMarkerSize( fIneffSize );
    marker->SetMarkerColor( fIneffColor );
    float ym = pix->GetRow();
    if ( IsMapYInverted() ) {
        ym = fYMaxDummy - pix->GetRow();
    }
    float xm = pix->GetColumn();
    marker->DrawMarker( xm, ym );
    
    fHistoScale->Fill( nTimesFired );
    fHistoInefficient->Fill( nTimesFired );

    delete marker;
}

//___________________________________________________________________
void THitMapDiscordant::AddHotPixel( shared_ptr<TPixHit> pix,
                                     const unsigned int nTimesFired )
{
    if ( !IsCanvasReady() ) {
        throw runtime_error( "THitMapDiscordant::AddHotPixel() - canvas not ready!" );
    }
    
    fNHotPixels++;
    
    fMapPadMain->cd();
    
    TMarker* marker = new TMarker();
    marker->SetMarkerStyle( fHotStyle );
    marker->SetMarkerSize( fHotSize );
    marker->SetMarkerColor( fHotColor );
    float ym = pix->GetRow();
    if ( IsMapYInverted() ) {
        ym = fYMaxDummy - pix->GetRow();
    }
    float xm = pix->GetColumn();
    marker->DrawMarker( xm, ym );
    
    fHistoScale->Fill( nTimesFired );
    fHistoHot->Fill( nTimesFired );

    delete marker;
}

//___________________________________________________________________
void THitMapDiscordant::Draw()
{
    FinishHitMap();
    DrawFiringFrequencyHisto();
    fSaveToFileReady = true;
}

//___________________________________________________________________
void THitMapDiscordant::SaveToFile( const char *fName )
{
    if ( !IsSaveToFileReady() ) {
        throw runtime_error( "THitMapDiscordant::SaveToFile() - not ready! Please use Draw() first." );
    }
    if ( fNDeadPixels | fNInefficientPixels | fNHotPixels ) {

        cout << "------------------------------- THitMapDiscordant::SaveToFile() " << endl;

        char fNameTemp[100];
        char fNameOpen[101], fNameClose[101];
        sprintf( fNameTemp,"%s", fName);
        sprintf( fNameOpen,"%s(", fNameTemp);
        sprintf( fNameClose,"%s)", fNameTemp);
        
        fMapCanvas->Print( fNameOpen, "Title:Hit map");
        fFireCanvas->Print( fNameClose, "Title:Firing frequency distribution");

        cout << "------------------------------- " << endl;

    } else {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapDiscordant::SaveToFile() - no bad pixel => nothing to save." << endl;
        }
    }
}

//___________________________________________________________________
void THitMapDiscordant::FinishHitMap()
{
    if ( !IsCanvasReady() ) {
        throw runtime_error( "THitMapDiscordant::FinishHitMap() - canvas not ready!" );
    }
    if ( fNDeadPixels | fNInefficientPixels | fNHotPixels ) {
        
        fMapPadLegend->cd();
        if ( fNDeadPixels ) {
            string label = "Dead pixels (";
            label += std::to_string( fNDeadPixels );
            label += ")";
            TText* textD = fMapLegend->AddText( label.c_str() );
            textD->SetTextColor( fDeadColor );
            textD->SetTextAlign(1);
        }
        if ( fNInefficientPixels ) {
            string label = "Inefficient pixels (";
            label += std::to_string( fNInefficientPixels );
            label += ")";
            TText* textI = fMapLegend->AddText( label.c_str() );
            textI->SetTextColor( fIneffColor );
            textI->SetTextAlign(1);
        }
        if ( fNHotPixels ) {
            string label = "Hot pixels (";
            label += std::to_string( fNHotPixels );
            label += ")";
            TText* textH = fMapLegend->AddText( label.c_str() );
            textH->SetTextColor( fHotColor );
            textH->SetTextAlign(1);
        }

        fMapLegend->Draw();
        
    } else {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapDiscordant::FinishHitMap() - no bad pixel => nothing to draw." << endl;
        }
    }
}

//___________________________________________________________________
void THitMapDiscordant::DrawFiringFrequencyHisto()
{
    if ( fNDeadPixels | fNInefficientPixels | fNHotPixels ) {

        fFireCanvas->cd();
        
        fHistoScale->Draw("AXIS");
        fHistoLegend->SetHeader( fHicChipName.c_str() );
        if ( fNDeadPixels ) {
            fHistoDead->Draw( "AH same" );
            fHistoLegend->AddEntry( fHistoDead->GetName(), "Dead pixels", "L"  );
        }
        if ( fNInefficientPixels ) {
            fHistoInefficient->Draw( "AH same" );
            fHistoLegend->AddEntry( fHistoInefficient->GetName(), "Inefficient pixels", "L"  );
        }
        if ( fNHotPixels ) {
            fHistoHot->Draw( "AH same" );
            fHistoLegend->AddEntry( fHistoHot->GetName(), "Hot pixels", "L"  );
        }
        
        TLine vline;
        vline.SetLineStyle( 1 );
        vline.SetLineColor( kGray );
        vline.SetLineWidth( 2 );
        vline.DrawLine( fNInjections, 0,
                        fNInjections, fHistoScale->GetMaximum()*1.05 );
        gPad->Update();

        fHistoLegend->Draw();
        
        
    } else {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapDiscordant::FinishHitMap() - no bad pixel => nothing to draw." << endl;
        }
    }
}


