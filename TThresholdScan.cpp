#include <unistd.h>
#include "TThresholdScan.h"
#include "THisto.h"
#include "TAlpide.h"
#include "TDevice.h"
#include "TChipConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardDAQ.h"
#include "TBoardConfig.h"

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
    shared_ptr<TScanConfig> currentConfig = fConfig.lock();
    fStart[0]  = currentConfig->GetChargeStart();
    fStop [0]  = currentConfig->GetChargeStop ();
    fStep [0]  = currentConfig->GetChargeStep ();
    
    fStart[1]  = 0;
    fStep [1]  = 1;
    fStop [1]  = currentConfig->GetNMaskStages();
    
    fStart[2]  = 0;
    fStep [2]  = 1;
    fStop [2]  = 1;
    
    fVPULSEH   = 170;
    fNTriggers = currentConfig->GetParamValue("NINJ");
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
    chip->BaseConfig();
    chip->ConfigureFromu();
    chip->ConfigureCMU();
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
        (currentDevice->GetBoard( i ))->SendOpCode( Alpide::OPCODE_GRST );
        (currentDevice->GetBoard( i ))->SendOpCode( Alpide::OPCODE_PRST );
    }
    
    for ( int i = 0; i < currentDevice->GetNChips(); i++ ) {
        if (! ((currentDevice->GetChipConfig( i ))->IsEnabled())) continue;
        ConfigureChip(i);
    }
    
    for ( int i = 0; i < currentDevice->GetNBoards(false); i++ ) {
        (currentDevice->GetBoard( i ))->SendOpCode( Alpide::OPCODE_RORST );
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
                (currentDevice->GetChip( ichip ))->WriteRegister( Alpide::REG_VPULSEL, fVPULSEH - fValue[0] );
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
    std::vector<TPixHit> *Hits = new std::vector<TPixHit>;
    
    shared_ptr<TDevice> currentDevice = fDevice.lock();

    for ( int iboard = 0; iboard < currentDevice->GetNBoards(false); iboard++ ) {
        (currentDevice->GetBoard( iboard ))->Trigger( fNTriggers );
    }
    
    for ( int iboard = 0; iboard < currentDevice->GetNBoards(false); iboard++ ) {
        int itrg = 0;
        int trials = 0;
        while ( itrg < fNTriggers * fEnabled[iboard] ) {
            if ((currentDevice->GetBoard( iboard ))->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
                usleep(100);
                trials++;
                if (trials == 3) {
                    cout << "Board " << iboard << ": reached 3 timeouts, giving up on this event" << endl;
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
                if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits)) {
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
        cout << "Found " << Hits->size() << " hits" << endl;
        FillHistos( Hits, iboard );
    }
}

//___________________________________________________________________
void TThresholdScan::FillHistos( std::vector<TPixHit> *Hits, const int iboard )
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

    for (int i = 0; i < (int)Hits->size(); i++) {
        if (Hits->at(i).address / 2 != fRow) continue;  // todo: keep track of spurious hits, i.e. hits in non-injected rows
        // !! This will not work when allowing several chips with the same Id
        idx.dataReceiver = (currentDevice->GetBoard( iboard ))->GetReceiver(Hits->at(i).chipId);
        idx.chipId       = Hits->at(i).chipId;
        int col = Hits->at(i).region * 32 + Hits->at(i).dcol * 2;
        int leftRight = ((((Hits->at(i).address % 4) == 1) || ((Hits->at(i).address % 4) == 2))? 1:0);
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

