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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2017.
 *
 */
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
// MOSAIC includes
#include "mboard.h"
#include "mexception.h"
#include "TAlpideDataParser.h"

using namespace std;

TAlpideDataParser::TAlpideDataParser() : MDataReceiver() , TVerbosity(),
fId( -1 )
{
}


// fast parse of input frame
// return the size of the data for the event pointed by dBuffer. 
long TAlpideDataParser::checkEvent(unsigned char *dBuffer, unsigned char *evFlagsPtr)
{
	unsigned char *p = dBuffer;
	unsigned char h;
    int lastRegion = -1, lastDataField = -1;

	for (int closed=0;!closed;){
		if (p-dBuffer > dataBufferUsed)
			throw MDataParserError("TAlpideDataParser::checkEvent() - Try to parse more bytes than received size");
			
		h = *p++;
		if ( (h>>DSHIFT_CHIP_EMPTY) == DCODE_CHIP_EMPTY ) {
            p++;
            closed=1;
            int d = h&0x0f;
            int fsd = *p;
            if ( GetVerboseLevel() >= kCHATTY ) {
                cout << endl << std::dec << fId << " TAlpideDataParser::checkEvent() - CHIP_EMPTY_FRAME ID:" << d << "  Frame Start Data:" << std::hex << fsd << endl;
            }
            lastRegion = -1;
            lastDataField = -1;
		} else if ( (h>>DSHIFT_CHIP_HEADER) == DCODE_CHIP_HEADER ) {
            p++;
            int d = h&0x0f;
            int fsd = *p;
            if ( GetVerboseLevel() >= kCHATTY ) {
                cout << endl << std::dec << fId << " TAlpideDataParser::checkEvent() - CHIP_HEADER ID:" << d << " frame start data:" << std::hex << fsd << endl;
            }
		} else if ((h>>DSHIFT_CHIP_TRAILER) == DCODE_CHIP_TRAILER ) {
            int d = h&0x0f;
            if ( GetVerboseLevel() >= kCHATTY ) {
                cout << std::dec << fId << " TAlpideDataParser::checkEvent() - CHIP_TRAILER Flags:" << d << endl;
            }
			closed=1;
			// additional trailer
			*evFlagsPtr = *p++;
            uint16_t fsd = *evFlagsPtr;
            if (fsd && (GetVerboseLevel() > kTERSE) ){
                cout << std::dec << fId << " =================== Event with flags != 0 (0x" << std::hex << fsd << ")" << endl;
                if (fsd & 0x01)
                    cout << std::dec << fId << " =================== Header error" << endl;
                if (fsd & 0x02)
                    cout << std::dec << fId << " =================== 10b8b Decoder error" << endl;
            }
		} else if ((h>>DSHIFT_REGION_HEADER) == DCODE_REGION_HEADER ) {
            int d = h&0x1f;
            if ( GetVerboseLevel() >= kCHATTY )
                cout << std::dec << fId << " TAlpideDataParser::checkEvent() - REGION_HEADER Region:" << d << endl;
            if ( (d <= lastRegion) && (GetVerboseLevel() >= kCHATTY) ){
                cout << std::dec << fId << " =================== REGION_HEADER ERROR" << endl;
            }
            lastRegion = d;
            lastDataField = -1;
		} else if ((h>>DSHIFT_DATA_SHORT) == DCODE_DATA_SHORT ) {
			p++;
            uint16_t dShort = ((h&0x3f) << 8) | *p;
            if ( GetVerboseLevel() >= kCHATTY )
                cout << std::dec << fId << "  DATA_SHORT Data:" << std::hex << dShort << endl;
            if ((lastRegion < 0) && (GetVerboseLevel() >= kCHATTY)){
                cout << std::dec << fId << " =================== DATA_SHORT Without Region header" << endl;
                lastRegion = 0;
            }
            if ((dShort < lastDataField) && (GetVerboseLevel() >= kCHATTY)){
                cout << std::dec << fId << " =================== DATA_SHORT ERROR" << endl;
                printf("dShort:%x lastDataField:%x\n", dShort, lastDataField);
            }
            lastDataField = dShort;
		} else if ((h>>DSHIFT_DATA_LONG) == DCODE_DATA_LONG ) {
            //p+=2;
            // TODO: check incrementation done by next 2 lines <=> line above
            uint16_t dShort = ((h&0x3f) << 8) | *p++;
            uint16_t hitMap = *p++;
            if ( GetVerboseLevel() >= kCHATTY ){
                cout << std::dec << fId << "  DATA_LONG  Data:" << std::hex << dShort;
                cout << " hit_map:" << std::hex << hitMap << endl;
            }
            if ((lastRegion < 0) && (GetVerboseLevel() >= kCHATTY)){
                cout << std::dec << fId << " =================== DATA_LONG Without Region header" << endl;
                lastRegion = 0;
            }
            if ((dShort < lastDataField) && (GetVerboseLevel() >= kCHATTY)){
                cout << std::dec << fId << " =================== DATA_LONG ERROR" << endl;
                printf("dShort:%x lastDataField:%x\n", dShort, lastDataField);
            }
            lastDataField = dShort;
		} else {
			int d = h;
			cout << std::dec << fId << " TAlpideDataParser::checkEvent() - Unknow data header: " << std::hex << d << endl;
		}
	}	
	
	return(p-dBuffer);
}

// parse all data starting from begin of buffer
long TAlpideDataParser::parse(int numClosed)
{
	unsigned char *dBuffer = (unsigned char*) &dataBuffer[0];
	unsigned char *p = dBuffer;
	long evSize;
	unsigned char evFlags;

	while (numClosed) {
		evSize = checkEvent(p, &evFlags);
		p += evSize;
		numClosed--;
	}

	return(p-dBuffer);
}

//
// Read only one frame of data 
// return the size of data frame
int  TAlpideDataParser::ReadEventData(int &nBytes, unsigned char *buffer)
{
	unsigned char *dBuffer = (unsigned char*) &dataBuffer[0];
	unsigned char *p = dBuffer;
	long evSize;
	unsigned char evFlags;

	if (numClosedData==0)	
		return 0;

	evSize = checkEvent(p, &evFlags);
	
	// copy the block header to the user buffer
	memcpy(buffer, blockHeader, MosaicIPbus::HEADER_SIZE);

	// copy data to user buffer
    memcpy(buffer+MosaicIPbus::HEADER_SIZE, dBuffer, evSize);
    nBytes = MosaicIPbus::HEADER_SIZE + evSize;

	// move unused bytes to the begin of buffer
	size_t bytesToMove = dataBufferUsed - evSize;
	if (bytesToMove>0)
		memmove(&dataBuffer[0], &dataBuffer[evSize], bytesToMove);
	dataBufferUsed -= evSize;
	numClosedData--;
	return evSize;
}



