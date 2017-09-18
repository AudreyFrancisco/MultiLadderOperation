#include "TScan.h"
#include "TScanConfig.h"
#include "THisto.h"
#include "TAlpide.h"
#include "TDevice.h"
#include "TChipConfig.h"
#include <iostream>

using namespace std;

//___________________________________________________________________
TScan::TScan()
{
    for ( int i = 0; i < MAXLOOPLEVEL; i++ ) {
        fStart[i] = 0;
        fStop[i] = 0;
        fStep[i] = 0;
        fValue[i] = 0;
    }
}

//___________________________________________________________________
TScan::TScan( shared_ptr<TScanConfig> aConfig,
              shared_ptr<TDevice> aDevice,
             std::deque<TScanHisto> *histoQue ) :
    fScanConfig( aConfig ),
    fDevice( aDevice ),
    fHistoQue( histoQue )
{
    for ( int i = 0; i < MAXLOOPLEVEL; i++ ) {
        fStart[i] = 0;
        fStop[i] = 0;
        fStep[i] = 0;
        fValue[i] = 0;
    }
}

//___________________________________________________________________
bool TScan::Loop( const int loopIndex )
{
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
void TScan::CreateScanHisto()
{
    common::TChipIndex id;
    fHisto = make_unique<TScanHisto>();
    
    THisto histo = CreateHisto();
    
    shared_ptr<TDevice> currentDevice = fDevice.lock();
    
    for ( unsigned int iboard = 0; iboard < currentDevice->GetNBoards(false); iboard ++ ) {
        for ( unsigned int ichip = 0; ichip < currentDevice->GetNChips(); ichip ++ ) {
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
    shared_ptr<TScanConfig> currentScanConfig = fScanConfig.lock();
    fPixPerStage = currentScanConfig->GetParamValue("PIXPERREGION");
}

//___________________________________________________________________
void TMaskScan::ConfigureMaskStage( const int ichip, const int istage)
{
    shared_ptr<TDevice> currentDevice = fDevice.lock();
    fRow = (currentDevice->GetChip( ichip ))->ConfigureMaskStage( fPixPerStage, istage );
}


