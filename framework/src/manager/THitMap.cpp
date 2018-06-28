#include "THitMap.h"

#include <iomanip>
#include <iostream>

// ROOT includes
#include "Rtypes.h"
#include "TStyle.h"
#include "TROOT.h" // useful for global ROOT pointers (such as gPad)
#include "TCanvas.h"
#include "TH2F.h"
#include "RtypesCore.h"

using namespace std;

const float THitMap::fXMinDummy = -0.5;
const float THitMap::fXMaxDummy = 1023.5;
const int THitMap::fXNbinDummy = 1024;
const float THitMap::fYMinDummy = -0.5;
const float THitMap::fYMaxDummy = 511.5;
const int THitMap::fYNbinDummy = 512;

//___________________________________________________________________
THitMap::THitMap() :
TVerbosity(),
fYaxisTitle( "Row" ),
fXaxisTitle( "Column" ),
fHicChipName( "" ),
fNInjections( 50 ),
fMapCanvas( nullptr ),
fH2Dummy( nullptr ),
fYInvertedDummy( true ),
fMapCanvasReady( false ),
fSaveToFileReady( false )
{
    fIdx.boardIndex = 0;
    fIdx.dataReceiver = 0;
    fIdx.ladderId = 0;
    fIdx.chipId = 0;
    fMapCanvas = new TCanvas( "cv" );
    fH2Dummy = new TH2F ( "h2dummy", "",
             fXNbinDummy, fXMinDummy, fXMaxDummy,
             fYNbinDummy, fYMinDummy, fYMaxDummy ),
    fH2Dummy->SetBit( kCanDelete );

    SetBaseStyle();
    fMapCanvas->UseCurrentStyle();
}

//___________________________________________________________________
THitMap::THitMap( const common::TChipIndex aChipIndex, const unsigned int nInjections ) :
TVerbosity(),
fYaxisTitle( "Row" ),
fXaxisTitle( "Column" ),
fHicChipName( "" ),
fNInjections( 50 ),
fMapCanvas( nullptr ),
fH2Dummy( nullptr ),
fYInvertedDummy( true ),
fMapCanvasReady( false ),
fSaveToFileReady( false )
{
    SetNInjections( nInjections );
    
    fIdx.boardIndex = aChipIndex.boardIndex;
    fIdx.dataReceiver = aChipIndex.dataReceiver;
    fIdx.ladderId = aChipIndex.ladderId;
    fIdx.chipId = aChipIndex.chipId;
    
    fMapCanvas = new TCanvas( "cv" );
    fH2Dummy = new TH2F ( "h2dummy", "",
                         fXNbinDummy, fXMinDummy, fXMaxDummy,
                         fYNbinDummy, fYMinDummy, fYMaxDummy ),
    fH2Dummy->SetBit( kCanDelete );
    
    
    SetBaseStyle();
    fMapCanvas->UseCurrentStyle();
    
    SetHicChipName();
    
    string dummyname = GetName( "h2dummy" );
    fH2Dummy->SetName( dummyname.c_str() );
    string title = GetHistoTitle( fHicChipName );
    fH2Dummy->SetTitle( title.c_str() );
    
    string cvname = GetName( "fMapCanvas" );
    fMapCanvas->SetName( cvname.c_str() );
}

//___________________________________________________________________
THitMap::~THitMap()
{
    // don't delete any other pointer to ROOT object
    // ROOT will take care by itself and delete anything in the Canvas
    fMapCanvas->Clear();
    delete fMapCanvas;
}

//___________________________________________________________________
void THitMap::SetNInjections( const unsigned int value )
{
    if ( value == 0 ) {
        cerr << "THitMap::SetNInjections() - zero injection is not valid !" << endl;
        return;
    }
    fNInjections = value;
}

//___________________________________________________________________
void THitMap::SetHicChipName()
{
    fHicChipName = "Board ";
    fHicChipName += std::to_string( fIdx.boardIndex );
    fHicChipName = " RCV ";
    fHicChipName += std::to_string( fIdx.dataReceiver );
    if ( fIdx.ladderId ) {
        fHicChipName = " Hic ";
        fHicChipName += std::to_string( fIdx.ladderId );
    }
    fHicChipName += " Chip ";
    fHicChipName += std::to_string( fIdx.chipId );
}

//___________________________________________________________________
void THitMap::SetBaseStyle()
{

    gStyle->SetCanvasColor(0);
    gStyle->SetPadColor(0);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetFrameBorderMode(0);
    
    gStyle->SetOptStat(0);
    
    gStyle->SetPadTickX( false );
    gStyle->SetPadTickY( false );
    gStyle->SetPadGridX( false );
    gStyle->SetPadGridY( false );
    
    gStyle->SetLegendFont(42);
}

//___________________________________________________________________
string THitMap::GetHistoTitle( const std::string prefix ) const
{
    string title =  prefix + ";" + fXaxisTitle + ";" + fYaxisTitle;
    return title;
}

//___________________________________________________________________
string THitMap::GetName( const string prefix ) const
{
    string name = prefix;

    name += "_board ";
    name += std::to_string( fIdx.boardIndex );
    name += "_rcv";
    name += std::to_string( fIdx.dataReceiver );
    if ( fIdx.ladderId ) {
        name += "_hic";
        name += std::to_string( fIdx.ladderId );
    }
    name += "_chip";
    name += std::to_string( fIdx.chipId );
    return name;
}

