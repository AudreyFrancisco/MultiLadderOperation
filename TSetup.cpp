#include <iostream>
#include <fstream>
#include <string.h>
#include <stdexcept>
#include <stdlib.h>
#include "TSetup.h"
#include "TDevice.h"
#include "TDeviceBuilder.h"
#include "TDeviceBuilderHalfStave.h"
#include "TDeviceBuilderIB.h"
#include "TDeviceBuilderIBSingleMosaic.h"
#include "TDeviceBuilderMFTLadder.h"
#include "TDeviceBuilderOB.h"
#include "TDeviceBuilderOBSingleDAQ.h"
#include "TDeviceBuilderOBSingleMosaic.h"
#include "TDeviceBuilderTelescopeDAQ.h"
#include "TScanConfig.h"

using namespace std;

const string TSetup::NEWALPIDEVERSION = "1.0_mft";

#pragma mark - Constructors/destructor

//___________________________________________________________________
TSetup::TSetup() : TVerbosity(),
    fConfigFileName( "Config.cfg" ),
    fConfigFile( nullptr ),
    fDeviceBuilder( nullptr ),
    fDevice( nullptr ),
    fScanConfig( nullptr )
{
    cout << "**  ALICE new-alpide-software  v." << NEWALPIDEVERSION
         << " **" << endl<< endl;
    fScanConfig = make_shared<TScanConfig>();
}

//___________________________________________________________________
TSetup::~TSetup()
{
    fDevice.reset();
    fDeviceBuilder.reset();
    fScanConfig.reset();
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
            exit(0);
        }
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
                exit(0);
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
                exit(0);
            default:
                return;
        }
}

//___________________________________________________________________
void TSetup::DumpConfigToFile( string fName )
{
    // FIXME: not implemented yet
    cout << "TSetup::DumpConfigToFile() - NOT IMPLEMENTED YET, filename = "
    << fName << endl;
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
        exit(0);
    }
    
    bool isConfigFrozen = false;
    TDeviceType dt = TDeviceType::kUNKNOWN;
    
    // first look for the type of setup in order to initialise config structure
    while ( (!isConfigFrozen) && (fgets(Line, 1023, fConfigFile) != NULL) ) {
        
        if ((Line[0] == '\n') || (Line[0] == '#')) continue;
        ParseLine (Line, Param, Rest, &Chip);
        if (!strcmp(Param, "DEVICE")) {
            try {
                dt = ReadDeviceType( Rest );
            } catch ( std::runtime_error &err ) {
                cerr << err.what() << endl;
                exit(0);
            }
            try {
                InitDeviceBuilder( dt );
            } catch ( std::runtime_error &err ) {
                cerr << err.what() << endl;
                exit(0);
            }
        }
        int nChips = 0;
        if (!strcmp(Param,"NCHIPS")){
            sscanf(Rest, "%d", &nChips);
            if ( dt == TDeviceType::kTELESCOPE ) {
                (dynamic_pointer_cast<TDeviceBuilderTelescope>(fDeviceBuilder))->SetNChips( nChips );
            }
        }
        int nModules = 0;
        if (!strcmp(Param,"NMODULES")){
            sscanf(Rest, "%d", &nModules);
            if ( dt == TDeviceType::kHALFSTAVE ) {
                (dynamic_pointer_cast<TDeviceBuilderHalfStave>(fDeviceBuilder))->SetNModules( nModules );
            }
        }
        try {
            // type and nchips has been found (nchips only needed for device kTELESCOPE)
            // calls the appropriate method, which in turn calls
            // the constructors for board and chip configs
            fDeviceBuilder->CreateDeviceConfig();
        } catch ( std::runtime_error &err ) {
            cerr << err.what() << endl;
            exit(0);
        }
        if ( fDevice ) {
            isConfigFrozen = fDevice->IsConfigFrozen();
        }
    }
    // now read the rest
    while (fgets(Line, 1023, fConfigFile) != NULL) {
        DecodeLine(Line);
    }
    
    // and initialise the device
    fDeviceBuilder->InitSetup();
}

#pragma mark - private methods

