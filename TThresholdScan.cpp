#include <unistd.h>
#include "TThresholdScan.h"
#include "THisto.h"
#include "AlpideDictionary.h"
#include "TAlpide.h"
#include "TDevice.h"
#include "TChipConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardDAQ.h"
#include "TBoardConfig.h"
#include "AlpideDecoder.h"
#include "TScanConfig.h"


using namespace std;

//___________________________________________________________________
TThresholdScan::TThresholdScan() : TMaskScan()
{
    fVPULSEH = 170;
    fNTriggers = 50;
}

//___________________________________________________________________
TThresholdScan::TThresholdScan( shared_ptr<TScanConfig> config,
                               shared_ptr<TDevice> aDevice,
                               deque<TScanHisto> *histoQue )
: TMaskScan( config, aDevice, histoQue )
{
    shared_ptr<TScanConfig> currentScanConfig = fConfig.lock();
    fStart[0]  = currentScanConfig->GetChargeStart();
    fStop [0]  = currentScanConfig->GetChargeStop ();
    fStep [0]  = currentScanConfig->GetChargeStep ();
    
    fStart[1]  = 0;
    fStep [1]  = 1;
    fStop [1]  = currentScanConfig->GetNMaskStages();
    
    fStart[2]  = 0;
    fStep [2]  = 1;
    fStop [2]  = 1;
    
    fVPULSEH   = 170;
    fNTriggers = currentScanConfig->GetParamValue("NINJ");
    CreateScanHisto();
}

//___________________________________________________________________
void TThresholdScan::ConfigureBoard( const int iboard )
{
    shared_ptr<TDevice> currentDevice = fDevice.lock();
    shared_ptr<TReadoutBoard> board = currentDevice->GetBoard( iboard );
    shared_ptr<TBoardConfig> boardConfig = currentDevice->GetBoardConfig( iboard );
    
    if ( boardConfig->GetBoardType() == TBoardType::kBOARD_MOSAIC ) {
        board->SetTriggerConfig( true, true,
                                 boardConfig->GetParamValue("STROBEDELAYBOARD"),
                                 boardConfig->GetParamValue("PULSEDELAY") );
        board->SetTriggerSource( TTriggerSource::kTRIG_INT );
    }
    else if ( boardConfig->GetBoardType() == TBoardType::kBOARD_DAQ ) {
        // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
        // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
        board->SetTriggerConfig( true, false,
                                 0,
                                 2 * boardConfig->GetParamValue("STROBEDELAYBOARD") );
        board->SetTriggerSource( TTriggerSource::kTRIG_EXT );
    }
}

//___________________________________________________________________
void TThresholdScan::ConfigureChip( const int ichip )
{
    shared_ptr<TDevice> currentDevice = fDevice.lock();
    shared_ptr<TAlpide> chip = currentDevice->GetChip( ichip );
    // the user should use a config file with settings relevant for threshold scan
    chip->BaseConfig();
}

//___________________________________________________________________
std::shared_ptr<THisto> TThresholdScan::CreateHisto()
{
    std::shared_ptr<THisto> histo = std::make_shared<THisto>("ThresholdHisto", "ThresholdHisto", 1024, 0, 1023, (fStop[0] - fStart[0]) / fStep[0], fStart[0], fStop[0]);
    return histo;
}

//___________________________________________________________________
void TThresholdScan::Init()
{
    CountEnabledChips();
    shared_ptr<TDevice> currentDevice = fDevice.lock();

    for ( int i = 0; i < currentDevice->GetNBoards(false); i++ ) {
        cout << "Board " << i << ", found " << fEnabled[i] << " enabled chips" << endl;
        ConfigureBoard(i);
        (currentDevice->GetBoard( i ))->SendOpCode( (uint16_t)AlpideOpCode::GRST );
        (currentDevice->GetBoard( i ))->SendOpCode( (uint16_t)AlpideOpCode::PRST );
    }
    
    for ( int i = 0; i < currentDevice->GetNChips(); i++ ) {
        if (! ((currentDevice->GetChipConfig( i ))->IsEnabled())) continue;
        ConfigureChip(i);
    }
    
    for ( int i = 0; i < currentDevice->GetNBoards(false); i++ ) {
        (currentDevice->GetBoard( i ))->SendOpCode( (uint16_t)AlpideOpCode::RORST );
        shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(currentDevice->GetBoard( i ));
        if ( myMOSAIC ) {
            myMOSAIC->StartRun();
        }
    }
}

//___________________________________________________________________
void TThresholdScan::PrepareStep( const int loopIndex )
{
    shared_ptr<TDevice> currentDevice = fDevice.lock();
    
    switch ( loopIndex ) {
        case 0:    // innermost loop: change VPULSEL
            for ( int ichip = 0; ichip < currentDevice->GetNChips(); ichip++ ) {
                if (! ((currentDevice->GetChipConfig( ichip ))->IsEnabled()) ) continue;
                fVPULSEH = (currentDevice->GetChipConfig( ichip ))->GetParamValue("VPULSEH"); // Replace default value with the one from config file
                (currentDevice->GetChip( ichip ))->WriteRegister( AlpideRegister::VPULSEL, fVPULSEH - fValue[0] ); // Automatically matches max pulse = VPULSEH in config
            }
            break;
        case 1:    // 2nd loop: mask staging
            for ( int ichip = 0; ichip < currentDevice->GetNChips(); ichip++ ) {
                if (! ((currentDevice->GetChipConfig( ichip ))->IsEnabled()) ) continue;
                ConfigureMaskStage( ichip, fValue[1]);
            }
            break;
        default:
            break;
    }
}

