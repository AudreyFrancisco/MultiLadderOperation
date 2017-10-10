#include "TAlpideDecoder.h"
#include "AlpideDictionary.h"
#include "TBoardDecoder.h"
#include "TChipConfig.h"
#include "Common.h"
#include "TDevice.h"
#include "TDeviceChipVisitor.h"
#include "TDeviceThresholdScan.h"
#include "TErrorCounter.h"
#include "THisto.h"
#include "TReadoutBoard.h"
#include "TScanConfig.h"
#include "TSCurveAnalysis.h"
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <string.h>

using namespace std;

//___________________________________________________________________
TDeviceThresholdScan::TDeviceThresholdScan() :
TDeviceMaskScan(),
fChargeStart( 0 ),
fChargeStep( 0 ),
fChargeStop( 0 ),
fNChargeSteps( 0 )
{
    
}

//___________________________________________________________________
TDeviceThresholdScan::TDeviceThresholdScan( shared_ptr<TDevice> aDevice,
                                            shared_ptr<TScanConfig> aScanConfig ) :
TDeviceMaskScan( aDevice, aScanConfig ),
fChargeStart( 0 ),
fChargeStep( 0 ),
fChargeStop( 0 ),
fNChargeSteps( 0 )
{
    fChipDecoder = make_unique<TAlpideDecoder>( aDevice, fErrorCounter );
}

//___________________________________________________________________
TDeviceThresholdScan::~TDeviceThresholdScan()
{
    fHistoQue.clear();
    fAnalyserCollection.clear();
}

//___________________________________________________________________
void TDeviceThresholdScan::Init()
{
    try {
        TDeviceMaskScan::Init();
    } catch ( std::exception &err ) {
        cerr << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    fScanHisto = fHistoQue.front();
    if ( !fScanHisto ) {
        throw runtime_error( "TDeviceThresholdScan::Init() - can not use a null pointer for the map of scan histo !" );
    }
    fChipDecoder->SetScanHisto( fScanHisto );
    fErrorCounter->Init( fScanHisto, fNTriggers );
    
    for ( unsigned int ichip = 0; ichip < fDevice->GetNWorkingChips(); ichip++ ) {
        common::TChipIndex aChipIndex = fDevice->GetWorkingChipIndex( ichip );
        AddChipSCurveAnalyzer( aChipIndex );
    }
}

//___________________________________________________________________
void TDeviceThresholdScan::SetVerboseLevel( const int level )
{
    for ( std::map<int, shared_ptr<TSCurveAnalysis>>::iterator it = fAnalyserCollection.begin(); it != fAnalyserCollection.end(); ++it ) {
        ((*it).second)->SetVerboseLevel( level );
    }
    TDeviceMaskScan::SetVerboseLevel( level );
}


//___________________________________________________________________
void TDeviceThresholdScan::Go()
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceThresholdScan::Go() - not initialized ! Please use Init() first." );
    }
    
    if ( fNMaskStages < 0 ) {
        fNMaskStages = 1;
    }
    
    unsigned int nHitsPerStage = 0, nHitsLastStage = 0;

    for ( int istage = 0; istage < fNMaskStages; istage ++ ) { //---------------- loop on pixels
        
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceThresholdScan::Go() - Mask stage "
            << std::dec << istage << endl;
        }
        DoConfigureMaskStage( fNPixPerRegion, istage );
        unsigned int deltaV = fChargeStart;

        if ( istage ) {
            nHitsLastStage = fChipDecoder->GetNHits();
        }

        for ( unsigned int iampl = 0; iampl < fNChargeSteps; iampl++ ) { //-- loop on VPulse low
            
            if ( deltaV >= fChargeStop ) { break; }
            // use current charge
            DoConfigureVPulseLow( deltaV );
            // Send triggers for all boards
            for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
                (fDevice->GetBoard( ib ))->Trigger(fNTriggers);
            }
            try {
                fScanHisto = fHistoQue.at( iampl );
            } catch ( std::exception &err ) {
                cerr << "TDeviceThresholdScan::Go() - Error, stage "
                     << std::dec << istage << " , iampl " << iampl
                     << " , " << err.what() << endl;
                exit( EXIT_FAILURE );
            }
            fChipDecoder->SetScanHisto( fScanHisto );
            // Read data for all boards
            for ( unsigned int ib = 0; ib < fDevice->GetNBoards(false); ib++ ) {
                ReadEventData( ib );
            }
            // next charge
            deltaV += fChargeStep;
            usleep(1000);
        } //---------------------------------------------------------- end of loop on VPulse low
        
        nHitsPerStage = fChipDecoder->GetNHits() - nHitsLastStage;
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceThresholdScan::Go() - stage "
            << std::dec << istage << " , found n hits = " << nHitsPerStage << endl;
        }
        usleep(200);
    } // end of loop on pixels
}

