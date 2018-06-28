#include "TAlpideDecoder.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TChipConfig.h"
#include "TDevice.h"
#include "TDeviceOccupancyScan.h"
#include "TErrorCounter.h"
#include "THisto.h"
#include "THitMapView.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TScanConfig.h"
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <string.h>

using namespace std;

//___________________________________________________________________
TDeviceOccupancyScan::TDeviceOccupancyScan() :
TDeviceHitScan(),
fScanHisto( nullptr ),
fNTriggersPerTrain( 0 ),
fTriggerSource( TTriggerSource::kTRIG_INT )
{ 
    fScanHisto = make_shared<TScanHisto>();
}

//___________________________________________________________________
TDeviceOccupancyScan::TDeviceOccupancyScan( shared_ptr<TDevice> aDevice,
                                  shared_ptr<TScanConfig> aScanConfig ) : 
TDeviceHitScan( aDevice, aScanConfig),
fScanHisto( nullptr ),
fNTriggersPerTrain( 0 ),
fTriggerSource( TTriggerSource::kTRIG_INT )
{ 
    fScanHisto = make_shared<TScanHisto>();
    fChipDecoder->SetScanHisto( fScanHisto );
}

//___________________________________________________________________
TDeviceOccupancyScan::~TDeviceOccupancyScan()
{
    if ( fScanHisto ) fScanHisto.reset();
    fHitMapCollection.clear();
}

//___________________________________________________________________
void TDeviceOccupancyScan::Init()
{
    try {
        TDeviceHitScan::Init();
    } catch ( std::exception &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    if ( !fScanHisto ) {
        throw runtime_error( "TDeviceOccupancyScan::Init() - can not use a null pointer for the map of scan histo !" );
    }
    fChipDecoder->SetScanHisto( fScanHisto );
    fErrorCounter->Init( fScanHisto, fNTriggers );
    
    for ( unsigned int ichip = 0; ichip < fDevice->GetNWorkingChips(); ichip++ ) {
        common::TChipIndex aChipIndex = fDevice->GetWorkingChipIndex( ichip );
        AddHitMapToCollection( aChipIndex );
    }
}

//___________________________________________________________________
void TDeviceOccupancyScan::SetVerboseLevel( const int level )
{
    fScanHisto->SetVerboseLevel( level );
    for ( std::map<int, shared_ptr<THitMapView>>::iterator it = fHitMapCollection.begin(); it != fHitMapCollection.end(); ++it ) {
        ((*it).second)->SetVerboseLevel( level );
    }
    TDeviceHitScan::SetVerboseLevel( level );
}

//___________________________________________________________________
void TDeviceOccupancyScan::Go()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceOccupancyScan::Go() - not initialized ! Please use Init() first." );
    }

    const int nTrains = fNTriggers / fNTriggersPerTrain;
    const int nRest   = fNTriggers % fNTriggersPerTrain;
    int nTrigsThisTrain = 0;

    cout << "TDeviceOccupancyScan::Go() - fNTriggers: " << fNTriggers << endl;
    cout << "TDeviceOccupancyScan::Go() - fNTriggersPerTrain: " << fNTriggersPerTrain << endl;
    cout << "TDeviceOccupancyScan::Go() - nTrains: " << nTrains << endl;
    cout << "TDeviceOccupancyScan::Go() - nRest: " << nRest << endl;


    for ( int itrain = 0; itrain <= nTrains; itrain ++ ) { 

        cout << "TDeviceOccupancyScan::Go() - trains: " << itrain << endl;   
        if ( itrain == nTrains ) {
            nTrigsThisTrain = nRest;
        } else {
            nTrigsThisTrain = fNTriggersPerTrain;
        }
        // Send triggers for all boards
        for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
            (fDevice->GetBoard( ib ))->Trigger(nTrigsThisTrain);
        } 
        // Read data for all boards
        for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
            ReadEventData( ib,  nTrigsThisTrain );
        }
    } // end of loop on trigger trains
}

//___________________________________________________________________
void TDeviceOccupancyScan::Terminate()
{
    TDeviceChipVisitor::Terminate();
    FillHitMaps();
    cout << endl;
    fErrorCounter->Dump();
}

//___________________________________________________________________
bool TDeviceOccupancyScan::IsInternalTrigger() const
{
    if ( fTriggerSource == TTriggerSource::kTRIG_INT ) return true;
    return false;
}

//___________________________________________________________________
void TDeviceOccupancyScan::FillHitMaps()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceOccupancyScan::FillHitMaps() - not initialized ! Please use Init() first." );
    }
    for ( std::map<int, shared_ptr<THitMapView>>::iterator it = fHitMapCollection.begin(); it != fHitMapCollection.end(); ++it ) {
        ((*it).second)->FillHitMap();
    }
}

//___________________________________________________________________
void TDeviceOccupancyScan::WriteDataToFile( const char *fName, bool Recreate )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceOccupancyScan::WriteHitsToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceOccupancyScan::WriteHitsToFile() - not terminated ! Please use Terminate() first." );
    }
    
    for ( std::map<int, shared_ptr<THitMapView>>::iterator it = fHitMapCollection.begin(); it != fHitMapCollection.end(); ++it ) {
        ((*it).second)->WriteHitsToFile( fName, Recreate );    
    }
}

