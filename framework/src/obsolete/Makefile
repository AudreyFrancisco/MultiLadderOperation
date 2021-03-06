#### for Unix/Linux, uncomment and use the next two lines
# CC=g++
# CFLAGS= -O2 -pipe -fPIC -Wno-unknown-pragmas -pthread -g -Wall -W -Woverloaded-virtual -std=c++14 -m64 -mcmodel=large -I/usr/local/include
### for MacOsX, you can use clang instead
CC=clang++
CFLAGS= -O2 -pipe -fPIC -Wno-unknown-pragmas -pthread -g -Wall -W -Woverloaded-virtual -stdlib=libc++ -std=c++14 -m64 -I/usr/local/include
###
LIBPATH=/usr/local/lib
LINKFLAGS=-lusb-1.0 -lpthread -L $(LIBPATH)
OBJECT= runTest
LIBRARY=libalpide.so
CLASS= USB.cpp \
           TAlpide.cpp \
           TAlpideDecoder.cpp \
           TBoardDecoder.cpp \
           TBoardConfig.cpp \
           TBoardConfigDAQ.cpp \
           TBoardConfigMOSAIC.cpp \
           TChipConfig.cpp \
           TDevice.cpp \
           TDeviceBuilder.cpp \
           TDeviceBuilderWithSlaveChips.cpp \
           TDeviceBuilderWithDAQBoards.cpp \
           TDeviceBuilderHalfStave.cpp \
           TDeviceBuilderIB.cpp \
           TDeviceBuilderIBSingleMosaic.cpp \
           TDeviceBuilderMFTLadder.cpp \
           TDeviceBuilderOB.cpp \
           TDeviceBuilderOBSingleDAQ.cpp \
           TDeviceBuilderOBSingleMosaic.cpp \
           TDeviceBuilderTelescopeDAQ.cpp \
           TDeviceChipVisitor.cpp \
           TDeviceFifoTest.cpp \
           TDeviceDigitalScan.cpp \
           THisto.cpp \
           TPixHit.cpp \
           TReadoutBoard.cpp \
           TReadoutBoardDAQ.cpp \
           TReadoutBoardMOSAIC.cpp  \
           TScan.cpp \
           TScanAnalysis.cpp \
           TScanConfig.cpp \
           TSetup.cpp \
           TThresholdScan.cpp \
           TVerbosity.cpp \
           MosaicSrc/alpidercv.cpp \
           MosaicSrc/controlinterface.cpp \
           MosaicSrc/i2cbus.cpp \
           MosaicSrc/i2cslave.cpp \
           MosaicSrc/i2csyspll.cpp \
           MosaicSrc/ipbus.cpp \
           MosaicSrc/ipbusudp.cpp \
           MosaicSrc/mdatagenerator.cpp \
           MosaicSrc/mdatareceiver.cpp \
           MosaicSrc/mdatasave.cpp \
           MosaicSrc/mexception.cpp \
           MosaicSrc/mruncontrol.cpp \
           MosaicSrc/mtriggercontrol.cpp \
           MosaicSrc/mwbbslave.cpp \
           MosaicSrc/pexception.cpp \
           MosaicSrc/pulser.cpp \
           MosaicSrc/mboard.cpp \
           MosaicSrc/TAlpideDataParser.cpp
OBJS = $(CLASS:.cpp=.o)
$(info OBJS="$(OBJS)")

#all:    test_mosaic test_fifo test_digital test_threshold test_noiseocc test_dacscan test_pulselength test_source test_poweron test_noiseocc_ext test_scantest test_temperature
all:    test_mosaic test_fifo test_digital

$(OBJECT):   $(OBJS) main.cpp
	$(CC) -o $(OBJECT) $(OBJS) $(CFLAGS) main.cpp $(LINKFLAGS)

lib: $(OBJS)
	$(CC) -shared $(OBJS) $(CFLAGS) $(LINKFLAGS) -o $(LIBRARY)

test_mosaic:   $(OBJS) main_mosaic.cpp
	$(CC) -o test_mosaic $(OBJS) $(CFLAGS) main_mosaic.cpp $(LINKFLAGS)

test_fifo:   $(OBJS) main_fifo.cpp
	$(CC) -o test_fifo $(OBJS) $(CFLAGS) main_fifo.cpp $(LINKFLAGS)

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
	rm -rf *.o $(OBJECT) test_mosaic test_fifo test_digital test_noiseocc test_threshold test_dacscan test_pulselength test_source test_poweron test_noiseocc_ext test_scantest test_temperature
	rm -rf MosaicSrc/*.o
