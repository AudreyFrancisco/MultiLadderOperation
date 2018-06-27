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
#include "RtypesCore.h"

using namespace std;

//___________________________________________________________________
THitMapView::THitMapView() : 
THitMap(),
fScanHisto( nullptr ),
fHitMap( nullptr ),
fHasData( false )
{
    fHitMap = new TH2F( "fHitMap", "",
            fXNbinDummy, fXMinDummy, fXMaxDummy,
            fYNbinDummy, fYMinDummy, fYMaxDummy );
    fHitMap->SetBit( kCanDelete );
}

//___________________________________________________________________
THitMapView::THitMapView(shared_ptr<TScanHisto> aScanHisto, 
                         const common::TChipIndex aChipIndex ) : 
THitMap( aChipIndex ),
fScanHisto( aScanHisto ),
fHitMap( nullptr ),
fHasData( false )
{
    fHitMap = new TH2F( "fHitMap", "",
            fXNbinDummy, fXMinDummy, fXMaxDummy,
            fYNbinDummy, fYMinDummy, fYMaxDummy );
    fHitMap->SetBit( kCanDelete );
    fChipIndex = aChipIndex;

    string name = GetName( "fHitMap" );
    fHitMap->SetName( name.c_str() );
    string title = GetHistoTitle( fHicChipName );
    fHitMap->SetTitle( title.c_str() );
}

//___________________________________________________________________
THitMapView::~THitMapView()
{
    // don't delete any other pointer to ROOT object
    // ROOT will take care by itself and delete anything in the Canvas
    if ( (!HasData()) && fHitMap ) delete fHitMap;
}

//___________________________________________________________________
void THitMapView::BuildCanvas()
{
    if ( !HasData() ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapView::BuildCanvas() - [board.rcv.ladder]chip = ["
                 << fChipIndex.boardIndex
                 << "." << fChipIndex.dataReceiver
                 << "." << fChipIndex.ladderId
                 << "]" << fChipIndex.chipId
                 << " : no data => no hit map." <<  endl; 
        }
        return;
    }
    if ( IsCanvasReady() ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "THitMapView::BuildCanvas() - canvas already built, nothing to be done." << endl;
        }
        return;
    }

    fMapCanvas->SetFillColor( kWhite );
    fMapCanvas->SetFillStyle( kFSolid );
    fMapCanvas->SetTopMargin( 0.08 );
    fMapCanvas->SetBottomMargin( 0.02 );
    fMapCanvas->SetLeftMargin( 0.02 );
    fMapCanvas->SetRightMargin( 0.04 );
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
            cout << "THitMapView::Draw() - [board.rcv.ladder]chip = ["
                 << fChipIndex.boardIndex
                 << "." << fChipIndex.dataReceiver
                 << "." << fChipIndex.ladderId
                 << "]" << fChipIndex.chipId
                 << " : no data => no hit map." <<  endl; 
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
    fHitMap->Draw("CONT1Z");
    gPad->Update();
    fSaveToFileReady = true;
}

//___________________________________________________________________
void THitMapView::FillHitMap()
{
    if ( !fScanHisto ) {
        throw runtime_error( "THitMapView::FillHitMap() - can not use a null pointer for the map of scan histo !" );
    }

    if ( !(fScanHisto->HasData(fChipIndex)) ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapView::FillHitMap() - [board.rcv.ladder]chip = ["
                 << fChipIndex.boardIndex
                 << "." << fChipIndex.dataReceiver
                 << "." << fChipIndex.ladderId
                 << "]" << fChipIndex.chipId
                 << " : no data => no hit map." <<  endl; 
        }
        fHasData = false;
        return;
    }

    TPixHit pixhit;
    pixhit.SetBoardIndex( fChipIndex.boardIndex );
    pixhit.SetBoardReceiver( fChipIndex.dataReceiver );
    pixhit.SetLadderId( fChipIndex.ladderId );
    pixhit.SetChipId( fChipIndex.chipId );

    for ( unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++ ) {
        for ( unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++ ) {
            pixhit.SetDoubleColumn( icol );
            pixhit.SetAddress( iaddr );
            unsigned int column = pixhit.GetColumn();
            unsigned int row = pixhit.GetRow();
            double hits = (*fScanHisto)(fChipIndex,icol,iaddr);
            if (hits > 0) {
                fHitMap->Fill( row, column, hits );
            }
        }
    } 
    fHasData = true;
}

//___________________________________________________________________
void THitMapView::WriteHitsToFile( const char *fName, const bool Recreate )
{
    if ( !HasData() ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapView::WriteHitsToFile() - [board.rcv.ladder]chip = ["
                 << fChipIndex.boardIndex
                 << "." << fChipIndex.dataReceiver
                 << "." << fChipIndex.ladderId
                 << "]" << fChipIndex.chipId
                 << " : no data => no hit map." <<  endl; 
        }
        return;
    }

    if ( GetVerboseLevel() > kSILENT ) {
        cout << "THitMapView::WriteHitsToFile() - [board.rcv.ladder]chip = [" 
             << fChipIndex.boardIndex
             << "." << fChipIndex.dataReceiver
             << "." << fChipIndex.ladderId
             << "]" << fChipIndex.chipId << endl;
    }

    char  fNameChip[100];    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    string suffix( fNameTemp );
    string filename = common::GetFileName( fChipIndex, suffix );
    strcpy( fNameChip, filename.c_str());

    FILE *fp;
    if ( Recreate ) fp = fopen(fNameChip, "w");
    else            fp = fopen(fNameChip, "a");
    if ( !fp ) {
        throw runtime_error( "THitMapView::WriteHitsToFile() - output file not found." );
    }
    if ( GetVerboseLevel() > kSILENT ) {
        cout << "THitMapView::WriteDataToFile() - Writing data to file "<< fNameChip << endl;
    }

    TPixHit pixhit;
    pixhit.SetBoardIndex( fChipIndex.boardIndex );
    pixhit.SetBoardReceiver( fChipIndex.dataReceiver );
    pixhit.SetLadderId( fChipIndex.ladderId );
    pixhit.SetChipId( fChipIndex.chipId );

    for ( unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++ ) {
        for ( unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++ ) {
            pixhit.SetDoubleColumn( icol );
            pixhit.SetAddress( iaddr );
            unsigned int column = pixhit.GetColumn();
            unsigned int row = pixhit.GetRow();
            double hits = (*fScanHisto)(fChipIndex,icol,iaddr);
            if (hits > 0) {
                fprintf(fp, "%d %d %d\n", row, column, (int)hits);
            }
        }
    } 
    if (fp) fclose (fp);
}

//___________________________________________________________________
void THitMapView::SaveToFile( const char *fName )
{
    if ( !HasData() ) {
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "THitMapView::SaveToFile() - [board.rcv.ladder]chip = ["
                 << fChipIndex.boardIndex
                 << "." << fChipIndex.dataReceiver
                 << "." << fChipIndex.ladderId
                 << "]" << fChipIndex.chipId
                 << " : no data => no hit map." <<  endl; 
        }
        return;
    }
    if ( !IsSaveToFileReady() ) {
        throw runtime_error( "THitMapDiTHitMapViewscordant::SaveToFile() - not ready! Please use Draw() first." );
    }
    char  fNameChip[100];
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    string suffix( fNameTemp );
    
    string filename = common::GetFileName( fChipIndex, suffix, "", ".pdf" );
    strcpy( fNameChip, filename.c_str() );

    fMapCanvas->Print( fNameChip );
}
