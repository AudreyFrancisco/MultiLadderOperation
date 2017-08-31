#include "TROOT.h"
#include <Riostream.h>
#include <TSystem.h>

using namespace std;

void load_classes() {
    gSystem->Load("AliPALPIDEFSRawStreamMS_cpp.so");
    gSystem->Load("BinaryPixel_cpp.so");
    gSystem->Load("BinaryCluster_cpp.so");
    gSystem->Load("BinaryPlane_cpp.so");
    gSystem->Load("BinaryEvent_cpp.so");
    gSystem->Load("helpers_cpp.so");

    cout << "load_classes() : Classes loadad." << endl;
}
