#include "TStorePixHit.h"
#include "TPixHit.h"
#include "Common.h"
#include <stdexcept>
#include <iostream>
#include <string.h>
// ROOT includes
#include "TFile.h"
#include "TTree.h"


using namespace std;

//___________________________________________________________________
TStorePixHit::TStorePixHit() : 
TVerbosity(),
fTree( nullptr ),
fFile( nullptr ),
fSuccessfulInit( false ),
fNEntriesAutoSave( 10000 )
{
    fData.boardIndex = 0;
    fData.dataReceiver = 0;
    fData.deviceType = (unsigned int)TDeviceType::kMFT_LADDER2;
    fData.deviceId = 0;
    fData.chipId = 8;
    fData.row = 0;
    fData.col = 0;
    fData.bunchNum = 0;
    fData.trgNum = 0;
    fData.trgTime = 0;
}

//___________________________________________________________________
TStorePixHit::~TStorePixHit()
{
    if ( fTree ) delete fTree;
    if ( fFile ) delete fFile;
}

//___________________________________________________________________
void TStorePixHit::SetNames( const char *baseName,
                            const common::TChipIndex aChipIndex )
{
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", baseName );
    strtok( fNameTemp, "." );
    string prefix( fNameTemp );
    
    // name of the output ROOT file
    SetFileName( prefix.c_str(), aChipIndex );

    // title of the TTree
    SetTreeTitle( aChipIndex );
}

//___________________________________________________________________
void TStorePixHit::SetCyclicAutoSave( long nEntries )
{
    if ( nEntries == 0 ) return;
    fNEntriesAutoSave = nEntries;
}

//___________________________________________________________________
void TStorePixHit::Init()
{
    if ( fOutFileName.empty() || fTreeTitle.empty() ) {
        throw runtime_error( "TStorePixHit::Init() - empty output names ! Please use SetNames() first." );
    }
    if ( !fFile ) {
        try {
            fFile = new TFile( fOutFileName.c_str(), "RECREATE" );
        } catch ( exception& msg ) {
            cerr << msg.what() << endl;
            exit( EXIT_FAILURE );
        }
    }
    if ( !fTree ) {
        if ( GetVerboseLevel() > kTERSE ) {
            cout << "TStorePixHit::Init() - creating tree in file " << fOutFileName << endl;
        }
        fTree = new TTree( "pixTree", fTreeTitle.c_str() );
        fTree->Branch( "fData", &fData, 
                        "boardIndex/i:dataReceiver/i:deviceType/i:deviceId/i:chipId/i:row/i:col/i:bunchNum/i:trgNum/i:trgTime/l" );
        fTree->SetAutoSave( fNEntriesAutoSave ); // flush the TTree to disk every N entries
        fTree->SetDirectory( fFile );
        fTree->SetImplicitMT(true);
    }
    fSuccessfulInit = true;
}

//___________________________________________________________________
void TStorePixHit::Fill( std::shared_ptr<TPixHit> hit )
{
    if ( !IsInitOk() ) {
        throw runtime_error( "TStorePixHit::Fill() - object not (successfully) initialized ! Please use Init() first." );
    }
    if ( (!fFile) || (fFile->IsZombie()) ) {
        fSuccessfulInit = false;
        throw runtime_error( "TStorePixHit::Fill() - no viable output file ! Please use Init() first." );
    }
    if ( !fTree ) {
        fSuccessfulInit = false;
        throw runtime_error( "TStorePixHit::Fill() - no TTree ! Please use Init() first." );
    }
    try {
        SetDataSummary( hit );
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        return;
    }
    fTree->Fill();
}

//___________________________________________________________________
void TStorePixHit::Terminate()
{
    fTree->Write();
    fFile->Close();
    if ( GetVerboseLevel() > kTERSE ) {
            cout << "TStorePixHit::Terminate() - tree written in file " << fOutFileName << endl;
    }
}

//___________________________________________________________________
void TStorePixHit::SetDataSummary( std::shared_ptr<TPixHit> hit )
{
    if ( !hit ) {
        throw runtime_error( "TStorePixHit::SetDataSummary() - can not use a null pointer !" );
        return;
    }
    fData.boardIndex = hit->GetBoardIndex();
    fData.dataReceiver = hit->GetBoardReceiver();
    fData.deviceType = (unsigned int)hit->GetDeviceType();
    fData.deviceId = hit->GetDeviceId();
    fData.chipId = hit->GetChipId();
    fData.row = hit->GetRow();
    fData.col = hit->GetColumn();
    fData.bunchNum = hit->GetBunchCounter();
    fData.trgNum = hit->GetTriggerNum();
    fData.trgTime = hit->GetTriggerTime();
}

//___________________________________________________________________
void TStorePixHit::SetFileName( string prefix, const common::TChipIndex aChipIndex )
{
    fOutFileName = "../../data/";
    fOutFileName += prefix;
    fOutFileName += "-B";
    fOutFileName += std::to_string( aChipIndex.boardIndex );
    if ( common::IsMFTladder(aChipIndex) || common::IsIBhic(aChipIndex) ) {
        if ( common::IsMFTladder(aChipIndex) ) {
            fOutFileName += "-ladder";
        }
        if ( common::IsIBhic(aChipIndex) ) {
            fOutFileName += "-ibhic";
        }
        fOutFileName += std::to_string( aChipIndex.deviceId );
    }
    fOutFileName += ".root";
}

//___________________________________________________________________
void TStorePixHit::SetTreeTitle( const common::TChipIndex aChipIndex )
{
    fTreeTitle = "Events ";
    fTreeTitle += " Board ";
    fTreeTitle += std::to_string( aChipIndex.boardIndex );
    if ( common::IsMFTladder(aChipIndex) || common::IsIBhic(aChipIndex) ) {
        if ( common::IsMFTladder(aChipIndex) ) {
            fTreeTitle += " Ladder ";
        }
        if ( common::IsIBhic(aChipIndex) ) {
            fTreeTitle += " IB hic ";
        }
        fTreeTitle += std::to_string( aChipIndex.deviceId );
    }
}