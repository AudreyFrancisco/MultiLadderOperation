# MFT fork of ITS new alpide software

The main interest into the ITS new alpide software is the following: it is meant to have ALPIDE test routines or utilities (such as FIFO scan, digital scan, threshold scan ...) that should be usable for any type of readout board (so far DAQ board and MOSAIC board, RU is being included as well, but not yet in this MFT fork). The test utilities will also be the same for different types of device (for e.g. single chip on a carrier board in IB mode, MFT ladder with 2 chips, MFT ladder with 5 chips ...). To be able to do so, the utilities must be fed at run time with a configuration describing how many chips are used, the configuration of each chip, and the type of readout board to be used.

The original ITS repository was forked into our own repository. The code was modified and optimized to match MFT specific needs. The _master_ branch will be regularly kept in sync with the corresponding _master_ branch from the original project. The **default** branch is set to be the _mft_ branch. This default branch is protected, meaning that only users with Master rank or the owner of the repository can commit to this default branch. The _mft-dev_ branch is the one where new features for MFT are being developed and tested. Once these new features are validated, they are merged into the _mft_ branch. The Git workflow to be followed to add new features to _mft-dev_ is outlined in the [Versioning](#versioning) section below.

A Doxygen web-based documentation will be added soon.

## Getting Started

Clone the repository (you will get the default branch, i.e. _mft_):

```
$ git clone ssh://git@gitlab.cern.ch:7999/alice-MFT-upgrade/new-alpide-software.git
```

### Prerequisites

Make sure you have either at least clang version Apple LLVM version 8.1.0 (clang-802.0.42) if you work on MacOS X, or a gcc version with C++14 is enabled by default i.e. at least gcc 6. If you don't have gcc 6, you can easily install it on Cent OS 7 thanks to devtoolset-6. More details at this [link](https://www.softwarecollections.org/en/scls/rhscl/devtoolset-6/). Don't forget to add the following line in your .bashrc:

```
#---------------------------
# enable recent gcc (C++14 default)
#---------------------------

# gcc (GCC) 6.2.1 20160916 (Red Hat 6.2.1-3)
source scl_source enable devtoolset-6
```

Then start a new shell to have gcc 6.

### Installing

Edit the Makefile according to your OS (instructions are in the Makefile). Then:

```
$ make
```

## Running the tests

So far, the avalaible tests are:
* Test the communication with the MOSAIC board (main_mosaic.cpp)
* FIFO scan (main_fifo.cpp)
* Digital scan (main_digital.cpp)

Each main*.cpp file has a few comments explaining how to run the test. If you use the MOSAIC board, make sure its IP address matches the one in the relevant configuration file. There is one configuration file per type of device and per type of test (i.e. one for a single chip in IB mode for FIFO scan, one for a MFT ladder for FIFO scan, one for MFT ladder for digital scan, etc).

### Example of FIFO scan on a MFT ladder

Assuming your MOSAIC is seen by your computer, and the device (an MFT ladder with 2 chips) is properly powered and connected to the MOSAIC board, execute the following command on your terminal:

```
./test_fifo -v 3 -c ConfigMFTladder_FIFOtest.cfg
```

## Versioning

* We use Cern GitLab for versioning, with the repository at https://gitlab.cern.ch/alice-MFT-upgrade/new-alpide-software
* We choose follow the **Git workflow** named *Hg* flow in SourceTree. **If you have no idea what this is**, before doing any modification to the _mft-dev_ branch, please have a look at the philosophy of this workflow nicely summarised at https://blog.sourcetreeapp.com/2012/08/01/smart-branching-with-sourcetree-and-git-flow/

## Acknowledgments

Thanks to the ITS team for providing the original code.
