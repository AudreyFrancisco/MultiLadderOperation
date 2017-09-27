#include "TFile.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TPDF.h"
#include "TROOT.h" // useful for global ROOT pointers (such as gPad)
#include "TGaxis.h"
#include "TMarker.h"
#include "TLine.h"
#include "Rtypes.h"
#include "TStyle.h"

using namespace std;

// This macro is a simple-test executable to check the linking against ROOT
int main() {
    
    TFile* f =  new TFile("../../data/test.root", "RECREATE");
    TH1F* h =  new TH1F("h", "h", 100, 0., 1.);
    h->Fill(0.2, 50);
    h->Fill(0.3, 30);
    h->Fill(0.4, 20);
    h->Write();
    f->Close();
    
    gStyle->SetPadGridX( false );
    gStyle->SetPadGridY( false );
    
    TCanvas* canvas = new TCanvas("canvas");
    canvas->cd();
    
    const float xMin = -0.5;
    const float xMax = 1023.5 ;
    const int xNbin = 1024;
    const float yMin = -0.5;
    const float yMax = 511.5;
    const int yNbin = 512;
    
    TH2F* histo = new TH2F("histo","Dead pixels map; Column; Row",
                           xNbin, xMin, xMax,
                           yNbin, yMin, yMax);
    histo->SetStats( kFALSE );
    histo->Draw("AXIS");
    
    histo->GetYaxis()->SetLabelOffset(999);
    histo->GetYaxis()->SetTickLength(0);
    histo->GetXaxis()->SetNdivisions( 512 );
    gPad->Update();
    TGaxis* newaxis = new TGaxis( gPad->GetUxmin(),
                                 gPad->GetUymax(),
                                 gPad->GetUxmin()-0.001,
                                 gPad->GetUymin(),
                                 histo->GetYaxis()->GetXmin(),
                                 histo->GetYaxis()->GetXmax(),
                                 510, "+" );
    newaxis->SetLabelOffset(-0.03);
    newaxis->SetLabelFont( 42 );
    newaxis->Draw();
    
    
    TMarker marker;
    marker.SetMarkerStyle( kOpenCircle );
    marker.SetMarkerSize( 0.4 );
    marker.SetMarkerColor( kBlack );
    const float ym = yMax-450;
    for ( unsigned int xm = 0; xm < 512; xm++ ) {
        if ( xm%4 == 0 ) {
            marker.DrawMarker( xm, ym );
        }
    }
    gPad->Update();
    
    TLine vline;
    vline.SetLineStyle( 3 );
    vline.SetLineColor( kGray );
    vline.SetLineWidth( 1 );
    for ( unsigned int region = 1; region < 32; region++ ) {
        vline.DrawLine( region*32, yMin, region*32, yMax );
    }
    gPad->Update();
    
    canvas->Print("../../data/plots.pdf", "Title:my custom title");

    return EXIT_SUCCESS;
}
