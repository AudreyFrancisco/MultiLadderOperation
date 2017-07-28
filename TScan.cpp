#include "TScan.h"
#include "TScanConfig.h"
#include "THisto.h"
#include "TAlpide.h"
#include "TDevice.h"
#include "TChipConfig.h"
#include <iostream>

using namespace std;
bool fScanAbort;

//___________________________________________________________________
TScan::TScan()
{
    fScanAbort = false;
    for ( int i = 0; i < MAXLOOPLEVEL; i++ ) {
        fStart[i] = 0;
        fStop[i] = 0;
        fStep[i] = 0;
        fValue[i] = 0;
    }
    for ( int i = 0; i < MAXBOARDS; i++ ) {
        fEnabled[i] = 0;
    }
}

//___________________________________________________________________
TScan::TScan( shared_ptr<TScanConfig> aConfig,
              shared_ptr<TDevice> aDevice,
             std::deque<TScanHisto> *histoQue ) :
    fConfig( aConfig ),
    fDevice( aDevice ),
    fHistoQue( histoQue )
{
    fScanAbort = false;
    for ( int i = 0; i < MAXLOOPLEVEL; i++ ) {
        fStart[i] = 0;
        fStop[i] = 0;
        fStep[i] = 0;
        fValue[i] = 0;
    }
    for ( int i = 0; i < MAXBOARDS; i++ ) {
        fEnabled[i] = 0;
    }
}

//___________________________________________________________________
bool TScan::Loop( const int loopIndex )
{
    if ( fScanAbort )
        return false;  // check for abort flag first
    if ( (fStep[loopIndex] > 0) && (fValue[loopIndex] < fStop[loopIndex]) )
        return true;  // limit check for positive steps
    if ( (fStep[loopIndex] < 0) && (fValue[loopIndex] > fStop[loopIndex]) )
        return true;  // same for negative steps
    
    return false;
}

//___________________________________________________________________
void TScan::Next( const int loopIndex )
{
    fValue[loopIndex] += fStep[loopIndex];
}

//___________________________________________________________________
void TScan::CountEnabledChips()
{
    shared_ptr<TDevice> currentDevice = fDevice.lock();

    //std::cout << "in count enabled chips, boards_size = " << fBoards.size() << ", chips_size = " << fChips.size() << std::endl;
    for (int i = 0; i < MAXBOARDS; i++) {
        fEnabled[i] = 0;
    }
    for ( int iboard = 0; iboard < currentDevice->GetNBoards(false); iboard ++ ) {
        for ( int ichip = 0; ichip < currentDevice->GetNChips(); ichip ++ ) {
            shared_ptr<TReadoutBoard> board = currentDevice->GetBoardByChip( ichip );
            if ( ((currentDevice->GetChipConfig( ichip ))->IsEnabled())
                && (  board == currentDevice->GetBoard(iboard)) ) {
                fEnabled[iboard] ++;
            }
        }
    }
}

//___________________________________________________________________
void TScan::CreateScanHisto()
{
    TChipIndex id;
    fHisto = make_unique<TScanHisto>();
    
    shared_ptr<THisto> histo = CreateHisto();
    
    shared_ptr<TDevice> currentDevice = fDevice.lock();
    
    for ( int iboard = 0; iboard < currentDevice->GetNBoards(false); iboard ++ ) {
        for ( int ichip = 0; ichip < currentDevice->GetNChips(); ichip ++ ) {
            shared_ptr<TReadoutBoard> board = currentDevice->GetBoardByChip( ichip );
            if ( ((currentDevice->GetChipConfig( ichip ))->IsEnabled())
                && (  board == currentDevice->GetBoard(iboard)) ) {
                id.boardIndex       = iboard;
                id.dataReceiver     = (currentDevice->GetChipConfig( ichip ))->GetParamValue("RECEIVER");
                id.chipId           = (currentDevice->GetChipConfig( ichip ))->GetChipId();
                fHisto->AddHisto( id, histo );
            }
        }
    }
    cout << "TScan::CreateHisto() - generated map with " << fHisto->GetSize() << " elements" << endl;
}

//___________________________________________________________________
TMaskScan::TMaskScan() : TScan()
{ }

//___________________________________________________________________
TMaskScan::TMaskScan( shared_ptr<TScanConfig> aConfig,
                     shared_ptr<TDevice> aDevice,
                     deque<TScanHisto> *histoQue )
: TScan( aConfig, aDevice, histoQue )
{
    shared_ptr<TScanConfig> currentScanConfig = fConfig.lock();
    fPixPerStage = currentScanConfig->GetParamValue("PIXPERREGION");
}

//___________________________________________________________________
void TMaskScan::ConfigureMaskStage( const int ichip, const int istage)
{
    shared_ptr<TDevice> currentDevice = fDevice.lock();
    fRow = (currentDevice->GetChip( ichip ))->ConfigureMaskStage( fPixPerStage, istage );
}


