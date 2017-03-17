# for Linux, use g++
#CC=g++
# for MacOsX, you can use clang instead
CC=clang++
#
INCLUDE=/usr/local/include
LIBPATH=/usr/local/lib
#
# for Linux, use option "-mcmodel=large"
#CFLAGS= -O2 -pipe -fPIC -Wno-unknown-pragmas -pthread -g -Wall -W -Woverloaded-virtual -stdlib=libc++ -std=c++11 -m64 -mcmodel=large -I $(INCLUDE)
# for MacOsX, remove option "-mcmodel=large" as it lead to non-usable executables
# after a clumsy linking (see for e.g. the warning below)
#ld: warning: PIE disabled. Absolute addressing (perhaps -mdynamic-no-pic) not allowed in code signed PIE, but used in __ZN5TChipC2Eiii from TChip.o. To fix this warning, don't compile with -mdynamic-no-pic or link with -Wl,-no_pie
CFLAGS= -O2 -pipe -fPIC -Wno-unknown-pragmas -pthread -g -Wall -W -Woverloaded-virtual -stdlib=libc++ -std=c++11 -m64 -I $(INCLUDE)
LINKFLAGS=-lusb-1.0 -lpthread -L $(LIBPATH)
OBJECT= runTest
LIBRARY=libalpide.so
CLASS= TChip.cpp TReadoutBoard.cpp TAlpide.cpp AlpideDecoder.cpp USB.cpp TReadoutBoardDAQ.cpp \
 TReadoutBoardMOSAIC.cpp TChipConfig.cpp TBoardConfig.cpp TBoardConfigDAQ.cpp TBoardConfigMOSAIC.cpp \
 BoardDecoder.cpp TSetup.cpp THisto.cpp TScanAnalysis.cpp\
 MosaicSrc/alpidercv.cpp MosaicSrc/controlinterface.cpp MosaicSrc/i2cbus.cpp MosaicSrc/i2cslave.cpp MosaicSrc/i2csyspll.cpp \
 MosaicSrc/ipbus.cpp MosaicSrc/ipbusudp.cpp MosaicSrc/mdatagenerator.cpp MosaicSrc/mdatareceiver.cpp MosaicSrc/mdatasave.cpp \
 MosaicSrc/mexception.cpp MosaicSrc/mruncontrol.cpp MosaicSrc/mtriggercontrol.cpp MosaicSrc/mwbbslave.cpp \
 MosaicSrc/pexception.cpp MosaicSrc/pulser.cpp MosaicSrc/mboard.cpp MosaicSrc/TAlpideDataParser.cpp \
 TScan.cpp TThresholdScan.cpp TScanConfig.cpp
OBJS = $(CLASS:.cpp=.o)
$(info OBJS="$(OBJS)")

all:    test_mosaic test_noiseocc test_threshold test_digital test_fifo test_dacscan test_pulselength test_source test_poweron test_noiseocc_ext test_scantest test_temperature

$(OBJECT):   $(OBJS) main.cpp
	$(CC) -o $(OBJECT) $(OBJS) $(CFLAGS) main.cpp $(LINKFLAGS)

lib: $(OBJS)
	$(CC) -shared $(OBJS) $(CFLAGS) $(LINKFLAGS) -o $(LIBRARY)

test_mosaic:   $(OBJS) main_mosaic.cpp
	$(CC) -o test_mosaic $(OBJS) $(CFLAGS) main_mosaic.cpp $(LINKFLAGS)

test_noiseocc:   $(OBJS) main_noiseocc.cpp
	$(CC) -o test_noiseocc $(OBJS) $(CFLAGS) main_noiseocc.cpp $(LINKFLAGS)

test_source:   $(OBJS) main_source.cpp
	$(CC) -o test_source $(OBJS) $(CFLAGS) main_source.cpp $(LINKFLAGS)

test_threshold:   $(OBJS) main_threshold.cpp
	$(CC) -o test_threshold $(OBJS) $(CFLAGS) main_threshold.cpp $(LINKFLAGS)

test_threshold_pb:   $(OBJS) main_threshold_pb.cpp
	$(CC) -o test_threshold_pb $(OBJS) $(CFLAGS) main_threshold_pb.cpp $(LINKFLAGS)

test_digital:   $(OBJS) main_digitalscan.cpp
	$(CC) -o test_digital $(OBJS) $(CFLAGS) main_digitalscan.cpp $(LINKFLAGS)

test_fifo:   $(OBJS) main_fifo.cpp
	$(CC) -o test_fifo $(OBJS) $(CFLAGS) main_fifo.cpp $(LINKFLAGS)

test_dacscan:   $(OBJS) main_dacscan.cpp
	$(CC) -o test_dacscan $(OBJS) $(CFLAGS) main_dacscan.cpp $(LINKFLAGS)

test_pulselength:   $(OBJS) main_pulselength.cpp
	$(CC) -o test_pulselength $(OBJS) $(CFLAGS) main_pulselength.cpp $(LINKFLAGS)

test_poweron:   $(OBJS) main_poweron.cpp
	$(CC) -o test_poweron $(OBJS) $(CFLAGS) main_poweron.cpp $(LINKFLAGS)

test_noiseocc_ext:   $(OBJS) main_noiseocc_ext.cpp
	$(CC) -o test_noiseocc_ext $(OBJS) $(CFLAGS) main_noiseocc_ext.cpp $(LINKFLAGS)

test_scantest:   $(OBJS) main_scantest.cpp
	$(CC) -o test_scantest $(OBJS) $(CFLAGS) main_scantest.cpp $(LINKFLAGS)

test_temperature:   $(OBJS) main_temperature.cpp
	$(CC) -o test_temperature $(OBJS) $(CFLAGS) main_temperature.cpp $(LINKFLAGS)

%.o: 	%.cpp %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o $(OBJECT)
	rm -rf MosaicSrc/*.o