//___________________________________________________________________
unsigned int TDeviceThresholdScan::GetHits( const common::TChipIndex aChipIndex,
                                      const unsigned int icol,
                                      const unsigned int iaddr,
                                      const unsigned int iampl )
{
    if ( icol > common::MAX_DCOL ) {
        throw domain_error( "TDeviceThresholdScan::GetHits() - bad double-colum id" );
    }
    if ( iaddr > common::MAX_ADDR ) {
        throw domain_error( "TDeviceThresholdScan::GetHits() - bad address id" );
    }
    if ( iampl >= fNChargeSteps ) {
        throw domain_error( "TDeviceThresholdScan::GetHits() - bad amplification step" );
    }
    double hits = 0;
    try {
        fScanHisto = fHistoQue.at( iampl );
    } catch ( std::exception &err ) {
        cerr << "TDeviceThresholdScan::GetHits() - Error, icol "
        << std::dec << icol << " , iaddr " << iaddr
        << " , iampl " << iampl << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    hits = (*fScanHisto)(aChipIndex,icol,iaddr);
    if ( hits < 0 ) {
        hits = 0;
    }
    return (unsigned int)hits;
}

//___________________________________________________________________
unsigned int TDeviceThresholdScan::GetInjectedCharge( const unsigned int iampl ) const
{
    if ( iampl >= fNChargeSteps ) {
        throw domain_error( "TDeviceThresholdScan::GetInjectedCharge() - bad amplification step" );
    }
    unsigned int icharge;
    if ( iampl == fNChargeSteps - 1 ) {
        icharge = fChargeStop;
    } else {
        icharge = fChargeStart + (iampl*fChargeStep);
        if ( icharge > fChargeStop ) {
            icharge = fChargeStop;
        }
    }
    return icharge;
}

//___________________________________________________________________
void TDeviceThresholdScan::Terminate()
{
    TDeviceChipVisitor::Terminate();
    AnalyzeData();
}

//___________________________________________________________________
void TDeviceThresholdScan::DrawAndSaveToFile( const char *fName )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceThresholdScan::DrawAndSaveToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceThresholdScan::DrawAndSaveToFile() - not terminated ! Please use Terminate() first." );
    }
    for ( std::map<int, shared_ptr<TSCurveAnalysis>>::iterator it = fAnalyserCollection.begin(); it != fAnalyserCollection.end(); ++it ) {
        ((*it).second)->DrawDistributions();
        ((*it).second)->SaveToFile( fName );
    }
}

