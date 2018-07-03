#include "TMultiDeviceOperator.h"
#include "TDevice.h"
#include "TDeviceHitScan.h"
#include "TSetup.h"
#include "TDeviceDigitalScan.h"
#include "TDeviceThresholdScan.h"
#include "TDeviceOccupancyScan.h"
#include "TReadoutBoard.h"
#include "TAlpide.h"
#include "TScanConfig.h"
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std;

//___________________________________________________________________
TMultiDeviceOperator::TMultiDeviceOperator() : 
TVerbosity(),
fScanType( MultiDeviceScanType::kNOISE_OCC_SCAN ),
fNDevices( 0 ),
fIsAdmissionClosed( false ),
fIsInitDone( false )
{
    SetVerboseLevel( 2 );
}

//___________________________________________________________________
TMultiDeviceOperator::~TMultiDeviceOperator()
{
    fDevices.clear();
    fDeviceOperators.clear();
    fSetups.clear();
}

//___________________________________________________________________
void TMultiDeviceOperator::SetScanType( const MultiDeviceScanType value )
{
    if ( !IsAdmissionClosed() ) {
        fScanType = value;
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::AddSetup( const std::string aConfigFileName )
{
    if ( IsAdmissionClosed() ) {
        cerr << "TMultiDeviceOperator::AddSetup() - not possible, admission is closed" << endl;
        return;
    }
    auto mySetup = make_shared<TSetup>();
    mySetup->SetVerboseLevel( this->GetVerboseLevel() );
    mySetup->SetConfigFileName( aConfigFileName );
    mySetup->ReadConfigFile();
    shared_ptr<TDevice> theDevice = mySetup->GetDevice();
    const int nBoards = theDevice->GetNBoards( false );
    if ( !nBoards ) {
        cerr << "TMultiDeviceOperator::AddSetup() - #" << fNDevices << " , config file " << aConfigFileName << endl;
        throw runtime_error( "TMultiDeviceOperator::AddSetup() - No board found!" );
    }    
    const int nWorkingChips = theDevice->GetNWorkingChips();
    if ( !nWorkingChips ) {
        cerr << "TMultiDeviceOperator::AddSetup() - #" << fNDevices << ", config file " << aConfigFileName << endl;
        throw runtime_error( "TMultiDeviceOperator::AddSetup() - No working chip found!" );
    }

    fSetups.push_back( move(mySetup) );
    fDevices.push_back( (fSetups.back())->GetDevice() );
    (fDevices.back())->SetUniqueBoardId( fNDevices );
    if ( GetVerboseLevel() > kTERSE ) {
        cout << "TMultiDeviceOperator::AddSetup() - added setup #" << fNDevices << " with config file " << aConfigFileName << endl << endl;
    }
    fNDevices++;
}

//___________________________________________________________________
void TMultiDeviceOperator::CloseAdmission()
{
    if ( IsAdmissionClosed() ) {
        cerr << "TMultiDeviceOperator::CloseAdmission() - doing nothing, admission was already closed" << endl;
        return;
    }
    fIsAdmissionClosed = true;
    fNDevices = fSetups.size();
    if ( !fNDevices ) {
        throw runtime_error( "TMultiDeviceOperator::CloseAdmission() - No device found!" );
    }
    if ( GetVerboseLevel() > kTERSE ) {
        cout << "TMultiDeviceOperator::CloseAdmission() - found  " << fNDevices << " devices" << endl;
    }
    for ( unsigned int i = 0; i < fNDevices; i++ ) {
        
        shared_ptr<TDevice> theDevice = (fSetups.at(i))->GetDevice();
        shared_ptr<TScanConfig> theScanConfig = (fSetups.at(i))->GetScanConfig();
        

        switch ((int)fScanType)
        {
            case (int)MultiDeviceScanType::kDIGITAL_SCAN:
            {
                shared_ptr<TDeviceDigitalScan> theDeviceTestor = make_shared<TDeviceDigitalScan>( theDevice, theScanConfig );
                theDeviceTestor->SetVerboseLevel( this->GetVerboseLevel() );
                theDeviceTestor->SetRescueBadChipId( true );
                theDeviceTestor->SetActivateTTree( false );
                theDeviceTestor->SetPrefixFilename( fName );
                theDeviceTestor->Init();
                fDeviceOperators.push_back( move(theDeviceTestor) );
                break;
            }
            default: // noise occupancy scan
            {
                shared_ptr<TDeviceOccupancyScan> theDeviceTestor = make_shared<TDeviceOccupancyScan>( theDevice, theScanConfig );
                theDeviceTestor->SetVerboseLevel( this->GetVerboseLevel() );
                theDeviceTestor->SetRescueBadChipId( true );
                theDeviceTestor->SetActivateTTree( true ); // true for noise occupancy scan
                theDeviceTestor->SetPrefixFilename( fName );
                theDeviceTestor->Init();
                fDeviceOperators.push_back( move(theDeviceTestor) );
                break;
            }
        }
    }
    sleep(1);
    CheckInitAllOk();
}

//___________________________________________________________________
void TMultiDeviceOperator::Go() 
{
    if ( !fSetups.size() ) {
        throw runtime_error( "TMultiDeviceOperator::Go() - No setup found! Please use AddSetup() first." );
    }
    if ( !fDevices.size() ) {
        throw runtime_error( "TMultiDeviceOperator::Go() - No device found! Please use AddSetup() first." );
    }
    if ( !fDeviceOperators.size() ) {
        throw runtime_error( "TMultiDeviceOperator::Go() - No device operator found! Please use CloseAdmission() first." );
    }
    if ( !fIsInitDone ) {
        throw runtime_error( "TMultiDeviceOperator::Go() - A device operator is not correctly initialized!" );
    }
    switch ((int)fScanType)
    {
        case (int)MultiDeviceScanType::kDIGITAL_SCAN: 
            GoDigitalScan();            
            break;

        default: 
            GoNoiseOccScan();
            break;
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::Terminate()
{
    switch ((int)fScanType)
    {
        case (int)MultiDeviceScanType::kDIGITAL_SCAN: 
            TerminateDigitalScan();            
            break;

        default: 
            TerminateNoiseOccScan();
            break;
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::CheckInitAllOk()
{
    fIsInitDone = true;
    for ( unsigned int i = 0; i < fDeviceOperators.size(); i++ ) {
        fIsInitDone = fIsInitDone && ((fDeviceOperators.at(i))->IsInitDone());
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::GoDigitalScan()
{    
    int configNMaskStages = ((fSetups.at(0))->GetScanConfig())->GetNMaskStages();
    int NStages = configNMaskStages;
    if ( configNMaskStages < 0 ) {
        NStages = 1;
    }
    int nTriggers = ((fSetups.at(0))->GetScanConfig())->GetNInj();
    int nPixPerRegion = ((fSetups.at(0))->GetScanConfig())->GetPixPerRegion();
    
    unsigned int nHitsTot = 0, nHitsLastStage = 0;

    for ( int istage = 0; istage < NStages; istage ++ ) {
        
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TMultiDeviceOperator::GoDigitalScan() - Mask stage "
                 << std::dec << istage << endl;
        }
        if ( configNMaskStages < 0 ) {
            DoConfigureMaskStage( nPixPerRegion, configNMaskStages );
        } else {
            DoConfigureMaskStage( nPixPerRegion, istage );
        }
        
        DoTrigger( nTriggers );

        if ( istage ) {
            nHitsLastStage = (fDeviceOperators.at(0))->GetNHits();
            for ( unsigned int d = 1; d < fDeviceOperators.size() ; d++ ) {
                nHitsLastStage += (fDeviceOperators.at(d))->GetNHits();
            }
        }
        usleep(2000);

        // Read data for all boards, multithread (one per board)
        ReadEventData();
        
        nHitsTot = (fDeviceOperators.at(0))->GetNHits();
        for ( unsigned int d = 1; d < fDeviceOperators.size() ; d++ ) {
            nHitsTot += (fDeviceOperators.at(d))->GetNHits();
        }
        nHitsTot -= - nHitsLastStage;
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TMultiDeviceOperator::GoDigitalScan() - stage "
                 << std::dec << istage << " , found n hits = " << nHitsTot << endl;
        }
        usleep(200);
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::GoNoiseOccScan()
{   
    const int nTriggers = ((fSetups.at(0))->GetScanConfig())->GetNTriggers();
    const int nTriggersPerTrain = ((fSetups.at(0))->GetScanConfig())->GetNTriggersPerTrain();
    const int nTrains = nTriggers / nTriggersPerTrain;
    const int nRest   = nTriggers % nTriggersPerTrain;
    int nTrigsThisTrain = 0;

    cout << "TMultiDeviceOperator::GoNoiseOccScan() - nTriggers: " << nTriggers << endl;
    cout << "TMultiDeviceOperator::GoNoiseOccScan() - nTriggersPerTrain: " << nTriggersPerTrain << endl;
    cout << "TMultiDeviceOperator::GoNoiseOccScan() - nTrains: " << nTrains << endl;
    cout << "TMultiDeviceOperator::GoNoiseOccScan() - nRest: " << nRest << endl;


    for ( int itrain = 0; itrain <= nTrains; itrain ++ ) { 

        cout << "TMultiDeviceOperator::GoNoiseOccScan() - trains: " << itrain << endl;   
        if ( itrain == nTrains ) {
            nTrigsThisTrain = nRest;
        } else {
            nTrigsThisTrain = nTriggersPerTrain;
        }
        // Send triggers for all boards
        DoTrigger( nTrigsThisTrain );

        // Read data for all boards, multithread (one per board)
        ReadEventData( nTrigsThisTrain );

    } // end of loop on trigger trains
}

//___________________________________________________________________
void TMultiDeviceOperator::DoConfigureMaskStage( int nPix, const int iStage )
{
    for ( unsigned int d = 0;  d < fDevices.size(); d++ ) {
        try {
            shared_ptr<TDevice> myDevice = fDevices.at(d);
            if ( !myDevice ) {
                cerr << "TMultiDeviceOperator::DoConfigureMaskStage() - device " << d  << endl;
                throw runtime_error( "TMultiDeviceOperator::DoConfigureMaskStage() - device is a null ptr" );
            }
            for (unsigned int i = 0; i < myDevice->GetNChips(); i ++) {
                myDevice->GetChip(i)->ConfigureMaskStage( nPix, iStage );
            }
        } catch ( exception& msg ) {
            cerr << msg.what() << endl;
            exit( EXIT_FAILURE );
        }
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::DoTrigger( const int nTriggers )
{
    for ( unsigned int d = 0;  d < fDevices.size(); d++ ) {
        try {
            shared_ptr<TDevice> myDevice = fDevices.at(d);
            if ( !myDevice ) {
                cerr << "TMultiDeviceOperator::DoTrigger() - device " << d  << endl;
                throw runtime_error( "TMultiDeviceOperator::DoTrigger() - device is a null ptr" );
            }
            for ( unsigned int ib = 0; ib < myDevice->GetNBoards(false); ib++ ) {
                // Send triggers for all boards
                // this will be effective on the Master MOSAIC board only
                (myDevice->GetBoard( ib ))->Trigger(nTriggers);
            }
        } catch ( exception& msg ) {
                cerr << msg.what() << endl;
                exit( EXIT_FAILURE );
        }
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::ReadEventDataByDevice( const unsigned int id, const int nTriggers )
{
    try {
        shared_ptr<TDeviceHitScan> myDeviceOperator = fDeviceOperators.at(id);
        myDeviceOperator->ReadEventData(0, nTriggers);
    } catch ( exception& msg ) {
        cerr << msg.what() << endl;
        exit( EXIT_FAILURE );
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::ReadEventData(  const int nTriggers )
{
    vector<thread> datathread;
    for ( unsigned int id = 0; id < fDeviceOperators.size(); id++ ) {
        datathread.push_back( thread( &TMultiDeviceOperator::ReadEventDataByDevice, this, id, nTriggers ) );
    }
    for ( auto &t : datathread ) {
        t.join();
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::TerminateDigitalScan()
{
    for ( unsigned int id = 0; id < fDeviceOperators.size(); id++ ) {
        shared_ptr<TDeviceDigitalScan> myDeviceTestor = dynamic_pointer_cast<TDeviceDigitalScan>(fDeviceOperators.at(id));
        myDeviceTestor->Terminate();
        const bool Recreate = true;
        myDeviceTestor->WriteDataToFile( Recreate );
        myDeviceTestor->WriteCorruptedHitsToFile( Recreate );
        myDeviceTestor->DrawAndSaveToFile();
    }
}

//___________________________________________________________________
void TMultiDeviceOperator::TerminateNoiseOccScan()
{
    for ( unsigned int id = 0; id < fDeviceOperators.size(); id++ ) {
        shared_ptr<TDeviceOccupancyScan> myDeviceTestor = dynamic_pointer_cast<TDeviceOccupancyScan>(fDeviceOperators.at(id));
        myDeviceTestor->Terminate();
        const bool Recreate = true;
        myDeviceTestor->WriteDataToFile( Recreate );
        myDeviceTestor->DrawAndSaveToFile();
    }
}
