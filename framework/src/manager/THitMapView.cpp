#include "THitMapView.h"
#include "THisto.h"
#include "TPixHit.h"
#include "TVerbosity.h"
#include <stdexcept>
#include <iostream>
#include <string.h>
// ROOT includes
#include "Rtypes.h"
#include "TStyle.h"
#include "TROOT.h" // useful for global ROOT pointers (such as gPad)
#include "TCanvas.h"
#include "TH2F.h"
#include "TString.h"
#include "TFile.h"
#include "RtypesCore.h"

using namespace std;

//___________________________________________________________________
THitMapView::THitMapView() : 
THitMap(),
fScanHisto( nullptr ),
fHisto2D( nullptr ),
fHasData( false )
{
    fHisto2D = new TH2F( "fHisto2D", "",
            fXNbinDummy, fXMinDummy, fXMaxDummy,
            fYNbinDummy, fYMinDummy, fYMaxDummy );
    fHisto2D->SetBit( kCanDelete );
}

//___________________________________________________________________
THitMapView::THitMapView(const TDeviceType dt,
                         shared_ptr<TScanHisto> aScanHisto, 
                         const common::TChipIndex aChipIndex ) : 
THitMap( dt, aChipIndex ),
fScanHisto( aScanHisto ),
fHisto2D( nullptr ),
fHasData( false )
{
    fHisto2D = new TH2F( "fHisto2D", "",
            fXNbinDummy, fXMinDummy, fXMaxDummy,
            fYNbinDummy, fYMinDummy, fYMaxDummy );
    fHisto2D->SetBit( kCanDelete );

    fChipIndex = aChipIndex;

    string name = GetName( "fHisto2D" );
    fHisto2D->SetName( name.c_str() );
    string title = GetHistoTitle( fHicChipName );
    fHisto2D->SetTitle( title.c_str() );
}

//___________________________________________________________________
THitMapView::~THitMapView()
{
    // don't delete any other pointer to ROOT object
    // ROOT will take care by itself and delete anything in the Canvas
    if ( (!HasData()) && fHisto2D ) delete fHisto2D;
}

//___________________________________________________________________
void THitMapView::BuildCanvas()
{
    if ( IsCanvasReady() ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "THitMapView::BuildCanvas() - canvas already built, nothing to be done." << endl;
        }
        return;
    }

    fMapCanvas->SetFillColor( kWhite );
    fMapCanvas->SetFillStyle( kFSolid );
    fMapCanvas->SetRightMargin( 0.12 );
    fMapCanvas->cd();

    fH2Dummy->SetStats( kFALSE );
    (fH2Dummy->GetXaxis())->SetNdivisions( 506 );
    (fH2Dummy->GetXaxis())->SetLabelFont( 42 );
    (fH2Dummy->GetXaxis())->SetLabelSize( 0.04 );
    (fH2Dummy->GetXaxis())->SetTitleSize( 0.04 );
    (fH2Dummy->GetXaxis())->SetTitleOffset( 1.3 );
    (fH2Dummy->GetYaxis())->SetTitleSize( 0.04 );
    (fH2Dummy->GetYaxis())->SetTitleOffset( 1.15 );
    fH2Dummy->Draw( "AXIS" );
    (fH2Dummy->GetYaxis())->SetNdivisions( 506 );
    (fH2Dummy->GetYaxis())->SetLabelFont( 42 );
    gPad->Update();

    fMapCanvasReady = true;

    return;
}

//___________________________________________________________________
void THitMapView::Draw()
{
    if ( !HasData() ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapView::Draw() - ";
            common::DumpId( fChipIndex );
            cout << " : no data => no hit map." <<  endl; 
        }
        return;
    }
    if ( !IsCanvasReady() ) {
        throw runtime_error( "THitMapView::Draw() - canvas not ready!" );
    }
    gStyle->SetPalette( 51 ); // DeepSea palette
	//gStyle->SetPalette( 1, 0 ); // pretty palette (rainbow)
	//gStyle->SetPalette( 7 ); // grey palette
    fMapCanvas->cd();
    fHisto2D->Draw("CONT1Z");
    gPad->Update();
    fMapCanvas->Update();
    fSaveToFileReady = true;
}

//___________________________________________________________________
void THitMapView::WriteHitsToFile( const char *baseFName, const bool Recreate )
{
    if ( !(fScanHisto->HasData(fChipIndex)) ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapView::WriteHitsToFile() - ";
            common::DumpId( fChipIndex );
            cout << " : no data => no hit map." <<  endl; 
        }
        fHasData = false;
        return;
    }

    if ( GetVerboseLevel() > kSILENT ) {
        cout << "THitMapView::WriteHitsToFile() - ";
        common::DumpId( fChipIndex ); 
        cout << endl;
    }

    char  filenameChip[100];    
    char filenameTemp[100];
    sprintf( filenameTemp,"%s", baseFName);
    strtok( filenameTemp, "." );
    string suffix( filenameTemp );
    string filenamePlot = common::GetFileName( fChipIndex, suffix );
    strcpy( filenameChip, filenamePlot.c_str());

    FILE *fp;
    if ( Recreate ) fp = fopen(filenameChip, "w");
    else            fp = fopen(filenameChip, "a");
    if ( !fp ) {
        throw runtime_error( "THitMapView::WriteHitsToFile() - output file not found." );
    }
    if ( GetVerboseLevel() > kSILENT ) {
        cout << "THitMapView::WriteDataToFile() - Writing data to file "<< filenameChip << endl;
    }

    TPixHit pixhit;
    pixhit.SetPixChipIndex( fChipIndex );
    for ( unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++ ) {
        for ( unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++ ) {
            pixhit.SetDoubleColumn( icol );
            pixhit.SetAddress( iaddr );
            unsigned int column = pixhit.GetColumn();
            unsigned int row = pixhit.GetRow();
            double hits = (*fScanHisto)(fChipIndex,icol,iaddr);
            if (hits > 0) {
                fHisto2D->Fill( column, row, hits );
                fprintf(fp, "%d %d %d\n", row, column, (int)hits);
            }
        }
    } 
    if (fp) fclose (fp);
    fHasData = true;
}

//___________________________________________________________________
void THitMapView::SaveToFile( const char *baseFName )
{
    if ( !HasData() ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapView::SaveToFile() - ";
            common::DumpId( fChipIndex );
            cout << " : no data => no hit map." <<  endl; 
        }
        return;
    }
    if ( !IsSaveToFileReady() ) {
        throw runtime_error( "THitMapDiTHitMapViewscordant::SaveToFile() - not ready! Please use Draw() first." );
    }

    char filenameChip[100];
    char filenameTemp[100];
    sprintf( filenameTemp,"%s", baseFName);
    strtok( filenameTemp, "." );
    string suffix( filenameTemp );
    
    // output ROOT file
    string filenameRoot = common::GetFileName( fChipIndex, suffix, "", ".root" );
    strcpy( filenameChip, filenameRoot.c_str() );
    TString name( filenameChip );
    TFile outfile( name.Data(), "RECREATE" );
    outfile.cd();
    fHisto2D->Write();
    outfile.Close();

    // output plot
    string filenamePlot = common::GetFileName( fChipIndex, suffix, "", ".pdf" );
    strcpy( filenameChip, filenamePlot.c_str() );
    fMapCanvas->Print( filenameChip );

}
