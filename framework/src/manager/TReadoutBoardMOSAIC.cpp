/*
 * Copyright (C) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * ====================================================
 *     __  __   __  _____  __   __
 *    / / /  | / / / ___/ /  | / / SEZIONE di BARI
 *   / / / | |/ / / /_   / | |/ /
 *  / / / /| / / / __/  / /| / /
 * /_/ /_/ |__/ /_/    /_/ |__/
 *
 * ====================================================
 * Written by Antonio Franco  <Anotnio.Franco@ba.infn.it>
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>
 *
 *
 *
 *  		HISTORY
 *  3/8/16	- Add the Board decoder class ...
 *  5/8/16  - adapt the read event to new definition
 *  18/01/17 - Review of ReadEventData. Added inheritance from class MBoard
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include "TChipConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include "TBoardConfig.h"
#include "TAlpide.h"
#include "mexception.h"
#include "pexception.h"

using namespace std;
std::vector<unsigned char> fDebugBuffer;

#pragma mark - constructor/destructor

//___________________________________________________________________
TReadoutBoardMOSAIC::TReadoutBoardMOSAIC() :
fBoardConfig( weak_ptr<TBoardConfigMOSAIC>() ),
fDataGenerator( nullptr ),
fI2cBus( nullptr ),
fPulser( nullptr ),
fDummyReceiver( nullptr )
{ }

//___________________________________________________________________
TReadoutBoardMOSAIC::TReadoutBoardMOSAIC( shared_ptr<TBoardConfigMOSAIC> boardConfig ) :
    fBoardConfig( boardConfig ),
    fDataGenerator( nullptr ),
    fI2cBus( nullptr ),
    fPulser( nullptr ),
    fDummyReceiver( nullptr )
{
    init();
}

//___________________________________________________________________
TReadoutBoardMOSAIC::~TReadoutBoardMOSAIC()
{
    delete fDummyReceiver;
    fPulser.reset();
    for (int i=0; i<BoardConfigMOSAIC::MAX_MOSAICTRANRECV; i++)
        delete fAlpideDataParser[i];
    
    for(int i=0; i<BoardConfigMOSAIC::MAX_MOSAICCTRLINT; i++)
        fControlInterface[i].reset();
    
    fI2cBus.reset();
    fDataGenerator.reset();
}

#pragma mark - public methods

// Read/Write registers
// Playing with doExecute, one can queue multiple read/write operations before execute()

//___________________________________________________________________
int TReadoutBoardMOSAIC::WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId, const bool doExecute )
{
    uint_fast16_t Cii = GetControlInterface(chipId);
    try {
        fControlInterface[Cii]->addWriteReg(chipId, address, value);
        if ( doExecute ) fControlInterface[Cii]->execute();
    } catch ( exception &err ) {
        cerr << err.what() << endl;
        throw err;
    }
    return(0);
}

//___________________________________________________________________
int TReadoutBoardMOSAIC::ReadChipRegister (uint16_t address, uint16_t &value, uint8_t chipId, const bool doExecute )
{
    uint_fast16_t Cii = GetControlInterface(chipId);
    try {
        fControlInterface[Cii]->addReadReg( chipId,  address,  &value);
        if ( doExecute ) fControlInterface[Cii]->execute();
    } catch ( exception &err ) {
        cerr << err.what() << endl;
        throw err;
    }
    return(0);
}

//___________________________________________________________________
int TReadoutBoardMOSAIC::SendOpCode (uint16_t  OpCode, uint8_t chipId)
{
    uint_fast16_t Cii = GetControlInterface(chipId);
    try {
        fControlInterface[Cii]->addWriteReg(chipId, (uint16_t)AlpideRegister::COMMAND, OpCode);
        fControlInterface[Cii]->execute();
    } catch ( exception &err ) {
        cerr << err.what() << endl;
        throw err;
    }
    return(0);
}

//___________________________________________________________________
int TReadoutBoardMOSAIC::SendOpCode (uint16_t  OpCode)
{
    uint8_t ShortOpCode = (uint8_t)OpCode;
    try {
        for(int Cii=0;Cii<BoardConfigMOSAIC::MAX_MOSAICCTRLINT;Cii++){
            fControlInterface[Cii]->addSendCmd(ShortOpCode);
            fControlInterface[Cii]->execute();
        }
    } catch ( exception &err ) {
        cerr << err.what() << endl;
        throw err;
    }
    return(0);
}

//___________________________________________________________________
int TReadoutBoardMOSAIC::SetTriggerConfig (bool enablePulse, bool enableTrigger, int triggerDelay, int pulseDelay)
{
    uint16_t pulseMode = 0;
    
    if(enablePulse)
        pulseMode |= Pulser::OPMODE_ENPLS_BIT;
    if(enableTrigger)
        pulseMode |= Pulser::OPMODE_ENTRG_BIT;
    fPulser->setConfig(triggerDelay, pulseDelay, pulseMode);
    return(pulseMode);
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::SetTriggerSource (TTriggerSource triggerSource)
{
    if(triggerSource == TTriggerSource::kTRIG_INT) {
        // Internal Trigger
        mTriggerControl->addEnableExtTrigger(false, 0);
    } else {
        // external trigger
        mTriggerControl->addEnableExtTrigger(true, 0);
    }
    mTriggerControl->execute();
}

//___________________________________________________________________
int  TReadoutBoardMOSAIC::Trigger (int nTriggers)
{
    fPulser->run(nTriggers);
    return(nTriggers);
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::StartRun()
{
    enableDefinedReceivers();
    connectTCP(); // open TCP connection
    mRunControl->startRun(); // start run
    usleep(5000);
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::StopRun()
{
    fPulser->run(0);
    mRunControl->stopRun();
    closeTCP();  // FIXME: this could cause the lost of the tail of the buffer ...
}

//___________________________________________________________________
int  TReadoutBoardMOSAIC::ReadEventData (int &nBytes, unsigned char *buffer)
{
    TAlpideDataParser *dr;
    long readDataSize;
    
    // check for data in the receivers buffer
    for (int i=0; i<BoardConfigMOSAIC::MAX_MOSAICTRANRECV; i++){
        if (fAlpideDataParser[i]->hasData())
            return (fAlpideDataParser[i]->ReadEventData(nBytes, buffer));
    }
    
    // try to read from TCP connection
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    for (;;){
        try {
            readDataSize = pollTCP(spBoardConfig->GetPollingDataTimeout(), (MDataReceiver **) &dr);
            if (readDataSize == 0)
                return -1;
        } catch (exception& e) {
            cerr << e.what() << endl;
            StopRun();
            decodeError();
            exit(1);
        }
        
        // get event data from the selected data receiver
        if (dr->hasData())
            return (dr->ReadEventData(nBytes, buffer));
    }
    return -1;
}

#pragma mark - private methods

// Private : Init the board
//___________________________________________________________________
void TReadoutBoardMOSAIC::init()
{
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    setIPaddress(spBoardConfig->GetIPaddress(), spBoardConfig->GetTCPport());
    
    // Data Generator
    fDataGenerator = make_shared<MDataGenerator>( mIPbus, WbbBaseAddress::dataGenerator );
    
    // I2C master (WBB slave) and connected peripherals
    fI2cBus = make_shared<I2Cbus>(mIPbus, WbbBaseAddress::i2cMaster);
    
    // System PLL on I2C bus
    mSysPLL = new I2CSysPll(mIPbus, WbbBaseAddress::i2cSysPLL);
    
    // CMU Control interface
    fControlInterface[0] = make_shared<ControlInterface>(mIPbus, WbbBaseAddress::controlInterface);
    fControlInterface[1] = make_shared<ControlInterface>(mIPbus, WbbBaseAddress::controlInterfaceB);
    
    // Pulser
    fPulser = make_shared<Pulser>(mIPbus, WbbBaseAddress::pulser);
    
    // ALPIDE Hi Speed data receiver
    for (int i=0; i<BoardConfigMOSAIC::MAX_MOSAICTRANRECV; i++){
        fAlpideRcv[i] = make_shared<ALPIDErcv>(mIPbus, WbbBaseAddress::alpideRcv+(i<<24));
        fAlpideRcv[i]->addEnable(false);
        fAlpideRcv[i]->addInvertInput(false);
        fAlpideRcv[i]->execute();
    }
    
    // The data consumer for hardware generators
    fDummyReceiver = new DummyReceiver();
    addDataReceiver(0, fDummyReceiver);
    
    for (int i=0; i<BoardConfigMOSAIC::MAX_MOSAICTRANRECV; i++){
        fAlpideDataParser[i] = new TAlpideDataParser();
        fAlpideDataParser[i]->SetId(i);
        addDataReceiver(i+1, fAlpideDataParser[i]);
    }
    
    // ----- Now do the initilization -------
    // Initialize the System PLL
    mSysPLL->setup();
    
    // wait for board to be ready
    uint32_t boardStatusReady = (BOARD_STATUS_GTP_RESET_DONE | \
                                 BOARD_STATUS_GTPLL_LOCK | \
                                 BOARD_STATUS_EXTPLL_LOCK | \
                                 BOARD_STATUS_FEPLL_LOCK );
    
    // wait 1s for transceivers reset done
    long int init_try;
    for (init_try=1000; init_try>0; init_try--){
        uint32_t st;
        usleep(1000);
        mRunControl->getStatus(&st);
        if ((st & boardStatusReady) == boardStatusReady)
            break;
    }
    if (init_try==0)
        throw MBoardInitError("Timeout setting MOSAIC system PLL");
    
    for(int i=0;i<BoardConfigMOSAIC::MAX_MOSAICCTRLINT;i++)
        setPhase(spBoardConfig->GetCtrlInterfacePhase(),i);  // set the Phase shift on the line
    
    setSpeedMode (spBoardConfig->GetSpeedMode());
    setInverted  (spBoardConfig->IsInverted(),-1);
    
    fPulser->run(0);
    mRunControl->stopRun();
    mRunControl->clearErrors();
    mRunControl->setAFThreshold(spBoardConfig->GetCtrlAFThreshold());
    mRunControl->setLatency(spBoardConfig->GetCtrlLatMode(), spBoardConfig->GetCtrlLatMode());
    mRunControl->setConfigReg(0); // // 0: internal 40 MHz clock (can be OR of bits: CFG_CLOCK_SEL_BIT,  CFG_CLOCK_20MHZ_BIT)
    
    return;
}

// ============================== DATA receivers private methods =======================================

//___________________________________________________________________
void TReadoutBoardMOSAIC::enableDefinedReceivers()
{
    bool Used[BoardConfigMOSAIC::MAX_MOSAICTRANRECV];
    for (int i = 0; i < BoardConfigMOSAIC::MAX_MOSAICTRANRECV; i++) {
        Used[i] = false;
    }
    
    for( int i=0; i < (int)fChipPositions.size(); i++ ) { //for each defined chip
        shared_ptr<TChipConfig> spChipConfig = (fChipPositions.at(i)).lock();
        int dataLink = spChipConfig->GetReceiver();
        if(dataLink >= 0) { // Enable the data receiver
            if ( spChipConfig->IsEnabled() ) {
                std::cout << "TReadoutBoardMOSAIC::enableDefinedReceivers() - ENabling receiver " << dataLink << std::endl;
                fAlpideRcv[dataLink]->addEnable(true);
                Used[dataLink] = true;
                //fAlpideRcv[dataLink]->execute();
            }
            else if (!Used[dataLink]){
                std::cout << "TReadoutBoardMOSAIC::enableDefinedReceivers() - DISabling receiver " << dataLink << std::endl;
                fAlpideRcv[dataLink]->addEnable(false);
            }
        }
    }
    return;
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::setSpeedMode(Mosaic::TReceiverSpeed ASpeed)
{
    mRunControl->setSpeed (ASpeed);
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::setInverted(bool AInverted, int Aindex)
{
    int st,en;
    Aindex = -1;
    st = (Aindex != -1) ? Aindex : 0;
    en = (Aindex != -1) ? Aindex+1 : BoardConfigMOSAIC::MAX_MOSAICTRANRECV;
    for(int i=st;i<en;i++) {
        fAlpideRcv[i]->addInvertInput(AInverted);
        fAlpideRcv[i]->execute();
    }
    return;
}


// Decode the Mosaic Error register
//___________________________________________________________________
uint32_t TReadoutBoardMOSAIC::decodeError()
{
    uint32_t runErrors;
    mRunControl->getErrors(&runErrors);
    if (runErrors){
        std::cout << "TReadoutBoardMOSAIC::decodeError() - Error register: 0x" << std::hex << runErrors << std::dec << " ";
        if (runErrors & (1<<0)) std::cout << "Board memory overflow, ";
        if (runErrors & (1<<1)) std::cout << "Board detected TCP/IP connection closed while running, ";
        for (int i=0; i<10; i++)
            if (runErrors & (1<<(8+i)))
                std::cout << " Alpide data receiver " << i << " detected electric idle condition, ";
        std::cout << std::endl;
    }
    return(runErrors);
}

