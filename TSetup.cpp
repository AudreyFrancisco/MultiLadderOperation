#include <iostream>
#include <ifstream>
#include "TSetup.h"
#include "USB.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TBoardConfigDAQ.h"
#include "TBoardConfigMOSAIC.h"
#include "TChipConfig.h"
#include "TAlpide.h"
#include <string.h>
#include <exception>
#include <stdexcept>

#define

using namespace std;

const string TSetup::NEWALPIDEVERSION = "1.0_mft";

#pragma mark - Constructors/destructor

//___________________________________________________________________
TSetup::TSetup() :
    fInitialisedSetup( false ),
    fCreatedConfig( false ),
    fVerboseLevel( 0 ),
    fConfigFileName( "Config.cfg" ),
    fNWorkingChips( 0 ),
    fNChips( 0 ),
    fNModules( 0 ),
    fStartChipId( 0 ),
    fBoardType( kBOARD_UNKNOWN ),
    fDeviceType( TYPE_UNKNOWN ),
    fScanConfig( nullptr ),
    fConfigFile( nullptr )
{
    cout << "**  ALICE new-alpide-software  v." << NEWALPIDEVERSION
         << " **" << endl<< endl;
}

//___________________________________________________________________
TSetup::~TSetup()
{
    fScanConfig.reset();
    fBoards.clear();
    fChips.clear();
    fBoardConfigs.clear();
    fChipConfigs.clear();
    if ( fConfigFile ) {
        fclose( fConfigFile );
    }
}

#pragma mark - setters
//___________________________________________________________________
void TSetup::SetConfigFileName( const string name )
{
    if ( name.empty() ) {
        cout << "TSetup::SetConfigFileName() - user gave an empty file name! Default config file name will be used." << endl;
    } else {
        try {
            ifstream file( name.c_str() );
            if ( file.good() ) {
                fConfigFileName = name;
                file.close();
            } else {
                throw invalid_argument( "TSetup::SetConfigFileName() - file does not exist! Default config file name will be used." );
            }
        } catch ( std::invalid_argument &err ) {
            cerr << err.what() << endl;
            exit();
        }
    }
}

#pragma mark - getters
//___________________________________________________________________
shared_ptr<TReadoutBoard> TSetup::GetBoard(const int iBoard)
{
    if ( (!fInitialisedSetup) || fBoards.empty() ) {
        throw runtime_error( "TSetup::GetBoard() - no board defined!" );
    }
    if ( (iBoard < 0) || (iBoard >= fBoards.size()) ) {
        cerr << "TSetup::GetBoard() - iBoard = " << iBoard << endl;
        throw out_of_range( "TSetup::GetBoard() - wrong board index!" );
    }
    shared_ptr<TReadoutBoard> myBoard = nullptr;
    try {
        myBoard = fBoards.at( iBoard );
    } catch ( ... ) {
        cerr << "TSetup::GetBoard() - error, exit!" << endl;
        exit();
    }
    return myBoard;
}

//___________________________________________________________________
shared_ptr<TBoardConfig> TSetup::GetBoardConfig(const int iBoard)
{
    if ( (!fCreatedConfig) || fBoardConfigs.empty() ) {
        throw runtime_error( "TSetup::GetBoardConfig() - no config defined!" );
    }
    if ( (iBoard < 0) || (iBoard >= fBoardConfigs.size()) ) {
        cerr << "TSetup::GetBoardConfig() - iBoard = " << iBoard << endl;
        throw out_of_range( "TSetup::GetBoardConfig() - wrong board config index!" );
    }
    shared_ptr<TBoardConfig> myBoardConfig = nullptr;
    try {
        myBoardConfig = fBoardConfigs.at( iBoard );
    } catch ( ... ) {
        cerr << "TSetup::GetBoardConfig() - error, exit!" << endl;
        exit();
    }
    return myBoardConfig;
}

//___________________________________________________________________
shared_ptr<TAlpide> TSetup::GetChip(const int iChip)
{
    if ( (!fInitialisedSetup) || fChips.empty() ) {
        throw runtime_error( "TSetup::GetChip() - no chip defined!" );
    }
    if ( (iChip < 0) || (iChip >= fChips.size()) ) {
        cerr << "TSetup::GetChip() - iChip = " << iChip << endl;
        throw out_of_range( "TSetup::GetChip() - wrong chip index!" );
    }
    shared_ptr<TAlpide> myChip = nullptr;
    try {
        myChip = fChips.at( iChip );
    } catch ( ... ) {
        cerr << "TSetup::GetChip() - error, exit!" << endl;
        exit();
    }
    return myChip;
}

//___________________________________________________________________
shared_ptr<TChipConfig> TSetup::GetChipConfig(const int iChip )
{
    if ( (!fCreatedConfig) || fChipConfigs.empty()  ) {
        throw runtime_error( "TSetup::GetChipConfig() - no config defined!" );
    }
    if ( (iChip < 0) || (iChip >= fChipConfigs.size()) ) {
        cerr << "TSetup::GetChipConfig() - iChip = " << iChip << endl;
        throw out_of_range( "TSetup::GetChipConfig() - wrong chip config index!" );
    }
    shared_ptr<TChipConfig> myChipConfig = nullptr;
    try {
        myChipConfig = fChipConfigs.at( iChip );
    } catch ( ... ) {
        cerr << "TSetup::GetChipConfig() - error, exit!" << endl;
        exit();
    }
    return myChipConfig;
}

//___________________________________________________________________
shared_ptr<TChipConfig> TSetup::GetChipConfigById( const int chipId )
{
    if ( (!fCreatedConfig) || fChipConfigs.empty() ) {
        throw runtime_error( "TSetup::GetChipConfigById() - no config defined!" );
    }
    shared_ptr<TChipConfig> config = nullptr;
    for (int i = 0; i < (int)fChipConfigs.size(); i++) {
        if (fChipConfigs.at(i)->GetChipId() == chipId) {
            config = fChipConfigs.at(i);
            break;
        }
    }
    if ( !config ) {
        cerr << "TSetup::GetChipConfigById() - requested chip id = " << chipId << endl;
        throw runtime_error( "TSetup::GetChipConfigById() - requested chip id not found." );
    }
    return config;
}


//___________________________________________________________________
int TSetup::GetNChips() const
{
    if ( fCreatedConfig ) {
        return (int)fChipConfigs.size();
    } else {
        return fNChips;
    }
}

