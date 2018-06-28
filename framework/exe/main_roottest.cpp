#include "THitMapDiscordant.h"
#include "Common.h"
#include "TPixHit.h"
#include "TVerbosity.h"

#include <memory>
#include <iostream>

// ROOT includes
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
    
    if ( 0 ) {
        TFile* f =  new TFile("../../data/test.root", "RECREATE");
        TH1F* h =  new TH1F("h", "h", 100, 0., 1.);
        h->Fill(0.2, 50);
        h->Fill(0.3, 30);
        h->Fill(0.4, 20);
        h->Write();
        f->Close();
        delete f;
    }
    
    gStyle->SetPadGridX( false );
    gStyle->SetPadGridY( false );
    
    if ( 1 ) {
        common::TChipIndex idx;
        idx.boardIndex = 0;
        idx.dataReceiver = 3;
        idx.deviceType = TDeviceType::kMFT_LADDER2;
        idx.deviceId = 35;
        idx.chipId = 8;
        
        const unsigned int nInjections = 50;
        std::shared_ptr<THitMapDiscordant> hitmap =
            make_shared<THitMapDiscordant>( idx, nInjections );
        hitmap->BuildCanvas();
        
        const unsigned int size = 67;
        const unsigned int dcol[] = {
            0 , 1 , 3 , 4 , 5 , 9 , 20 , 22 , 23 , 29 , 31 , 33 , 36 , 37 , 38 , 40 , 42 ,46 , 52 , 55 , 60 , 61 , 63 , 65 , 67 , 70 , 71 , 80 , 83 , 84 , 88 , 90 , 93 , 97 , 98 , 99 , 102 , 104 , 105 , 108 , 111 , 113 , 115 , 120 , 122 , 123 , 126 , 131 , 134 , 137 , 139 , 144 , 145 , 148 , 150 , 155 , 159 , 161 , 167 , 172 , 174 , 175 , 177 , 197 , 199 , 206 , 210
        };
        const unsigned int addr = 970;
        for ( unsigned int ip = 0; ip < size; ip++ ) {
            std::shared_ptr<TPixHit> pix = make_shared<TPixHit>();
            pix->SetBoardIndex( idx.boardIndex );
            pix->SetBoardReceiver( idx.dataReceiver );
            pix->SetDeviceType( idx.deviceType );
            pix->SetDeviceId( idx.deviceId );
            pix->SetDoubleColumn( dcol[ip] );
            pix->SetAddress( addr );
            float region = floor( ((float)dcol[ip])/((float)common::NDCOL_PER_REGION) );
            pix->SetRegion( (unsigned int)region );
            pix->SetPixFlag( TPixFlag::kDEAD );
            hitmap->AddDeadPixel( pix );
        }
        hitmap->Draw();
        hitmap->SaveToFile( "plots.pdf" );
        
    }
    
    if ( 0 ) {
        TCanvas canvas ("canvas");
        canvas.cd();
        
        const float xMin = -0.5;
        const float xMax = 1023.5 ;
        const int xNbin = 1024;
        const float yMin = -0.5;
        const float yMax = 511.5;
        const int yNbin = 512;
        
        TH2F histo ("histo","Dead pixels map; Column; Row",
                    xNbin, xMin, xMax,
                    yNbin, yMin, yMax);
        histo.SetStats( kFALSE );
        histo.Draw("AXIS");
        
        histo.GetYaxis()->SetLabelOffset(999);
        histo.GetYaxis()->SetTickLength(0);
        histo.GetXaxis()->SetNdivisions( 512 );
        gPad->Update();
        TGaxis newaxis ( gPad->GetUxmin(),
                        gPad->GetUymax(),
                        gPad->GetUxmin()-0.001,
                        gPad->GetUymin(),
                        histo.GetYaxis()->GetXmin(),
                        histo.GetYaxis()->GetXmax(),
                        510, "+" );
        newaxis.SetLabelOffset(-0.03);
        newaxis.SetLabelFont( 42 );
        newaxis.Draw();
        
        
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
        
        canvas.Print("../../data/plots.pdf", "Title:my custom title");
    }


    return EXIT_SUCCESS;
}
