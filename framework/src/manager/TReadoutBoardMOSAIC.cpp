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
#include "mexception.h"
#include "pexception.h"
#include "mservice.h"
#include <stdexcept>


using namespace std;
std::vector<unsigned char> fDebugBuffer;

/* System PLL register setup for:
		Primary Input Frequency: 200
		Secondary Input Frequency: 40
		Version 2
		C1: 18.0849p
		R2: 1.003k
		C2: 9.9706n
		R3: 60
		C3: 377.5p
		Charge Pump: 2.5m
		Input MUX set to Primary
		All inputs/Outputs LVDS 3.3V/2.5V
*/

I2CSysPll::pllRegisters_t TReadoutBoardMOSAIC::sysPLLregContent(new uint16_t[22]{ 
	/* Register 0: */ 0x02A9,
	/* Register 1: */ 0x0000,
	/* Register 2: */ 0x000E,
	/* Register 3: */ 0x08F5,
	/* Register 4: */ 0x346F,	// set to 0x346f to set reference clock from secondary input (0x246f for primary)
	/* Register 5: */ 0x0023,
	/* Register 6: */ 0x0002,
	/* Register 7: */ 0x0023,
	/* Register 8: */ 0x0002,
	/* Register 9: */ 0x0003,
	/* Register 10: */ 0x0020,
	/* Register 11: */ 0x0000,
	/* Register 12: */ 0x0003,  // 0x0003, 0x2003 bypass Y5 from primary input
	/* Register 13: */ 0x0020,
	/* Register 14: */ 0x0000,
	/* Register 15: */ 0x0003,
	/* Register 16: */ 0x0020,
	/* Register 17: */ 0x0000,
	/* Register 18: */ 0x0003,
	/* Register 19: */ 0x0020,
	/* Register 20: */ 0x0000,
	/* Register 21: */ 0x0006			// RO register
});

#pragma mark - constructor/destructor

//___________________________________________________________________
TReadoutBoardMOSAIC::TReadoutBoardMOSAIC() :
    fBoardConfig( weak_ptr<TBoardConfigMOSAIC>() ),
    fI2cBus( nullptr ),
    fPulser( nullptr ),
    fDummyReceiver( nullptr ),
    fTrgRecorder( nullptr ),
    fTrgDataParser( nullptr ),
    fCoordinator( nullptr ),
    fTheVersionId(""),
    fTheVersionMaj( 0 ),
    fTheVersionMin( 0 ),
    fClockOuputsEnabled( false )
{ }

//___________________________________________________________________
TReadoutBoardMOSAIC::TReadoutBoardMOSAIC( shared_ptr<TBoardConfigMOSAIC> boardConfig ) :
    fBoardConfig( boardConfig ),
    fI2cBus( nullptr ),
    fPulser( nullptr ),
    fDummyReceiver( nullptr ),
    fTrgRecorder( nullptr ),
    fTrgDataParser( nullptr ),
    fCoordinator( nullptr ),
    fTheVersionId(""),
    fTheVersionMaj( 0 ),
    fTheVersionMin( 0 ),
    fClockOuputsEnabled( false )
{
    init();
}

//___________________________________________________________________
TReadoutBoardMOSAIC::~TReadoutBoardMOSAIC()
{
    delete fDummyReceiver;
    delete fTrgDataParser;

    for (int i=0; i<(int)MosaicBoardConfig::MAX_TRANRECV; i++)
        delete fAlpideDataParser[i];


}

#pragma mark - public methods

// Read/Write registers
// Playing with doExecute, one can queue multiple read/write operations before execute()