//___________________________________________________________________
int TSetup::GetStartChipID() const
{
    if ( fCreatedConfig ) {
        return GetChipConfig(0)->GetChipId();
    } else {
        return fStartChipId;
    }
}

//___________________________________________________________________
bool TSetup::IsMFTLadder() const
{
    if ( !fCreatedConfig ) {
        throw runtime_error( "TSetup::IsMFTLadder() - device not created yet." );
    }
    if ( (GetDeviceType() == TYPE_MFT_LADDER5)
        || (GetDeviceType() == TYPE_MFT_LADDER4)
        || (GetDeviceType() == TYPE_MFT_LADDER3)
        || (GetDeviceType() == TYPE_MFT_LADDER2) ) {
        return true;
    } else {
        return false;
    }
}

#pragma mark - other public methods
// Decode line command parameters
//___________________________________________________________________
void TSetup::DecodeCommandParameters(int argc, char **argv)
{
    int c;
    
    while ((c = getopt (argc, argv, "hv:c:")) != -1)
        switch (c) {
            case 'h':  // prints the Help of usage
                cout << "Usage : " << argv[0] << " -h -v <level> -c <configuration_file> "<< endl;
                cout << "-h  :  Display this message" << endl;
                cout << "-v <level> : Sets the verbosity level (partly implemented)" << endl;
                cout << "-c <configuration_file> : Sets the configuration file used" << endl << endl;
                exit();
                break;
            case 'v':  // sets the verbose level
                SetVerboseLevel( atoi(optarg) );
                break;
            case 'c':  // the name of Configuration file
                char ConfigurationFileName[1024];
                strncpy(ConfigurationFileName, optarg, 1023);
                SetConfigFileName( string(ConfigurationFileName) );
                break;
            case '?':
                if (optopt == 'c') {
                    cerr << "Option -" << optopt << " requires an argument." << endl;
                } else {
                    if (isprint (optopt)) {
                        cerr << "Unknown option `-" << optopt << "`" << endl;
                    } else {
                        cerr << "Unknown option character `" << std::hex << optopt << std::dec << "`" << endl;
                    }
                }
                exit();
            default:
                return;
        }
}

//___________________________________________________________________
void TSetup::DumpConfigToFile( string fName )
{
    // FIXME: not implemented yet
    cout << "TSetup::InitSetupTelescopeDAQ() - NOT IMPLEMENTED YET, filename = "
    << fName << endl;
}