//___________________________________________________________________
void TDeviceThresholdScan::WriteDataToFile( const char *fName, bool Recreate )
{
    if ( !fIsInitDone ) {
        throw runtime_error( "TDeviceThresholdScan::WriteDataToFile() - not initialized ! Please use Init() first." );
    }
    if ( !fIsTerminated ) {
        throw runtime_error( "TDeviceThresholdScan::WriteDataToFile() - not terminated ! Please use Terminate() first." );
    }
    if ( !fHistoQue.size() ) {
        throw runtime_error( "TDeviceThresholdScan::WriteDataToFile() - histo deque is empty !" );
    }
    
    char  fNameChip[100];
    FILE *fp;
    
    char fNameTemp[100];
    sprintf( fNameTemp,"%s", fName);
    strtok( fNameTemp, "." );
    string suffix( fNameTemp );
    
    for ( unsigned int ichip = 0; ichip < fDevice->GetNWorkingChips(); ichip++ ) {
        
        common::TChipIndex aChipIndex = fDevice->GetWorkingChipIndex( ichip );
        
        if ( !HasData( aChipIndex ) ) {
            if ( GetVerboseLevel() > kSILENT ) {
                cout << "TDeviceThresholdScan::WriteDataToFile() - Chip ID = "
                << aChipIndex.chipId ;
                if ( aChipIndex.ladderId ) {
                    cout << " , Ladder ID = " << aChipIndex.ladderId;
                }
                cout << " : no data, skipped." <<  endl;
            }
            continue;  // write files only for chips with data
        }
        string filename = common::GetFileName( aChipIndex, suffix );
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceThresholdScan::WriteDataToFile() - Chip ID = "<< aChipIndex.chipId ;
            if ( aChipIndex.ladderId ) {
                cout << " , Ladder ID = " << aChipIndex.ladderId;
            }
            cout << endl;
        }
        strcpy( fNameChip, filename.c_str());
        if ( GetVerboseLevel() > kSILENT ) {
            cout << "TDeviceThresholdScan::WriteDataToFile() - Writing data to file "<< fNameChip << endl;
        }
        if ( Recreate ) fp = fopen(fNameChip, "w");
        else            fp = fopen(fNameChip, "a");
        if ( !fp ) {
            throw runtime_error( "TDeviceThresholdScan::WriteDataToFile() - output file not found." );
        }
        for ( unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++ ) {
            for ( unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++ ) {
                double hits_at_max_charge = GetHits( aChipIndex, icol, iaddr, fNChargeSteps - 1 );
                for ( unsigned int iampl = 0; iampl < fNChargeSteps; iampl ++ ) {
                    double hits = GetHits( aChipIndex, icol, iaddr, iampl );
                    // also write zero hit for pixels who are responding at max injected charge
                    if ( (hits_at_max_charge > 0) || (hits > 0) ) {
                        fprintf(fp, "%d %d %d %d\n",
                                icol, iaddr, GetInjectedCharge(iampl), (int)hits);
                    }
                }
            }
        }
        if (fp) fclose (fp);
    }
}

//___________________________________________________________________
void TDeviceThresholdScan::AddHisto()
{
    unsigned int currentCharge = fChargeStart;
    
    while ( currentCharge < fChargeStop ) {
        
        auto aScanHisto = make_shared<TScanHisto>();
        common::TChipIndex id;
        // histo for a given value of the injected charge
        THisto histo ("ThrScanHisto", "ThrScanHisto",
                      common::MAX_DCOL+1, 0, common::MAX_DCOL,
                      common::MAX_ADDR+1, 0, common::MAX_ADDR);
        
        for ( unsigned int ichip = 0; ichip < fDevice->GetNChips(); ichip++ ) {
            if ( fDevice->GetChipConfig(ichip)->IsEnabled() ) {
                id.boardIndex   = fDevice->GetBoardIndexByChip(ichip);
                id.dataReceiver = fDevice->GetChipConfig(ichip)->GetParamValue("RECEIVER");
                id.ladderId     = fDevice->GetLadderId();
                id.chipId       = fDevice->GetChipId(ichip);
                aScanHisto->AddHisto( id, histo );
            }
        }
        aScanHisto->FindChipList();
        if ( GetVerboseLevel() > kCHATTY ) {
            cout << endl << "TDeviceThresholdScan::AddHisto() - generated map with " << std::dec << aScanHisto->GetSize() << " elements" << endl;
        }
        fHistoQue.push_back( move(aScanHisto) );
        currentCharge += fChargeStep;
    }
    fNChargeSteps = fHistoQue.size();
    if ( GetVerboseLevel() > kSILENT ) {
        cout << endl << "TDeviceThresholdScan::AddHisto() - generated deque with " << std::dec << fNChargeSteps << " elements" << endl;
    }
}