//___________________________________________________________________
int TReadoutBoardMOSAIC::WriteChipRegister (uint16_t address, uint16_t value, uint8_t chipId, const bool doExecute )
{
    if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::WriteChipRegister() - clock outputs disabled" );
    }
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
    if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::ReadChipRegister() - clock outputs disabled" );
    }
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
    if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::SendOpCode() - clock outputs disabled" );
    }
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
    if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::SendOpCode() - clock outputs disabled" );
    }
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    uint8_t ShortOpCode = (uint8_t)OpCode;
    try {
        for(int Cii = 0; Cii < spBoardConfig->GetCtrlInterfaceNum(); Cii++){
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

    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    if ( spBoardConfig->IsMasterSlaveModeOn() ) {
        // only Master can send trigger in Master/Slave board config
        // if mode is Alone, the board can also send trigger
        if ((GetCoordinatorMode() == MCoordinator::Master) || (GetCoordinatorMode() == MCoordinator::Alone)) {
            fPulser->setConfig(triggerDelay, pulseDelay, pulseMode);
        }
    } else {
        // standalone board
        fPulser->setConfig(triggerDelay, pulseDelay, pulseMode);
    }

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
uint32_t TReadoutBoardMOSAIC::GetTriggerCount()
{
    uint32_t counter = 0xDEADBEEF;
    mTriggerControl->getTriggerCounter(&counter);
    return counter;
}

//___________________________________________________________________
int  TReadoutBoardMOSAIC::Trigger (int nTriggers)
{
     if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::Trigger() - clock outputs disabled" );
    }
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    if ( spBoardConfig->IsMasterSlaveModeOn() ) { // Master/Slave exists in the firmware
        if ( GetCoordinatorMode() == MCoordinator::Master ) { // Boards synchronization
            fCoordinator->addSync(); // prepend this command to the next "pulser->run(nPulses)" 
                                     // to have a fixed delay from sync to first trigger sent by Master
                                     // it can be replaced by "fCoordinator->sync()"
        }
        // only Master can send trigger in Master/Slave board config
        // if mode is Alone, the board can also send trigger
        if ((GetCoordinatorMode() == MCoordinator::Master) || (GetCoordinatorMode() == MCoordinator::Alone)) {
            fPulser->run(nTriggers);
        }
    } else {
        // standalone board
        fPulser->run(nTriggers);
    }

    return(nTriggers);
}

//___________________________________________________________________
int  TReadoutBoardMOSAIC::ReadEventData (int &nBytes, unsigned char *buffer)
{
    TAlpideDataParser *dr;
    long readDataSize;
    
    // check for data in the receivers buffer
    for (int i = 0; i < (int)MosaicBoardConfig::MAX_TRANRECV; i++){
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
            exit( EXIT_FAILURE );
        }
        
        // get event data from the selected data receiver
        if (dr->hasData())
            return (dr->ReadEventData(nBytes, buffer));
    }
    return -1;
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::StartRun()
{
    if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::StartRun() - clock outputs disabled" );
    }
    enableDefinedReceivers();
    connectTCP(); // open TCP connection
    mRunControl->startRun(); // start run
    usleep(5000);
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::StopRun()
{
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    if ( spBoardConfig->IsMasterSlaveModeOn() ) {
        // only Master can send trigger in Master/Slave board config
        // if mode is Alone, the board can also send trigger
        if ((GetCoordinatorMode() == MCoordinator::Master) || (GetCoordinatorMode() == MCoordinator::Alone)) { 
            fPulser->run(0);
        }
    } else {
        // standalone board
        fPulser->run(0);
    }
    mRunControl->stopRun();
    closeTCP();  // FIXME: this could cause the lost of the tail of the buffer ...
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::EnableControlInterfaces(const bool en)
{
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    for (int Cii = 0; Cii < spBoardConfig->GetCtrlInterfaceNum(); Cii++) {
        fControlInterface[Cii]->addEnable(en);
        fControlInterface[Cii]->addDisableME(spBoardConfig->GetManchesterDisable() == 1 ? true : false);
        fControlInterface[Cii]->execute();
    }
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::EnableControlInterface(const unsigned int interface, const bool en)
{
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    try {
        if (interface < spBoardConfig->GetCtrlInterfaceNum()) {
            fControlInterface[interface]->addEnable(en);
            fControlInterface[interface]->addDisableME(spBoardConfig->GetManchesterDisable() == 1 ? true : false);
            fControlInterface[interface]->execute();
        } else {
            cerr << "TReadoutBoardMOSAIC::enableControlInterface() - index = " << interface << endl;
            throw out_of_range("TBoardConfigMOSAIC::enableControlInterface() - index out of range!");
        } 
    } catch ( std::out_of_range &err ) {
        exit(EXIT_FAILURE);
    }
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::EnableClockOutputs(const bool en)
{
    // just a wrapper
    if ( GetVerboseLevel() ) {
        cout << "TReadoutBoardMOSAIC::EnableClockOutputs() - ";
        if ( en ) cout << "true" << endl;
        else cout << "false" << endl;
    }
    EnableControlInterfaces(en);
    fClockOuputsEnabled = en;
    return;
} 

//___________________________________________________________________
void TReadoutBoardMOSAIC::EnableClockOutput(const unsigned int interface, const bool en)
{
    // just a wrapper
    EnableControlInterface(interface, en);
    return;
} 

//___________________________________________________________________
void TReadoutBoardMOSAIC::SetVerboseLevel( const int level )
{
    for (int i=0; i<(int)MosaicBoardConfig::MAX_TRANRECV; i++){
        fAlpideDataParser[i]->SetVerboseLevel( level );
    }
    fTrgDataParser->SetVerboseLevel( level );
    fPulser->SetVerboseLevel( level );
    TVerbosity::SetVerboseLevel( level );
}

//___________________________________________________________________
string TReadoutBoardMOSAIC::GetRegisterDump()
{
    string result;
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    result += "TReadoutBoardMOSAIC::GetRegisterDump() - IP Address: ";
    result += spBoardConfig->GetIPaddress();
    result += '\n';
    result += "Pulser\n";
    result += fPulser->dumpRegisters();
    result += "mRunControl\n";
    result += mRunControl->dumpRegisters();
    result += "mRunTriggerControl\n";
    result += mTriggerControl->dumpRegisters();
    result += "mTrgRecorder\n";
    result += fTrgRecorder->dumpRegisters();
    if ( spBoardConfig->IsMasterSlaveModeOn() ) {
        result += "coordinator\n";
        result += fCoordinator->dumpRegisters();
    }
    return result;
}

//___________________________________________________________________
MCoordinator::mode_t TReadoutBoardMOSAIC::GetCoordinatorMode() const 
{
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    if ( spBoardConfig->IsMasterSlaveModeOn() ) {
	    return fCoordinator->getMode();
    }
    return MCoordinator::Alone;
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::SendBroadcastReset()
{
    if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::SendBroadcastReset() - clock outputs disabled" );
    }
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
	for (int i = 0; i < spBoardConfig->GetCtrlInterfaceNum(); i++){
		fControlInterface[i]->addSendCmd((uint8_t)AlpideOpCode::GRST);
		fControlInterface[i]->execute();
	}
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::SendBroadcastROReset()
{
     if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::SendBroadcastROReset() - clock outputs disabled" );
    }
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
	for (int i = 0; i < spBoardConfig->GetCtrlInterfaceNum(); i++){
		fControlInterface[i]->addSendCmd((uint8_t)AlpideOpCode::RORST);
		fControlInterface[i]->execute();
	}
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::SendBroadcastBCReset()
{
    if ( !ClockOutputsEnabled() ) {
        throw runtime_error( "TReadoutBoardMOSAIC::SendBroadcastBCReset() - clock outputs disabled" );
    }
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
	for (int i = 0; i < spBoardConfig->GetCtrlInterfaceNum(); i++){
		fControlInterface[i]->addSendCmd((uint8_t)AlpideOpCode::BCRST);
		fControlInterface[i]->execute();
	}
}



#pragma mark - private methods

// Private : Init the board
//___________________________________________________________________
void TReadoutBoardMOSAIC::init()
{
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    setIPaddress(spBoardConfig->GetIPaddress(), spBoardConfig->GetTCPport());
    
    cout << "TReadoutBoardMOSAIC::init() - MOSAIC firmware version: " << getFirmwareVersion() << endl;
    
    // I2C master (WBB slave) and connected peripherals
    fI2cBus = make_unique<I2Cbus>(mIPbus, WbbBaseAddress::add_i2cMaster);
    
    // CMU Control interface
    fControlInterface[0] = make_unique<ControlInterface>(mIPbus, WbbBaseAddress::add_controlInterface);
    fControlInterface[1] = make_unique<ControlInterface>(mIPbus, WbbBaseAddress::add_controlInterfaceB);
    int addDisp = 0;
    for (int i = 2; i < spBoardConfig->GetCtrlInterfaceNum(); i++) {
        fControlInterface[i] = make_unique<ControlInterface>(mIPbus, WbbBaseAddress::add_controlInterface_0 + (addDisp << 24));
        addDisp++;
    }

    // Pulser
    fPulser = make_unique<Pulser>(mIPbus, WbbBaseAddress::pulser);
    
    // ALPIDE Hi Speed data receiver
    for (int i=0; i<(int)MosaicBoardConfig::MAX_TRANRECV; i++){  
        fAlpideRcv[i] = make_unique<ALPIDErcv>(mIPbus, WbbBaseAddress::add_alpideRcv+(i<<24));
        fAlpideRcv[i]->addEnable(false);
        fAlpideRcv[i]->addInvertInput(false);
        fAlpideRcv[i]->execute();
    }

    // Trigger recorder
    fTrgRecorder = make_unique<TrgRecorder>(mIPbus, WbbBaseAddress::add_trgRecorder);
    fTrgRecorder->addEnable( spBoardConfig->IsTrgRecorderEnable() );

    // The data consumer for hardware generators
    fDummyReceiver = new DummyReceiver();
    addDataReceiver(0, fDummyReceiver); // ID 0
    
    for (int i=0; i<(int)MosaicBoardConfig::MAX_TRANRECV; i++){
        fAlpideDataParser[i] = new TAlpideDataParser();
        fAlpideDataParser[i]->SetId(i);
        fAlpideDataParser[i]->SetVerboseLevel( this->GetVerboseLevel() );
        addDataReceiver(i+1, fAlpideDataParser[i]); // ID 1-10
    }

    // Trigger data recorder
    fTrgDataParser = new TrgRecorderParser();
    fTrgDataParser->SetVerboseLevel( this->GetVerboseLevel() );
    addDataReceiver(11, fTrgDataParser); // ID 11;
    
    if ( spBoardConfig->IsMasterSlaveModeOn() ) {
        try {
            // Master/Slave coordinator
            fCoordinator = make_unique<MCoordinator>(mIPbus, WbbBaseAddress::add_coordinator);
            switch ( (int)spBoardConfig->GetMasterSlaveMode() ) {
                case 0 : fCoordinator->setMode(MCoordinator::Alone);  break;
                case 1 : fCoordinator->setMode(MCoordinator::Master); break;
                case 2 : fCoordinator->setMode(MCoordinator::Slave);  break;
                default : fCoordinator->setMode(MCoordinator::Alone); break;
            }
        } catch (...) {
            throw runtime_error( "TReadoutBoardMOSAIC::init() - Could not communicate with the Master/Slave coordinator, please upgrade your firmware or disable MASTERSLAVEMODEON setting for the MOSAIC board!");
        }
    } else {
        // clock is always sent in old firmware
        fClockOuputsEnabled = true;
    }

#ifdef ENABLE_EXTERNAL_CLOCK
    // Enable external clock input
    mRunControl->setConfigReg(CFG_EXTCLOCK_SEL_BIT); // can be OR of ALPIDEboard::configBits_e
    printf("Enabling external clock\n");
#else
    mRunControl->setConfigReg(0);
#endif

    // ----- Now do the initilization -------
    // Initialize the System PLL
    mSysPLL->setup(sysPLLregContent);

#ifdef ENABLE_EXTERNAL_CLOCK
    uint32_t boardStatusReady = (BOARD_STATUS_FEPLL_LOCK);
#else
    // Disable this code if using an external clock != 40 MHz
    // wait for board to be ready
    uint32_t boardStatusReady = ( (int)MosaicStatusBits::BOARD_STATUS_GTP_RESET_DONE 
                                    | (int)MosaicStatusBits::BOARD_STATUS_GTPLL_LOCK 
                                    | (int)MosaicStatusBits::BOARD_STATUS_EXTPLL_LOCK 
                                    | (int)MosaicStatusBits::BOARD_STATUS_FEPLL_LOCK 
                                );
#endif
    
    // wait 1s for transceivers reset done
    long int init_try;
    for (init_try = 1000; init_try > 0; init_try--){
        uint32_t st;
        usleep(1000);
        mRunControl->getStatus(&st);
        if ((st & boardStatusReady) == boardStatusReady) break;
    }
    if (init_try == 0)
        throw MBoardInitError("TReadoutBoardMOSAIC::init() - Timeout setting MOSAIC system PLL");
    
    for(int i = 0; i < (int)MosaicBoardConfig::MAX_CTRLINT; i++)
        setPhase(spBoardConfig->GetCtrlInterfacePhase(),i);  // set the Phase shift on the line
    
    setSpeedMode (spBoardConfig->GetSpeedMode());
    setInverted  (spBoardConfig->IsInverted(),-1);
    
    fPulser->run(0);
    mRunControl->stopRun();
    mRunControl->clearErrors();
    mRunControl->setAFThreshold(spBoardConfig->GetCtrlAFThreshold());
    mRunControl->setLatency(spBoardConfig->GetCtrlLatMode(), spBoardConfig->GetCtrlLatMode());
    EnableControlInterfaces(true);
    // mRunControl->setConfigReg(0); // // 0: internal 40 MHz clock (can be OR of bits: CFG_CLOCK_SEL_BIT,  CFG_CLOCK_20MHZ_BIT)
    
    return;
}

//___________________________________________________________________
std::string TReadoutBoardMOSAIC::getFirmwareVersion()
{
    char *theIPAddr;
    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    theIPAddr = spBoardConfig->GetIPaddress();
    
    MService::fw_info_t MOSAICinfo;
    MService *endPoint = new MService();
    endPoint->setIPaddress( theIPAddr );
    endPoint->readFWinfo(&MOSAICinfo);
    
    fTheVersionMaj = MOSAICinfo.ver_maj;
    fTheVersionMin = MOSAICinfo.ver_min;

    std::string word( MOSAICinfo.fw_identity );
    fTheVersionId = word;
    return fTheVersionId;
}

// ============================== DATA receivers private methods =======================================

//___________________________________________________________________
void TReadoutBoardMOSAIC::enableDefinedReceivers()
{
    bool Used[(int)MosaicBoardConfig::MAX_TRANRECV]; 
    for (int i = 0; i < (int)MosaicBoardConfig::MAX_TRANRECV; i++) { 
        Used[i] = false;
    }
    
    for( int i=0; i < (int)fChipPositions.size(); i++ ) { //for each defined chip
        shared_ptr<TChipConfig> spChipConfig = (fChipPositions.at(i)).lock();
        int dataLink = spChipConfig->GetReceiver();
        if(dataLink >= 0) { // Enable the data receiver
            if ( spChipConfig->IsEnabled() && !Used[dataLink] ) {
                cout << "TReadoutBoardMOSAIC::enableDefinedReceivers() - ENabling receiver " << dataLink << endl;
                fAlpideRcv[dataLink]->addEnable(true);
                Used[dataLink] = true;
                //fAlpideRcv[dataLink]->execute();
            }
            else if (!Used[dataLink]){
                cout << "TReadoutBoardMOSAIC::enableDefinedReceivers() - DISabling receiver " << dataLink << endl;
                fAlpideRcv[dataLink]->addEnable(false);
            }
        }
    }
    return;
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::setPhase(const int APhase, const int ACii)
{

    shared_ptr<TBoardConfigMOSAIC> spBoardConfig = fBoardConfig.lock();
    try {
        if (ACii < spBoardConfig->GetCtrlInterfaceNum()) {
	        fControlInterface[ACii]->setPhase(APhase);
	        fControlInterface[ACii]->addSendCmd((uint8_t)MosaicOpCode::OPCODE_GRST);
	        fControlInterface[ACii]->execute();
        } else {
            cerr << "TReadoutBoardMOSAIC::setPhase() - index = " << ACii << endl;
            throw out_of_range("TBoardConfigMOSAIC::setPhase() - index out of range!");
        } 
    } catch ( std::out_of_range &err ) {
        exit(EXIT_FAILURE);
    }
	return;
};

//___________________________________________________________________
void TReadoutBoardMOSAIC::setSpeedMode(MosaicReceiverSpeed ASpeed)
{
    int regSet = 0;
    cout << "TReadoutBoardMOSAIC::setSpeedMode() - " ;

    switch (ASpeed) {
        case MosaicReceiverSpeed::RCV_RATE_400:
            regSet = (int)MosaicConfigBits::CFG_RATE_400;
            cout << "400 Mb/s " << endl;
            break;

        case MosaicReceiverSpeed::RCV_RATE_600:
            regSet = (int)MosaicConfigBits::CFG_RATE_600;
            cout << "600 Mb/s " << endl;
            break;

        case MosaicReceiverSpeed::RCV_RATE_1200:
            regSet = (int)MosaicConfigBits::CFG_RATE_1200;
            cout << "1.2 Gb/s " << endl;
            break;
  }
  mRunControl->rmwConfigReg(~(uint32_t)MosaicConfigBits::CFG_RATE_MASK, regSet);
}

//___________________________________________________________________
void TReadoutBoardMOSAIC::setInverted(bool AInverted, int Aindex)
{
    int st,en;
    Aindex = -1;
    st = (Aindex != -1) ? Aindex : 0;
    en = (Aindex != -1) ? Aindex+1 : (int)MosaicBoardConfig::MAX_TRANRECV; 
    for(int i = st; i < en; i++) {
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