//___________________________________________________________________
void TSetup::DecodeLine(const char* Line)
{
    int Chip;
    char Param[128], Rest[896];
    if ((Line[0] == '\n') || (Line[0] == '#')) {   // empty Line or comment
        return;
    }
    
    ParseLine(Line, Param, Rest, &Chip);
    
    if ( fScanConfig->IsParameter(Param) ) {
        fScanConfig->SetParamValue (Param, Rest);
    } else {
        fDeviceBuilder->SetDeviceParamValue( Param, Rest, Chip );
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
TDeviceType TSetup::ReadDeviceType( const char* deviceName )
{
    TDeviceType dt = TDeviceType::kUNKNOWN;
    
    if ( !strcmp (deviceName, "CHIPDAQ")) {
        dt = TDeviceType::kCHIP_DAQ;
    }
    else if (!strcmp(deviceName, "TELESCOPE")) {
        dt = TDeviceType::kTELESCOPE;
    }
    else if (!strcmp(deviceName, "OBHIC")) {
        dt = TDeviceType::kOBHIC;
    }
    else if (!strcmp(deviceName, "IBHIC")) {
        dt = TDeviceType::kIBHIC;
    }
    else if (!strcmp(deviceName, "MFT5")) {
        dt = TDeviceType::kMFT_LADDER5;
    }
    else if (!strcmp(deviceName, "MFT4")) {
        dt = TDeviceType::kMFT_LADDER4;
    }
    else if (!strcmp(deviceName, "MFT3")) {
        dt = TDeviceType::kMFT_LADDER3;
    }
    else if (!strcmp(deviceName, "MFT2")) {
        dt = TDeviceType::kMFT_LADDER2;
    }
    else if (!strcmp(deviceName, "OBCHIPMOSAIC")) {
        dt = TDeviceType::kOBCHIP_MOSAIC;
    }
    else if (!strcmp(deviceName, "IBCHIPMOSAIC")) {
        dt = TDeviceType::kIBCHIP_MOSAIC;
    }
    else if (!strcmp(deviceName, "HALFSTAVE")) {
        dt = TDeviceType::kHALFSTAVE;
    }
    else {
        cerr << "TSetup::ReadDeviceType() - unidentified device name = " << deviceName << endl;
    }
    return dt;
}

//___________________________________________________________________
void TSetup::InitDeviceBuilder( TDeviceType dt )
{
    if ( fDeviceBuilder ) {
        throw runtime_error( "TSetup::InitDeviceBuilder() - can not overwrite existing instance of TDeviceBuilder." );
    }
    switch ( dt )
    {
        case TDeviceType::kCHIP_DAQ:
            fDeviceBuilder = make_shared<TDeviceBuilderOBSingleDAQ>();
            break;
        case TDeviceType::kTELESCOPE:
            fDeviceBuilder = make_shared<TDeviceBuilderTelescope>();
            break;
        case TDeviceType::kOBHIC:
            fDeviceBuilder = make_shared<TDeviceBuilderOB>();
            break;
        case TDeviceType::kIBHIC:
            fDeviceBuilder = make_shared<TDeviceBuilderIB>();
            break;
        case TDeviceType::kMFT_LADDER5:
            fDeviceBuilder = make_shared<TDeviceBuilderMFTLadder>();
            break;
        case TDeviceType::kMFT_LADDER4:
            fDeviceBuilder = make_shared<TDeviceBuilderMFTLadder>();
            break;
        case TDeviceType::kMFT_LADDER3:
            fDeviceBuilder = make_shared<TDeviceBuilderMFTLadder>();
            break;
        case TDeviceType::kMFT_LADDER2:
            fDeviceBuilder = make_shared<TDeviceBuilderMFTLadder>();
            break;
        case TDeviceType::kOBCHIP_MOSAIC:
            fDeviceBuilder = make_shared<TDeviceBuilderOBSingleMosaic>();
            break;
        case TDeviceType::kIBCHIP_MOSAIC:
            fDeviceBuilder = make_shared<TDeviceBuilderIBSingleMosaic>();
            break;
        case TDeviceType::kHALFSTAVE:
            fDeviceBuilder = make_shared<TDeviceBuilderHalfStave>();
            break;
        default:
            throw runtime_error( "TSetup::InitDeviceBuilder() - unknown device type, doing nothing." );
            return;
    }
    fDeviceBuilder->CreateDevice();
    fDeviceBuilder->SetVerboseLevel( this->GetVerboseLevel() );
    try {
        fDeviceBuilder->SetDeviceType( dt );
    } catch ( std::runtime_error &err ) {
        cerr << err.what() << endl;
        exit(0);
    }
    fDevice = fDeviceBuilder->GetCurrentDevice();
}