//___________________________________________________________________
void TSetup::InitSetup()
{
    if ( fInitialisedSetup || fChips.size() ) {
        throw runtime_error( "TSetup::InitSetup() - init already done." );
    }
    try {
        if ( !fCreatedConfig ) {
            throw runtime_error( "TSetup::InitSetup() - no device created, can not init." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }

    switch ( fDeviceType )
    {
        case TYPE_CHIP_DAQ:
            InitSetupOBSingleDAQ();
            break;
        case TYPE_TELESCOPE:
            InitSetupTelescopeDAQ();
            break;
        case TYPE_OBHIC:
            InitSetupOB();
            break;
        case TYPE_IBHIC:
            InitSetupIB();
            break;
        case TYPE_MFT_LADDER5:
            InitSetupMFTLadder();
            break;
        case TYPE_MFT_LADDER4:
            InitSetupMFTLadder();
            break;
        case TYPE_MFT_LADDER3:
            InitSetupMFTLadder();
            break;
        case TYPE_MFT_LADDER2:
            InitSetupMFTLadder();
            break;
        case TYPE_OBCHIP_MOSAIC:
            InitSetupOBSingleMosaic();
            break;
        case TYPE_IBCHIP_MOSAIC:
            InitSetupIBSingleMosaic();
            break;
        case TYPE_HALFSTAVE:
            InitSetupHalfStave();
            break;
        default:
            throw runtime_error( "TSetup::InitSetup() - unknown device type, doing nothing." );
            return;
    }
}

//___________________________________________________________________
void TSetup::ReadConfigFile()
{
    char        Line[1024], Param[50], Rest[50];
    int         Chip;
    fConfigFile = fopen( fConfigFileName.c_str(), "r" );
    
    try {
        if ( !fConfigFile ) {
            throw runtime_error( "TSetup::ReadConfigFile() - config file not found." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    
    // first look for the type of setup in order to initialise config structure
    while ( (!fCreatedConfig) && (fgets(Line, 1023, fp) != NULL) ) {
        
        if ((Line[0] == '\n') || (Line[0] == '#')) continue;
        ParseLine (Line, Param, Rest, &Chip);
        if (!strcmp(Param,"NCHIPS")){
            sscanf(Rest, "%d", &fNChips);
        }
        if (!strcmp(Param,"NMODULES")){
            sscanf(Rest, "%d", &fNModules);
        }
        if (!strcmp(Param, "DEVICE")) {
            try {
                ReadDeviceType( Rest );
            } catch ( std::runtime_error &err ) {
                cerr << err.what() << endl;
                exit();
            }
        }
        try {
            // type and nchips has been found (nchips only needed for device TYPE_TELESCOPE)
            // calls the appropriate method, which in turn calls
            // the constructors for board and chip configs
            CreateDeviceConfig();
        } catch ( std::runtime_error &err ) {
            cerr << err.what() << endl;
            exit();
        }
    }
    fNChips = fChipConfigs.size();
    fScanConfig = make_shared<TScanConfig>();
    
    // now read the rest
    while (fgets(Line, 1023, fp) != NULL) {
        DecodeLine(Line);
    }
}

#pragma mark - other private methods

// Try to communicate with all chips, disable chips that are not answering
//___________________________________________________________________
void TSetup::CheckControlInterface()
{
    uint16_t WriteValue = 10;
    uint16_t Value;
    if ( fVerboseLevel ) {
        cout << endl
        << "TSetup::CheckControlInterface() - Before starting actual test:"
        << endl
        << "TSetup::CheckControlInterface() - Checking the control interfaces of all chips by doing a single register readback test"
        << endl;
    }
    
    if ( !fChips.size() ) {
        throw runtime_error( "TSetup::CheckControlInterface() - no chip defined!" );
    }
    
    for ( unsigned int i = 0; i < fChips.size(); i++ ) {
        
        if ( !GetChipConfig(i)->IsEnabled() ) continue;
        
        GetChip(i)->WriteRegister( 0x60d, WriteValue );
        try {
            GetChip(i)->ReadRegister( 0x60d, Value );
            if ( WriteValue == Value ) {
                if ( fVerboseLevel ) {
                    cout << "TSetup::CheckControlInterface() -  Chip ID " << GetChipConfig(i)->GetChipId() << ", readback correct." << endl;
                }
                fNWorkingChips++;
            } else {
                cerr << "TSetup::CheckControlInterface() - Chip ID " << GetChipConfig(i)->GetChipId() << ", wrong readback value (" << Value << " instead of " << WriteValue << "), disabling." << endl;
                GetChipConfig(i)->SetEnable(false);
            }
        } catch (std::exception &e) {
            cerr << "TSetup::CheckControlInterface() - Chip ID " << GetChipConfig(i)->GetChipId() << ", not answering, disabling." << endl;
            GetChipConfig(i)->SetEnable(false);
        }
        
    }
    cout << "TSetup::CheckControlInterface() - Found " << GetNWorkingChips() << " working chips." << endl << endl;
    
    if ( GetNWorkingChips() == 0 ) {
        throw runtime_error( "TSetup::CheckControlInterface() - no working chip found!" );
    }
}

//___________________________________________________________________
void TSetup::DecodeLine(const char* Line)
{
    int Chip, Start, ChipStop, BoardStop;
    char Param[128], Rest[896];
    if ((Line[0] == '\n') || (Line[0] == '#')) {   // empty Line or comment
        return;
    }
    
    ParseLine(Line, Param, Rest, &Chip);
    
    if (Chip == -1) {
        Start     = 0;
        ChipStop  = fChipConfigs.size();
        BoardStop = fBoardConfigs.size();
    }
    else {
        Start     = Chip;
        ChipStop  = Chip+1;
        BoardStop = Chip+1;
    }
    
    // Todo: correctly handle the number of readout boards
    // currently only one is written
    // Note: having a config file with parameters for the mosaic board, but a setup with a DAQ board
    // (or vice versa) will issue unknown-parameter warnings...
    if (fChipConfigs.at(0)->IsParameter(Param)) {
        for (int i = Start; i < ChipStop; i++) {
            fChipConfigs.at(i)->SetParamValue (Param, Rest);
        }
    }
    else if (fBoardConfigs.at(0)->IsParameter(Param)) {
        for (int i = Start; i < BoardStop; i++) {
            fBoardConfigs.at(i)->SetParamValue (Param, Rest);
        }
    }
    else if (fScanConfig->IsParameter(Param)) {
        fScanConfig->SetParamValue (Param, Rest);
    }
    else if ((!strcmp(Param, "ADDRESS")) && (fBoardConfigs.at(0)->GetBoardType() == kBOARD_MOSAIC)) {
        for (int i = Start; i < BoardStop; i++) {
            shared_ptr<TBoardConfigMOSAIC> boardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC>(fBoardConfigs.at(i));
            boardConfig->SetIPaddress(Rest);
        }
    }
    else {
        std::cout << "Warning: Unknown parameter " << Param << std::endl;
    }
}

//___________________________________________________________________
void TSetup::EnableSlave( const int mychip )
{
    bool toggle = false;
    TChipConfig* mychipConfig = GetChipConfig( mychip );
    int mychipID = mychipConfig->GetChipId();
    if ( mychipConfig->IsOBMaster() ) {
        for ( int i = mychipID + 1; i <= mychipID + 6; i++ ) {
            if ( GetChipConfig(i)->IsEnabled() ) {
                toggle = true;
                break;
            }
        }
    }
    mychipConfig->SetEnableSlave( toggle );
}

// Make the daisy chain for OB readout, based on enabled chips
// i.e. to be called after CheckControlInterface
//___________________________________________________________________
void TSetup::MakeDaisyChain()
{
    //   TConfig* config, std::vector <TAlpide *> * chips
    int firstLow[8], firstHigh[8], lastLow[8], lastHigh[8];
    
    for (int imod = 0; imod < 8; imod ++) {
        firstLow  [imod] = 0x77;
        firstHigh [imod] = 0x7f;
        lastLow   [imod] = 0x0;
        lastHigh  [imod] = 0x8;
    }
    
    // find the first and last enabled chip in each row
    for (unsigned int i = 0; i < fChips.size(); i++) {
        if (!GetChipConfig(i)->IsEnabled()) continue;
        int chipId   = GetChipConfig(i)->GetChipId();
        int modId    = (chipId & 0x70) >> 4;
        
        if ( (chipId & 0x8) && (chipId < firstHigh [modId])) firstHigh [modId] = chipId;
        if (!(chipId & 0x8) && (chipId < firstLow  [modId])) firstLow  [modId] = chipId;
        
        if ( (chipId & 0x8) && (chipId > lastHigh [modId])) lastHigh [modId] = chipId;
        if (!(chipId & 0x8) && (chipId > lastLow  [modId])) lastLow  [modId] = chipId;
    }
    
    for (unsigned int i = 0; i < fChips.size(); i++) {
        if (!GetChipConfig(i)->IsEnabled()) continue;
        int chipId   = GetChipConfig(i)->GetChipId();
        int modId    = (chipId & 0x70) >> 4;
        int previous = -1;
        
        // first chip in row gets token and previous chip is last chip in row (for each module)
        // (first and last can be same chip)
        if (chipId == firstLow [modId]) {
            GetChipConfig(i)->SetInitialToken(true);
            GetChipConfig(i)->SetPreviousId(lastLow [modId]);
        }
        else if (chipId == firstHigh [modId]) {
            GetChipConfig(i)->SetInitialToken(true);
            GetChipConfig(i)->SetPreviousId(lastHigh [modId]);
        }
        // chip is enabled, but not first in row; no token, search previous chip
        // search range: first chip in row on same module .. chip -1
        else if (chipId & 0x8) {
            GetChipConfig(i)->SetInitialToken(false);
            for (int iprev = chipId - 1; (iprev >= firstHigh [modId]) && (previous == -1); iprev--) {
                if (GetChipConfigById(iprev)->IsEnabled()) {
                    previous = iprev;
                }
            }
            GetChipConfig(i)->SetPreviousId(previous);
        }
        else if (!(chipId & 0x8)) {
            GetChipConfig(i)>SetInitialToken(false);
            for (int iprev = chipId - 1; (iprev >= firstLow [modId]) && (previous == -1); iprev--) {
                if (GetChipConfigById(iprev)->IsEnabled()) {
                    previous = iprev;
                }
            }
            GetChipConfig(i)->SetPreviousId(previous);
        }
        
        cout << "TSetup::MakeDaisyChain()  - Chip Id " << chipId
        << ", token = " << (bool)GetChipConfig(i)->GetInitialToken()
        << ", previous = " << GetChipConfig(i)->GetPreviousId() << endl;
    }
}

//___________________________________________________________________
void TSetup::ParseLine(const char* Line, char* Param, char* Rest, int* Chip)
{
    char MyParam[132];
    char *MyParam2;
    if (!strchr(Line, '_')) {
        *Chip = -1;
        sscanf (Line,"%s\t%s",Param, Rest);
    }
    else {
        sscanf (Line,"%s\t%s", MyParam, Rest);
        MyParam2 = strtok(MyParam, "_");
        sprintf(Param, "%s", MyParam2);
        sscanf (strpbrk(Line, "_")+1, "%d", Chip);
    }
}

//___________________________________________________________________
void TSetup::ReadDeviceType( const char* deviceName )
{
    if ( !strcmp (deviceName, "CHIPDAQ")) {
        fDeviceType = TYPE_CHIP_DAQ;
    }
    else if (!strcmp(deviceName, "TELESCOPE")) {
        fDeviceType = TYPE_TELESCOPE;
    }
    else if (!strcmp(deviceName, "OBHIC")) {
        fDeviceType = TYPE_OBHIC;
    }
    else if (!strcmp(deviceName, "IBHIC")) {
        
        fDeviceType = TYPE_IBHIC;
    }
    else if (!strcmp(deviceName, "MFT5")) {
        fDeviceType = TYPE_MFT_LADDER5;
    }
    else if (!strcmp(deviceName, "MFT4")) {
        fDeviceType = TYPE_MFT_LADDER4;
    }
    else if (!strcmp(deviceName, "MFT3")) {
        fDeviceType = TYPE_MFT_LADDER3;
    }
    else if (!strcmp(deviceName, "MFT2")) {
        fDeviceType = TYPE_MFT_LADDER2;
    }
    else if (!strcmp(deviceName, "OBCHIPMOSAIC")) {
        fDeviceType = TYPE_OBCHIP_MOSAIC;
    }
    else if (!strcmp(deviceName, "IBCHIPMOSAIC")) {
        fDeviceType = TYPE_IBCHIP_MOSAIC;
    }
    else if (!strcmp(deviceName, "HALFSTAVE")) {
        fDeviceType = TYPE_HALFSTAVE;
    }
    else {
        cerr << "TSetup::ReadDeviceType() - device name = " << deviceName << endl;
        throw runtime_error( "TSetup::ReadDeviceType() - unknown setup type." );
    }
}

#pragma mark - device creation

//___________________________________________________________________
void TSetup::CreateDeviceConfig()
{
    if ( fCreatedConfig ) {
        throw runtime_error( "TSetup::CreateDeviceConfig() - device already created." );
    }
    switch ( fDeviceType )
    {
        case TYPE_CHIP_DAQ:
            fStartChipId = 16; // master OB chip
            fNChips = 1; // overwrite the value read in config file
            try { CreateOBSingleDAQ(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_TELESCOPE:
            fStartChipId = 16; // master OB chip
            // fNChips taken from the config file
            try { CreateTelescopeDAQ(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_OBHIC:
            // fStartChipId defined in CreateOB()
            fNChips = 15; // overwrite the value read in config file
            try { CreateOB(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_IBHIC:
            fStartChipId = 0;
            fNChips = 9; // overwrite the value read in config file
            try { CreateIB(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_MFT_LADDER5:
            fStartChipId = 4;
            fNChips = 5; // overwrite the value read in config file
            try { CreateMFTLadder(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_MFT_LADDER4:
            fStartChipId = 5;
            fNChips = 4; // overwrite the value read in config file
            try { CreateMFTLadder(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_MFT_LADDER3:
            fStartChipId = 6;
            fNChips = 3; // overwrite the value read in config file
            try { CreateMFTLadder(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_MFT_LADDER2:
            fStartChipId = 7;
            fNChips = 2; // overwrite the value read in config file
            try { CreateMFTLadder(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_OBCHIP_MOSAIC:
            fStartChipId = 16; // master OB chip
            fNChips = 1; // overwrite the value read in config file
            try { CreateOBSingleMosaic(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_IBCHIP_MOSAIC:
            fStartChipId = 0; // imposed by the carrier board (resistor removed)
            fNChips = 1; // overwrite the value read in config file
            try { CreateIBSingleMosaic(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        case TYPE_HALFSTAVE:
            try { CreateHalfStave(); } catch ( ... ) {
                throw runtime_error( "TSetup::CreateDeviceConfig() - failed." );
            }
            break;
        default:
            throw runtime_error( "TSetup::CreateDeviceConfig() - unknown device type, doing nothing." );
            return;
    }
}

//___________________________________________________________________
void TSetup::CreateHalfStave()
{
    if ( fCreatedConfig ) {
        return;
    }
    if ( !GetNModules() ) {
        throw runtime_error( "TSetup::CreateHalfStave() - no module!" );
    }
    fBoardType = kBOARD_MOSAIC;
    const int nBoards = 2;
    for (int iboard = 0; iboard < nBoards; i++) {
        auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
        fBoardConfigs.push_back( move(newBoardConfig) );
    }
    for (int imod = 0; imod < GetNModules(); imod++) {
        int moduleId = imod + 1;
        for (int i = 0; i < 15; i++) {
            if (i == 7) continue;
            int chipId = i + ((moduleId & 0x7) << 4);
            auto newChipConfig = make_shared<TChipConfig>( chipID );
            fChipConfigs.push_back( move(newChipConfig) );
        }
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::CreateHalfStave() - done" << endl;
    }
    fCreatedConfig = true;
}

//___________________________________________________________________
void TSetup::CreateIB()
{
    if ( fCreatedConfig ) {
        return;
    }
    fBoardType = kBOARD_MOSAIC;
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fBoardConfigs.push_back( move(newBoardConfig) );
    
    for (int ichip = 0; ichip < GetNChips(); ichip ++) {
        int chipID = ichip;
        auto newChipConfig = make_shared<TChipConfig>( chipID );
        fChipConfigs.push_back( move(newChipConfig) );
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::CreateIB() - done" << endl;
    }
    fCreatedConfig = true;
}

//___________________________________________________________________
void TSetup::CreateIBSingleMosaic()
{
    if ( fCreatedConfig ) {
        return;
    }
    fBoardType = kBOARD_MOSAIC;
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fBoardConfigs.push_back( move(newBoardConfig) );
    
    auto newChipConfig = make_shared<TChipConfig>( GetStartChipID() );
    fChipConfigs.push_back( move(newChipConfig) );
    
    if ( fVerboseLevel ) {
        cout << "TSetup::CreateIBSingleMosaic() - done" << endl;
    }
    fCreatedConfig = true;
}

//___________________________________________________________________
void TSetup::CreateMFTLadder()
{
    if ( fCreatedConfig ) {
        return;
    }
    fBoardType = kBOARD_MOSAIC;
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fBoardConfigs.push_back( move(newBoardConfig) );
    
    for (int ichip = 0; ichip < GetNChips(); ichip ++) {
        int chipID = GetStartChipID() + ichip;
        auto newChipConfig = make_shared<TChipConfig>( chipID );
        fChipConfigs.push_back( move(newChipConfig) );
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::CreateMFTLadder() - done" << endl;
    }
    fCreatedConfig = true;
}

//___________________________________________________________________
void TSetup::CreateOB()
{
    if ( fCreatedConfig ) {
        return;
    }
    fBoardType = kBOARD_MOSAIC;
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fBoardConfigs.push_back( move(newBoardConfig) );
    
    for (int ichip = 0; ichip < GetNChips(); ichip ++) {
        if (ichip == 7) continue;
        int chipId = ichip + ((DEFAULT_MODULE_ID & 0x7) << 4);
        if ( ichip == 0 ) {
            fStartChipId = chipID;
        }
        auto newChipConfig = make_shared<TChipConfig>( chipID );
        fChipConfigs.push_back( move(newChipConfig) );
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::CreateOB() - done" << endl;
    }
    fCreatedConfig = true;
}

//___________________________________________________________________
void TSetup::CreateOBSingleDAQ()
{
    if ( fCreatedConfig ) {
        return;
    }
    fBoardType = kBOARD_DAQ;
    auto newBoardConfig = make_shared<TBoardConfigDAQ>();
    fBoardConfigs.push_back( move(newBoardConfig) );
    
    auto newChipConfig = make_shared<TChipConfig>( GetStartChipID() );
    newChipConfig->SetEnableSlave( false ); // with no slave
    fChipConfigs.push_back( move(newChipConfig) );
    
    if ( fVerboseLevel ) {
        cout << "TSetup::CreateOBSingleDAQ() - done" << endl;
    }
    fCreatedConfig = true;
}

//___________________________________________________________________
void TSetup::CreateOBSingleMosaic()
{
    if ( fCreatedConfig ) {
        return;
    }
    fBoardType = kBOARD_MOSAIC;
    auto newBoardConfig = make_shared<TBoardConfigMOSAIC>();
    fBoardConfigs.push_back( move(newBoardConfig) );
    
    auto newChipConfig = make_shared<TChipConfig>( GetStartChipID() );
    newChipConfig->SetEnableSlave( false ); // with no slave
    fChipConfigs.push_back( move(newChipConfig) );
    
    if ( fVerboseLevel ) {
        cout << "TSetup::CreateOBSingleMosaic() - done" << endl;
    }
    fCreatedConfig = true;
}

//___________________________________________________________________
void TSetup::CreateTelescopeDAQ()
{
    if ( fCreatedConfig ) {
        return;
    }
    if ( GetNChips() < 1 ) {
        throw runtime_error( "TSetup::ReadDeviceType() - TELESCOPE not useable with less than 1 chips." );
    }
    fBoardType = kBOARD_DAQ;
    int nBoards = fNChips; // one DAQ board per chip
    for (int iboard = 0; iboard < nBoards; iboard ++) {
        auto newBoardConfig = make_shared<TBoardConfigDAQ>();
        fBoardConfigs.push_back( move(newBoardConfig) );
    }
    for (int i = 0; i < GetNChips(); i++) {
        auto newChipConfig = make_shared<TChipConfig>( GetStartChipID() );
        newChipConfig->SetEnableSlave( false ); // with no slave
        fChipConfigs.push_back( move(newChipConfig) );
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::CreateTelescopeDAQ() - done" << endl;
    }
    fCreatedConfig = true;
}

#pragma mark - setup initialisation

// implicit assumptions on the setup in this method
// - chips of master 0 of all modules are connected to 1st mosaic, chips of master 8 to 2nd MOSAIC
//___________________________________________________________________
void TSetup::InitSetupHalfStave()
{
    if ( fInitialisedSetup || !fCreatedConfig ) {
        return;
    }
    try {
        if ( fDeviceType != TYPE_HALFSTAVE ) {
            throw runtime_error( "TSetup::InitSetupIB() - wrong device type." );
        }
    } catch (std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    try {
        if ( fBoardType != kBOARD_MOSAIC ) {
            throw runtime_error( "TSetup::InitSetupIB() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupHalfStave() - start" << endl;
    }
    for (int i = 0; i < GetNBoards(); i++) {
        shared_ptr<TBoardConfigMOSAIC> boardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC> GetBoardConfig(i);
        boardConfig->SetInvertedData(false);  //already inverted in the adapter plug ?
        boardConfig->SetSpeedMode(Mosaic::RCV_RATE_400);
    }
    
    for (int i = 0; i < GetNChips(); i++) {
        shared_ptr<TChipConfig> chipConfig = GetChipConfig(i);
        int          chipId     = chipConfig->GetChipId();
        int          mosaic     = (chipId & 0x1000) ? 1:0;
        auto alpide = make_shared<TAlpide>( chipConfig );
        fChips.push_back( move(alpide) );
        (fChips.at(i))->SetReadoutBoard(fBoards.at(mosaic));
        
        // to be checked when final layout of adapter fixed
        int ci  = 0;
        int rcv = (chipId & 0x7) ? -1 : 9*ci; //FIXME
        (fBoards.at(mosaic))->AddChip(chipId, ci, rcv);
    }
    for (int i = 0; i < GetNChips(); i++) {
        EnableSlave( i );
    }
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    sleep(5);
    MakeDaisyChain();
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupHalfStave() - end" << endl;
    }
    fInitialisedSetup = true;
}

// Setup definition for inner barrel stave with MOSAIC
//    - all chips connected to same control interface
//    - each chip has its own receiver, mapping defined in RCVMAP
//___________________________________________________________________
void TSetup::InitSetupIB()
{
    if ( fInitialisedSetup ) {
        return;
    }
    try {
        if ( fDeviceType != TYPE_IBHIC ) {
            throw runtime_error( "TSetup::InitSetupIB() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    try {
        if ( fBoardType != kBOARD_MOSAIC ) {
            throw runtime_error( "TSetup::InitSetupIB() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupIB() - start" << endl;
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC>GetBoardConfig(0);
    boardConfig->SetInvertedData( false );
    Mosaic::TReceiverSpeed speed;
    
    switch (GetChipConfig(0)->GetParamValue("LINKSPEED")) {
        case 400:
            speed = Mosaic::RCV_RATE_400;
            break;
        case 600:
            speed = Mosaic::RCV_RATE_600;
            break;
        case 1200:
            speed = Mosaic::RCV_RATE_1200;
            break;
        default:
            cout << "TSetup::InitSetupIB() - Warning: invalid link speed, using 1200" << endl;
            speed = Mosaic::RCV_RATE_1200;
            break;
    }
    cout << "TSetup::InitSetupIB() - Speed mode = " << speed << endl;
    boardConfig->SetSpeedMode( speed );

    auto newBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fBoards.push_back( move(newBoard) );
    
    for (int i = 0; i < GetNChips(); i++) {
        
        shared_ptr<TChipConfig> chipConfig = GetChipConfig(i);
        int control  = chipConfig->GetParamValue("CONTROLINTERFACE");
        int receiver = chipConfig->GetParamValue("RECEIVER");
        auto alpide = make_shared<TAlpide>( chipConfig );
        alpide->SetReadoutBoard( GetBoard(0) );
        fChips.push_back( move(alpide) );
        
        if (control  < 0) {
            control = 0;
            chipConfig->SetParamValue("CONTROLINTERFACE", control);
        }
        if (receiver < 0) {
            receiver = boardConfig->GetRCVMAP(i);
            chipConfig->SetParamValue("RECEIVER", receiver);
        }
        GetBoard(0)->AddChip( chipConfig->GetChipId(), control, receiver );
    }
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupIB() - end" << endl;
    }
    fInitialisedSetup = true;
}

// Setup for a single chip in inner barrel mode with MOSAIC
// Assumption : ICM_H board is used to interface the chip and the MOSAIC
//___________________________________________________________________
void TSetup::InitSetupIBSingleMosaic()
{
    if ( fInitialisedSetup ) {
        return;
    }
    try {
        if ( fDeviceType != TYPE_IBCHIP_MOSAIC ) {
            throw runtime_error( "TSetup::InitSetupIBSingleMosaic() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    try {
        if ( fBoardType != kBOARD_MOSAIC ) {
            throw runtime_error( "TSetup::InitSetupIBSingleMosaic() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupIBSingleMosaic() - start" << endl;
    }
    
    shared_ptr<TChipConfig> chipConfig  = GetChipConfig(0);
    int control  = chipConfig->GetParamValue("CONTROLINTERFACE");
    int receiver = chipConfig->GetParamValue("RECEIVER");
    
    if ( receiver < 0 ) {
        // Imposed by hardware of ICM_H board
        receiver = 4;
        chipConfig->SetParamValue("RECEIVER", receiver);
    }
    if ( control  < 0 ) {
        control  = 0;
        chipConfig->SetParamValue("CONTROLINTERFACE", control);
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC> GetBoardConfig(0);
    boardConfig->SetInvertedData( false );
    Mosaic::TReceiverSpeed speed;
    switch ( chipConfig->GetParamValue("LINKSPEED") ) {
        case 400:
            speed = Mosaic::RCV_RATE_400;
            break;
        case 600:
            speed = Mosaic::RCV_RATE_600;
            break;
        case 1200:
            speed = Mosaic::RCV_RATE_1200;
            break;
        default:
            cout << "TSetup::InitSetupIBSingleMosaic() - Warning: invalid link speed, using 1200" << endl;
            speed = Mosaic::RCV_RATE_1200;
            break;
    }
    boardConfig->SetSpeedMode( speed );
    
    auto myBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fBoards.push_back( move(myBoard) );
    
    auto alpide = make_shared<TAlpide>( chipConfig );
    alpide->SetReadoutBoard( GetBoard(0) );
    fChips.push_back( move(alpide) );
    GetBoard(0)->AddChip( chipConfig->GetChipId(), control, receiver );
    
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupIBSingleMosaic() - end" << endl;
    }
    fInitialisedSetup = true;
}

// Setup definition for MFT ladder with MOSAIC
//    - all chips connected to same control interface
//    - each chip has its own receiver, mapping is a non-trivial function of RCVMAP
//___________________________________________________________________
void TSetup::InitSetupMFTLadder()
{
    if ( fInitialisedSetup ) {
        return;
    }
    try {
        if ( !IsMFTLadder() ) {
            throw runtime_error( "TSetup::InitSetupMFTLadder() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    try {
        if ( fBoardType != kBOARD_MOSAIC ) {
            throw runtime_error( "TSetup::InitSetupMFTLadder() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupMFTLadder() - start" << endl;
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC>GetBoardConfig(0);
    boardConfig->SetInvertedData( false );
    Mosaic::TReceiverSpeed speed;
    
    switch (GetChipConfig(0)->GetParamValue("LINKSPEED")) {
        case 400:
            speed = Mosaic::RCV_RATE_400;
            break;
        case 600:
            speed = Mosaic::RCV_RATE_600;
            break;
        case 1200:
            speed = Mosaic::RCV_RATE_1200;
            break;
        default:
            cout << "TSetup::InitSetupMFTLadder() - Warning: invalid link speed, using 1200" << endl;
            speed = Mosaic::RCV_RATE_1200;
            break;
    }
    cout << "TSetup::InitSetupMFTLadder() - Speed mode = " << speed << endl;
    boardConfig->SetSpeedMode( speed );
    
    auto newBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fBoards.push_back( move(newBoard) );
    
    for (int i = 0; i < GetNChips(); i++) {
        
        shared_ptr<TChipConfig> chipConfig = GetChipConfig(i);
        int control  = chipConfig->GetParamValue("CONTROLINTERFACE");
        int receiver = chipConfig->GetParamValue("RECEIVER");
        auto alpide = make_shared<TAlpide>( chipConfig );
        alpide->SetReadoutBoard( GetBoard(0) );
        fChips.push_back( move(alpide) );
        
        if (control  < 0) {
            control = 0;
            chipConfig->SetParamValue("CONTROLINTERFACE", control);
        }
        if (receiver < 0) {
            // For MFT, the last chip (far from connector) is always:
            // - mapped with the RCVMAP[0]
            // - with chipId = 8
            // - at position GetNChips()-1 in the vector of chips.
            // And the chip id increases from the first chip (at position 0 in
            // the vector of chips, close to connector) to the last chip.
            // As a result:
            // RCVMAP[0] = 3, chipId = 8 (last chip on all types of ladder)
            // RCVMAP[1] = 5, chipId = 7 (1st chip on 2-chips ladder)
            // RCVMAP[2] = 7, chipId = 6 (1st chip on 3-chips ladder)
            // RCVMAP[3] = 8, chipId = 5 (1st chip on 4-chips ladder)
            // RCVMAP[4] = 6, chipId = 4 (1st chip on 5-chips ladder)
            int index = GetNChips() - 1 - i;
            receiver = boardConfig->GetRCVMAP(index);
            chipConfig->SetParamValue("RECEIVER", receiver);
        }
        GetBoard(0)->AddChip( chipConfig->GetChipId(), control, receiver );
    }
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    fInitialisedSetup = true;
}

// Setup definition for outer barrel module with MOSAIC
//    - module ID (3 most significant bits of chip ID) defined by moduleId
//      (usually 1)
//    - chips connected to two different control interfaces
//    - masters send data to two different receivers (0 and 1)
//    - receiver number for slaves set to -1 (not connected directly to receiver)
//      (this ensures that a receiver is disabled only if the connected master is disabled)
//___________________________________________________________________
void TSetup::InitSetupOB()
{
    if ( fInitialisedSetup ) {
        return;
    }
    try {
        if ( fDeviceType != TYPE_OBHIC ) {
            throw runtime_error( "TSetup::InitSetupOB() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    try {
        if ( fBoardType != kBOARD_MOSAIC ) {
            throw runtime_error( "TSetup::InitSetupOB() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupOB() - start" << endl;
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC> GetBoardConfig(0);
    boardConfig->SetInvertedData(boardConfig->IsInverted()); // ???: circular definition? (AR)
    boardConfig->SetSpeedMode( Mosaic::RCV_RATE_400 );
    auto newBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fBoards.push_back( move(newBoard) );
    
    for (int i = 0; i < GetNChips(); i++) {

        shared_ptr<TChipConfig> chipConfig = GetChipConfig(i);
        int chipId = chipConfig->GetChipId();
        int control = chipConfig->GetParamValue("CONTROLINTERFACE");
        int receiver = chipConfig->GetParamValue("RECEIVER");
        
        if (chipId%8!=0) chipConfig->SetParamValue("LINKSPEED", "-1"); // deactivate the DTU/PLL for none master chips
        
        auto alpide = make_shared<TAlpide>( chipConfig );
        alpide->SetReadoutBoard( GetBoard(0) );
        fChips.push_back( move(alpide) );

        if (i < 7) {              // first master-slave row
            if (control < 0) {
                control = 1;
                chipConfig->SetParamValue("CONTROLINTERFACE", control);
            }
            if (receiver < 0) {
                receiver = 9;
                chipConfig->SetParamValue("RECEIVER", receiver);
            }
        } else {                    // second master-slave row
            if (control < 0) {
                control = 0;
                chipConfig->SetParamValue("CONTROLINTERFACE", control);
            }
            if (receiver < 0) {
                receiver = 0;
                chipConfig->SetParamValue("RECEIVER", receiver);
            }
        }
        GetBoard(0)->AddChip(chipId, control, receiver);
    }
    for (int i = 0; i < GetNChips(); i++) {
        EnableSlave( i );
    }
    try {
        CheckControlInterface();
    } catch ( runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    sleep(5);
    MakeDaisyChain();
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupOB() - end" << endl;
    }
    fInitialisedSetup = true;
}

//___________________________________________________________________
void TSetup::InitSetupOBSingleDAQ()
{
    if ( fInitialisedSetup ) {
        return;
    }
    try {
        if ( fDeviceType != TYPE_CHIP_DAQ ) {
            throw runtime_error( "TSetup::InitSetupOBSingleDAQ() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    try {
        if ( fBoardType != kBOARD_DAQ ) {
            throw runtime_error( "TSetup::InitSetupOBSingleDAQ() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupOBSingleDAQ() - start" << endl;
    }

    InitLibUsb();
    //  The following code searches the USB bus for DAQ boards, creates them and adds them to the readout board vector:
    FindDAQBoards();
    if ( fBoards.size() != 1 ) {
        throw runtime_error( "TSetup::InitSetupOBSingleDAQ() - Error in creating readout board object." );
    }
    
    shared_ptr<TChipConfig> chipConfig = GetChipConfig(0);
    chipConfig->SetParamValue("LINKSPEED", "-1");
    // values for control interface and receiver currently ignored for DAQ board
    //  int               control     = chipConfig->GetParamValue("CONTROLINTERFACE");
    //  int               receiver    = chipConfig->GetParamValue("RECEIVER");
    int control = 0;
    int receiver = 0;
    // for Cagliari DAQ board disable DDR and Manchester encoding
    chipConfig->SetEnableDdr( false );
    chipConfig->SetDisableManchester( true );
    
    // create chip object and connections with readout board
    auto alpide = make_shared<TAlpide>( chipConfig );
    alpide->SetReadoutBoard( GetBoard(0) );
    fChips.push_back( move(alpide) );
    GetBoard(0)->AddChip(chipConfig->GetChipId(), control, receiver);
    
    shared_ptr<TReadoutBoardDAQ> myDAQBoard = dynamic_pointer_cast<TReadoutBoardDAQ> GetBoard(0);
    powerOn(myDAQBoard);
    
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupOBSingleDAQ() - end" << endl;
    }
    fInitialisedSetup = true;
}

// Setup for a single chip in outer barrel mode with MOSAIC
// Assumption : ITS adaptors are used to interface the chip and the MOSAIC
// (see https://twiki.cern.ch/twiki/bin/view/ALICE/ALPIDE-adaptor-boards)
//___________________________________________________________________
void TSetup::InitSetupOBSingleMosaic()
{
    if ( fInitialisedSetup ) {
        return;
    }
    try {
        if ( fDeviceType != TYPE_OBCHIP_MOSAIC ) {
            throw runtime_error( "TSetup::InitSetupOBSingleMosaic() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    try {
        if ( fBoardType != kBOARD_MOSAIC ) {
            throw runtime_error( "TSetup::InitSetupOBSingleMosaic() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupOBSingleMosaic() - start" << endl;
    }

    shared_ptr<TChipConfig> chipConfig  = GetChipConfig(0);
    int control  = chipConfig->GetParamValue("CONTROLINTERFACE");
    int receiver = chipConfig->GetParamValue("RECEIVER");
    
    if ( receiver < 0 ) {
        receiver = 3;   // HSData is connected to pins for first chip on a stave
        chipConfig->SetParamValue("RECEIVER", receiver);
    }
    if ( control  < 0 ) {
        control  = 0;
        chipConfig->SetParamValue("CONTROLINTERFACE", control);
    }
    
    shared_ptr<TBoardConfigMOSAIC> boardConfig = dynamic_pointer_cast<TBoardConfigMOSAIC> GetBoardConfig(0);
    boardConfig->SetInvertedData( false );
    boardConfig->SetSpeedMode( Mosaic::RCV_RATE_400 );
    
    auto myBoard = make_shared<TReadoutBoardMOSAIC>( boardConfig );
    fBoards.push_back( move(myBoard) );
    
    auto alpide = make_shared<TAlpide>( chipConfig );
    alpide->SetReadoutBoard( GetBoard(0) );
    fChips.push_back( move(alpide) );
    GetBoard(0)->AddChip(chipConfig->GetChipId(), control, receiver);

    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupOBSingleMosaic() - end" << endl;
    }
    fInitialisedSetup = true;
}


//___________________________________________________________________
void TSetup::InitSetupTelescopeDAQ()
{
    if ( fInitialisedSetup ) {
        return;
    }
    try {
        if ( fDeviceType != TYPE_TELESCOPE ) {
            throw runtime_error( "TSetup::InitSetupTelescopeDAQ() - wrong device type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    try {
        if ( fBoardType != kBOARD_DAQ ) {
            throw runtime_error( "TSetup::InitSetupTelescopeDAQ() - wrong board type." );
        }
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit();
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupTelescopeDAQ() - start" << endl;
    }

    // FIXME: not implemeted yet
    cout << "TSetup::InitSetupTelescopeDAQ() - NOT IMPLEMENTED YET" << endl;
    
    if ( fVerboseLevel ) {
        cout << "TSetup::InitSetupOBSingleMosaic() - end" << endl;
    }
    fInitialisedSetup = true;

}

#pragma mark - Specific to DAQ board settings

//___________________________________________________________________
bool TSetup::AddDAQBoard( shared_ptr<libusb_device> device )
{
    // note: this should change to use the correct board config according to index or geographical id
    shared_ptr<TBoardConfigDAQ> boardConfig = dynamic_pointer_cast<TBoardConfigDAQ>(GetBoardConfig(0));
    auto readoutBoard = make_shared<TReadoutBoardDAQ>(device, boardConfig);
    
    if ( readoutBoard ) {
        fBoards->push_back( move(readoutBoard) );
        return true;
    }
    return false;
}

//___________________________________________________________________
void TSetup::FindDAQBoards()
{
    libusb_device **list;
    
    if ( Setup::fContext == 0 ) {
        throw runtime_error( "TSetup::FindDAQBoards() - Error, libusb not initialised." );
    }
    ssize_t cnt = libusb_get_device_list( Setup::fContext, &list );
    
    if ( cnt < 0 ) {
        throw runtime_error( "TSetup::FindDAQBoards() - Error getting device list." );
    }
    
    for ( ssize_t i = 0; i < cnt; i++ ) {
        libusb_device *device = list[i];
        if ( IsDAQBoard(device) ) {
            if ( AddDAQBoard( device ) ) {
                libusb_free_device_list(list, 1);
                cerr << "TSetup::FindDAQBoards() - Problem adding DAQ board." << endl;
            }
        }
    }
    if ( fVerboseLevel ) {
        cout << "TSetup::FindDAQBoards() - Found " << fBoards.size() << " DAQ boards" << endl;
    }
    libusb_free_device_list(list, 1);
}

//___________________________________________________________________
void TSetup::InitLibUsb()
{
    int err = libusb_init( &Setup::fContext );
    if (err) {
        cerr << "TSetup::InitLibUsb() - Error " << err << endl;
        throw runtime_error( "TSetup::InitLibUsb() - Error while trying to init libusb." );
    }
}

//___________________________________________________________________
bool TSetup::IsDAQBoard( shared_ptr<libusb_device> device )
{
    libusb_device_descriptor desc;
    libusb_get_device_descriptor(device, &desc);
    
    // std::cout << std::hex << "Vendor id " << (int)desc.idVendor << ", Product id " << (int)desc.idProduct << std::dec << std::endl;
    
    if ((desc.idVendor == DAQ_BOARD_VENDOR_ID) && (desc.idProduct == DAQ_BOARD_PRODUCT_ID)) {
        //std::cout << "Serial number " << (int)desc.iSerialNumber << std::endl;
        return true;
    }
    
    return false;
}

//___________________________________________________________________
void TSetup::PowerOnDaqBoard( shared_ptr<TReadoutBoardDAQ> aDAQBoard )
{
    int overflow;
    
    if ( aDAQBoard->PowerOn(overflow) ) cout << "LDOs are on" << endl;
    else cout << "LDOs are off" << endl;
    cout << "Version = " << std::hex << aDAQBoard->ReadFirmwareVersion()
    << std::dec << endl;
    aDAQBoard->SendOpCode(Alpide::OPCODE_GRST);
    //sleep(1); // sleep necessary after GRST? or PowerOn?
    
    cout << "Analog Current  = " << aDAQBoard->ReadAnalogI()     << endl;
    cout << "Digital Current = " << aDAQBoard->ReadDigitalI()    << endl;
    cout << "Temperature     = " << aDAQBoard->ReadTemperature() << endl;
}