//___________________________________________________________________
void TDeviceThresholdScan::DumpScanParameters()
{
    cout << "TDeviceThresholdScan::DumpScanParameters() - injected charge :" << endl;
    cout << "\t start   " << fChargeStart << endl;
    cout << "\t stop    " << fChargeStop << endl;
    cout << "\t step    " << fChargeStep << endl;
}

//___________________________________________________________________
void TDeviceThresholdScan::InitScanParameters()
{
    TDeviceMaskScan::InitScanParameters();
    fChargeStart = (unsigned int)fScanConfig->GetChargeStart();
    fChargeStep  = fScanConfig->GetChargeStep();
    fChargeStop  = (unsigned int)fScanConfig->GetChargeStop();

    // in case the input values were in a bad ordering
    if ( fChargeStart > fChargeStop ) {
        int buffer = fChargeStart;
        fChargeStart = fChargeStop;
        fChargeStop = buffer;
    }
    if ( fChargeStep < 0 ) {
        fChargeStep *= -1.;
    }
    if ( GetVerboseLevel() > kSILENT ) {
        DumpScanParameters();
    }
}

//___________________________________________________________________
bool TDeviceThresholdScan::HasData( const common::TChipIndex idx )
{
    unsigned int iampl = fNChargeSteps - 1;
    try {
        fScanHisto = fHistoQue.at( iampl );
    } catch ( std::exception &err ) {
        cerr << "TDeviceThresholdScan::HasData() - Error, iampl "
        << std::dec << iampl << err.what() << endl;
        exit( EXIT_FAILURE );
    }
    if ( !fScanHisto->IsValidChipIndex( idx ) ) {
        return false;
    }
    return fScanHisto->HasData( idx );
    return false;
}

//___________________________________________________________________
void TDeviceThresholdScan::AddChipSCurveAnalyzer( const common::TChipIndex idx )
{
    int int_index = common::GetMapIntIndex( idx );
    
    auto analyzer = make_shared<TSCurveAnalysis>( idx, fNTriggers, fChargeStop );
    analyzer->Init();
    fAnalyserCollection.insert( std::pair<int, shared_ptr<TSCurveAnalysis>>(int_index, analyzer) );
}

//___________________________________________________________________
void TDeviceThresholdScan::AnalyzeData()
{
    for ( unsigned int ichip = 0; ichip < fDevice->GetNWorkingChips(); ichip++ ) {
        
        common::TChipIndex chipIndex = fDevice->GetWorkingChipIndex( ichip );
        
        for ( unsigned int icol = 0; icol <= common::MAX_DCOL; icol ++ ) {
            for ( unsigned int iaddr = 0; iaddr <= common::MAX_ADDR; iaddr ++ ) {
         
                AnalyzePixelSCurve( chipIndex, icol, iaddr );
                
            }
        }
        
    }
}



//___________________________________________________________________
void TDeviceThresholdScan::AnalyzePixelSCurve( const common::TChipIndex aChipIndex,
                                              const unsigned int icol,
                                              const unsigned int iaddr )
{
    int int_index = common::GetMapIntIndex( aChipIndex );
    (fAnalyserCollection.at(int_index))->SetPixelCoordinates( icol, iaddr );

    if ( !GetHits( aChipIndex, icol, iaddr, fNChargeSteps-1 ) ) {
        // skip pixels that have no data (presumably not pulsed)
        return;
    }
    
    for ( unsigned int iampl = 0; iampl < fNChargeSteps; iampl++ ) {
        unsigned int icharge = GetInjectedCharge( iampl );
        unsigned int nhits = GetHits( aChipIndex, icol, iaddr, iampl );
        (fAnalyserCollection.at(int_index))->FillPixelData( iampl, icharge, nhits );
    }
    
    (fAnalyserCollection.at(int_index))->ProcessPixelData();
}

