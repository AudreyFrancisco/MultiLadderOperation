# MFT fork of ITS new alpide software

The main interest into the ITS new alpide software is the following: it is meant to have ALPIDE test routines or utilities (such as FIFO scan, digital scan, threshold scan ...) that should be usable for any type of readout board (so far DAQ board and MOSAIC board, RU is being included as well, but not yet in this MFT fork). The test utilities will also be the same for different types of device (for e.g. single chip on a carrier board in IB mode, MFT ladder with 2 chips, MFT ladder with 5 chips ...). To be able to do so, the utilities must be fed at run time with a configuration describing how many chips are used, the configuration of each chip, and the type of readout board to be used.

The original ITS repository was forked into our own repository. The code was modified and optimized to match MFT specific needs. The _master_ branch was intended to be regularly kept in sync with the corresponding _master_ branch from the original project (but this stopped as it was too time consuming). The **default** branch is set to be the _mft_ branch. This default branch is protected, meaning that only users with Master rank or the owner of the repository can commit to this default branch. The _mft-dev_ branch is the one where new features for MFT are being developed and tested. Once these new features are validated, they are merged into the _mft_ branch. The Git workflow to be followed to add new features to _mft-dev_ is outlined in the [Versioning](#versioning) section below.

A Doxygen web-based documentation will be added soon.

## Getting Started

Clone the repository (you will get the default branch, i.e. _mft_):

```
$ git clone ssh://git@gitlab.cern.ch:7999/alice-MFT-upgrade/new-alpide-software.git
```

### Prerequisites

#### (a) Recent compiler

Make sure you have either at least clang version Apple LLVM version 8.1.0 (clang-802.0.42) if you work on MacOS X, or a gcc version with C++14 is enabled by default i.e. at least gcc 6. If you have an earlier version of gcc, you can easily install gcc 7 on Cent OS 7 thanks to devtoolset-7. More details at this [link](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-7/). Don't forget to add the following line in your .bashrc:

```
#---------------------------
# enable recent gcc (C++14 default)
#---------------------------

# gcc (GCC) 7.3.1 20180303 (Red Hat 7.3.1-5)
source scl_source enable devtoolset-7
```

Then start a new shell to have gcc 7.

#### (b) Recent CMake

The minimal version is 3.4.3.

#### (c) Recent ROOT

Old versions of [ROOT](https://root.cern.ch/), i.e. prior to ROOT 6, are not supported. 
6.14/04
Linking against ROOT was successfully tested for ROOT version 6.14/04. If ROOT was installed into a custom directory (i.e. not via yum), make sure to have your $ROOTSYS shell environment variable pointing to your working ROOT distribution, as it is used by CMake to prepare the Makefile needed to perform the compilation.

### Installing

Make sure that your cmake executable points to a CMake version >= 3, otherwise use cmake3 instead of cmake in the following lines.

Generate the Makefile with CMake:

```
$ cd new-alpide-software
$ mkdir build
$ cd build
$ cmake ../framework/
```

Then compile (replace N by the number of processors on your computer):

```
$ make -jN
```

## Running the tests on a single device

### Available tests

So far, the avalaible tests for a single device (1 ladder or 1 chip read by 1 MOSAIC board) are:
* Test the communication with the MOSAIC board (main_mosaic.cpp)
* FIFO scan (main_fifo.cpp)
* Digital scan (main_digitalscan.cpp)
* Threshold scan (main_thresholdscan.cpp)
* Noise occupancy scan, with internal or external trigger (main_noiseocc.cpp)

The following test does not involve any device at all (no ladder or chip, no MOSAIC board). Indeed, its sole purpose is to check that the linking of the sofware against ROOT librairies is working properly.
* ROOT test (main_roottest.cpp)

Each main*.cpp file has a few comments explaining how to run the test. 

### Configuration files

They can be found at:

```
$ ls new-alpide-software/framework/config
```

* If you use the MOSAIC board, make sure its IP address matches the one in the relevant configuration file. 
* There is one configuration file per type of device and per type of test (i.e. one for a single chip in IB mode for FIFO scan, one for a MFT ladder for FIFO scan, one for MFT ladder for digital scan, etc). 
* Don't forget to adapt the *DEVICE* name in the configutation file with the length of your HIC. For e.g., for a MFT ladder made with a FPC that can hold 3 chips, the device name in the configuration file is *MFT3*. 
* Don't forget to adapt the DAC settings with your back-bias if you are using the analog part of the ALPIDE chips. _BB3_ in the name of the configuration file indicates that the DAC settings are the one appropriate to operate the chips with a back-bias voltage of -3V.
* To change from internal to external trigger for the noise occupancy scan, just use the test_noiseocc with a different configuration file (with _ext_ in the name of the configuration file).

### Example of digital scan on a MFT ladder

Assuming your MOSAIC is seen by your computer, and the device (here it is hic 35, an MFT ladder with 2 chips) is properly powered and connected to the MOSAIC board, execute the following command on your terminal:

```
$ cd new-alpide-software/framework/bin
$ ./test_digitalscan -v 3 -c ../config/ConfigMFTladder_DigitalScan.cfg -l 35
```

To get a short help on the meaning of the arguments, just execute:

```
$ cd new-alpide-software/framework/bin
./test_digitalscan -h
```

## Running the tests simultaneously and synchronously on multiple devices

### Available tests

So far, the avalaible tests for multiple devices (many ladders or chips, each being read by 1 MOSAIC board) are:
* Digital scan (multi_digitalscan.cpp)
* Noise occupancy scan, with internal trigger, no back-bias (multi_noiseocc_int.cpp)
* Noise occupancy scan, with internal trigger, with back-bias voltage -3V (multi_noiseocc_int_BB3.cpp)
* Noise occupancy scan, with external trigger, no back-bias (multi_noiseocc_ext.cpp)
* Noise occupancy scan, with external trigger, with back-bias voltage -3V (multi_noiseocc_ext_BB3.cpp)

### Minimum version of the MOSAIC firmware

Multi-MOSAIC acquisition works with one board being Master, and the others being Slave. For more details on these settings, see this [link](https://twiki.cern.ch/twiki/bin/view/ALICE/MftFirstTestBeamUtilities#Hardware_how_to_set_the_MOSAIC_b).

The Master-Slave operating modes are available in the firmware version starting from 2018.06.05. More details at this [link](https://twiki.cern.ch/twiki/bin/view/ALICE/MftFirstTestBeamUtilities#Version_of_the_MOSAIC_firmware).

### Configuration files

They can be found at:

```
$ ls new-alpide-software/framework/config/multiboard/
```

### Example of digital scan running synchronously on multiple devices

For each type of test, there is one configuration file per device to be read by a MOSAIC. 

For e.g. a digital scan, here, there are 4 configuration files:
* 1 file for a telescope of chips (read by the Master MOSAIC)
* 3 files for each ladder (each one read by a Slave MOSAIC).

Be careful to:
* write the correct the IP address of the MOSAIC board in each configuration file
* write the id of the device (after the keyword *DEVICEID*) otherwise the default id = 0 will be used to name output files
* check that the *MASTERSLAVEMODE* is the correct one for the MOSAIC board chosen.

In this example, all the 4 configuration files are explicitely used in multi_digitalscan.cpp. As a consequence, it has to be recompiled each time there is any change in the name of the configuration files used. To run the test, execute the following command on your terminal:

```
$ cd new-alpide-software/framework/bin
$ ./test_multi_digitalscan
```


## Versioning

* We use Cern GitLab for versioning, with the repository at https://gitlab.cern.ch/alice-MFT-upgrade/new-alpide-software
* We choose follow the **Git workflow** named *Hg* flow in SourceTree. **If you have no idea what this is**, before doing any modification to the _mft-dev_ branch, please have a look at the philosophy of this workflow nicely summarised at https://blog.sourcetreeapp.com/2012/08/01/smart-branching-with-sourcetree-and-git-flow/

## Acknowledgments

Thanks to the ITS team for providing the original code.