//___________________________________________________________________
void TDeviceOccupancyScan::DrawAndSaveToFile( const char *fName )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceOccupancyScan::DrawAndSaveToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceOccupancyScan::DrawAndSaveToFile() - not terminated ! Please use Terminate() first." );
    }
    for ( std::map<int, shared_ptr<THitMapView>>::iterator it = fHitMapCollection.begin(); it != fHitMapCollection.end(); ++it ) {
        try {
            ((*it).second)->BuildCanvas();
            ((*it).second)->Draw();
            ((*it).second)->SaveToFile( fName );
        } catch ( std::exception &err ) {
            cerr << err.what() << endl;
            exit( EXIT_FAILURE );
        }
    }
}

//___________________________________________________________________
void TDeviceOccupancyScan::AddHisto()
{
    common::TChipIndex id;
    
    THisto histo ("NoiseScanHisto", "NoiseScanHisto",
                  common::MAX_DCOL+1, 0, common::MAX_DCOL,
                  common::MAX_ADDR+1, 0, common::MAX_ADDR);
    
    for ( unsigned int ichip = 0; ichip < fDevice->GetNChips(); ichip++ ) {
        if ( fDevice->GetChipConfig(ichip)->IsEnabled() ) {
            id.boardIndex   = fDevice->GetBoardIndexByChip(ichip);
            id.dataReceiver = fDevice->GetChipConfig(ichip)->GetParamValue("RECEIVER");
            id.ladderId     = fDevice->GetLadderId();
            id.chipId       = fDevice->GetChipId(ichip);
            fScanHisto->AddHisto( id, histo );
        }
    }
    fScanHisto->FindChipList();
    if ( GetVerboseLevel() > kSILENT ) {
        cout << endl << "TDeviceOccupancyScan::AddHisto() - generated map with " << std::dec << fScanHisto->GetSize() << " elements" << endl;
    }
    return;
}

//___________________________________________________________________
void TDeviceOccupancyScan::AddHitMapToCollection( const common::TChipIndex idx )
{
    int int_index = common::GetMapIntIndex( idx );
    auto hitmap = make_shared<THitMapView>( fScanHisto, idx );
    fHitMapCollection.insert( std::pair<int, shared_ptr<THitMapView>>(int_index, hitmap) );
}

//___________________________________________________________________
void TDeviceOccupancyScan::InitScanParameters()
{
    fNTriggers = fScanConfig->GetNTriggers();
    fNTriggersPerTrain = fScanConfig->GetNTriggersPerTrain();
    fTriggerSource = fDevice->GetBoardConfig(0)->GetTriggerSource(); 
} 

//___________________________________________________________________
void TDeviceOccupancyScan::ConfigureBoards()
{
    shared_ptr<TReadoutBoard> theBoard;
    
    for ( unsigned int iboard = 0; iboard < fDevice->GetNBoards(false); iboard++ ) {
        
        theBoard = fDevice->GetBoard( iboard );
        if ( !theBoard ) {
            throw runtime_error( "TDeviceOccupancyScan::ConfigureBoard() - no readout board found." );
        }
        
        if ( fDevice->GetBoardConfig(iboard)->GetBoardType() == TBoardType::kBOARD_DAQ ) { // DAQ board
            
            // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
            // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
            const bool enablePulse = true;
            const bool enableTrigger = IsInternalTrigger() ? true : false; // condition ? value_if_true : value_if_false 
            const int triggerDelay = IsInternalTrigger() ? 10 : 0;
            const int pulseDelay = fDevice->GetBoardConfig(iboard)->GetParamValue( "STROBEDELAYBOARD" );
            theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
            theBoard->SetTriggerSource( TTriggerSource::kTRIG_EXT );      
            if ( !IsInternalTrigger( )) {
                shared_ptr<TBoardConfigDAQ> cnf = dynamic_pointer_cast<TBoardConfigDAQ>(fDevice->GetBoardConfig(iboard));
                cnf->SetStrobePulseSeq(0);
                cnf->SetPktBasedROEnable(true);
                cnf->SetNTriggers(0);
                shared_ptr<TReadoutBoardDAQ>myDAQBoard = dynamic_pointer_cast<TReadoutBoardDAQ>(fDevice->GetBoardConfig(iboard));
                myDAQBoard->WriteReadoutModuleConfigRegisters();
                myDAQBoard->WriteResetModuleConfigRegisters();
                myDAQBoard->WriteTriggerModuleConfigRegisters();
            }
      
        } else { // MOSAIC board
            
            const bool enablePulse = IsInternalTrigger() ? true : false; // condition ? value_if_true : value_if_false 
            const bool enableTrigger = IsInternalTrigger() ? true : false; 
            const int triggerDelay = fDevice->GetBoardConfig(iboard)->GetParamValue("STROBEDELAYBOARD");
            const int pulseDelay = 10 * (fDevice->GetBoardConfig(iboard)->GetParamValue("PULSEDELAY"));
            theBoard->SetTriggerConfig( enablePulse, enableTrigger, triggerDelay, pulseDelay );
            if ( IsInternalTrigger() ) theBoard->SetTriggerSource( TTriggerSource::kTRIG_INT );
            else theBoard->SetTriggerSource( TTriggerSource::kTRIG_EXT );            
        }
    }
}

//___________________________________________________________________
bool TDeviceOccupancyScan::HasData( const common::TChipIndex idx )
{
    if ( !fScanHisto->IsValidChipIndex( idx ) ) {
        return false;
    }
    return fScanHisto->HasData( idx );
}