//___________________________________________________________________
void TThresholdScan::Execute()
{
    unsigned char         buffer[1024*4000];
    int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
    int                   nBad = 0, skipped = 0;
    TBoardHeader          boardInfo;
    
    shared_ptr<TDevice> currentDevice = fDevice.lock();

    for ( int iboard = 0; iboard < currentDevice->GetNBoards(false); iboard++ ) {
        (currentDevice->GetBoard( iboard ))->Trigger( fNTriggers );
    }
    
    for ( int iboard = 0; iboard < currentDevice->GetNBoards(false); iboard++ ) {
        int itrg = 0;
        int trials = 0;
        while ( itrg < fNTriggers * fEnabled[iboard] ) {
            if ((currentDevice->GetBoard( iboard ))->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
                usleep(100); // Increment from 100us
                trials++;
                if (trials == 10) {
                    cout << "Board " << iboard << ": reached 10 timeouts, giving up on this event" << endl;
                    itrg = fNTriggers * fEnabled[iboard];
                    skipped++;
                    trials = 0;
                }
                continue;
            }
            else {
                BoardDecoder::DecodeEvent( (currentDevice->GetBoardConfig( iboard ))->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo );
                // decode Chip event
                int n_bytes_chipevent=n_bytes_data-n_bytes_header;//-n_bytes_trailer;
                if (boardInfo.eoeCount < 2) n_bytes_chipevent -= n_bytes_trailer;
                if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, fHits)) {
                    cout << "Found bad event, length = " << n_bytes_chipevent << endl;
                    nBad ++;
                    if (nBad > 10) continue;
                    FILE *fDebug = fopen ("DebugData.dat", "a");
                    fprintf(fDebug, "Bad event:\n");
                    for (int iByte=0; iByte<n_bytes_data + 1; ++iByte) {
                        fprintf (fDebug, "%02x ", (int) buffer[iByte]);
                    }
                    fprintf(fDebug, "\nFull Event:\n");
                    for (int ibyte = 0; ibyte < (int)fDebugBuffer.size(); ibyte ++) {
                        fprintf (fDebug, "%02x ", (int) fDebugBuffer.at(ibyte));
                    }
                    fprintf(fDebug, "\n\n");
                    fclose (fDebug);
                }
                itrg++;
            }
        }
        cout << "Found " << fHits.size() << " hits" << endl;
        FillHistos( iboard );
        fHits.clear();
    }
}

//___________________________________________________________________
void TThresholdScan::FillHistos( const int iboard )
{
    TChipIndex idx;
    idx.boardIndex = iboard;
    /*
     int chipId;
     int region;
     int dcol;
     int address;
     */
    
    shared_ptr<TDevice> currentDevice = fDevice.lock();

    for (int i = 0; i < (int)fHits.size(); i++) {
        if ((fHits.at(i))->GetAddress() / 2 != fRow) continue;  // todo: keep track of spurious hits, i.e. hits in non-injected rows
        // !! This will not work when allowing several chips with the same Id
        idx.dataReceiver = (currentDevice->GetBoard( iboard ))->GetReceiver((fHits.at(i))->GetChipId());
        idx.chipId       = (fHits.at(i))->GetChipId();
        int col = (fHits.at(i))->GetRegion() * 32 + (fHits.at(i))->GetDoubleColumn() * 2;
        int leftRight = (((((fHits.at(i))->GetAddress() % 4) == 1) || (((fHits.at(i))->GetAddress() % 4) == 2))? 1:0);
        col += leftRight;
        
        fHisto->Incr( idx, col, fValue[0] );
    }
}

//___________________________________________________________________
void TThresholdScan::LoopEnd( const int loopIndex )
{
    if ( loopIndex == 0 ) {
        TScanHisto myHisto = TScanHisto( *fHisto );
        fHistoQue->push_back( myHisto );
        fHisto->Clear();
    }
}

//___________________________________________________________________
void TThresholdScan::Terminate()
{
    shared_ptr<TDevice> currentDevice = fDevice.lock();

    // write Data;
    for ( int iboard = 0; iboard < currentDevice->GetNBoards(false); iboard ++ ) {
        shared_ptr<TReadoutBoardMOSAIC> myMOSAIC = dynamic_pointer_cast<TReadoutBoardMOSAIC>(currentDevice->GetBoard( iboard ));
        if (myMOSAIC) {
            myMOSAIC->StopRun();
        }
        shared_ptr<TReadoutBoardDAQ> myDAQBoard = dynamic_pointer_cast<TReadoutBoardDAQ>(currentDevice->GetBoard( iboard ));
        if (myDAQBoard) {
            myDAQBoard->PowerOff();
        }
    }
}

