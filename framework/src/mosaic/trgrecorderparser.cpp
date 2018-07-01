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
#include <sstream>
#include "mboard.h"
#include "mexception.h"
#include "trgrecorderparser.h"
#include "mdictionary.h"

#define PLATFORM_IS_LITTLE_ENDIAN

TrgRecorderParser::TrgRecorderParser() : MDataReceiver(), TVerbosity()
{
	fTrgNum = 0;
	fTrgTime = 0;
	dataReceiverType = kTrgRecorderParser;
}

void TrgRecorderParser::flush()
{
}

uint32_t TrgRecorderParser::buf2uint32(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
	return (*(uint32_t *) buf) & 0xffffffff;
#else
	uint32_t d;

	d = *buf++;
	d |= (*buf++) << 8;
	d |= (*buf++) << 16;
	d |= (*buf++) << 24;

	return d;
#endif
}

uint64_t TrgRecorderParser::buf2uint64(unsigned char *buf)
{
#ifdef PLATFORM_IS_LITTLE_ENDIAN
	return (*(uint64_t *) buf) & 0xffffffffffffffff;
#else
	uint64_t d;

	d = *buf++;
	d |= (*buf++) << 8;
	d |= (*buf++) << 16;
	d |= (*buf++) << 24;
	d |= (*buf++) << 32;
	d |= (*buf++) << 40;
	d |= (*buf++) << 48;
	d |= (*buf++) << 56;

	return d;
#endif
}


// parse the data starting from begin of buffer
long TrgRecorderParser::parse(int numClosed)
{
	unsigned char *dBuffer = (unsigned char*) &dataBuffer[0];
	unsigned char *p = dBuffer;
	long evSize = (long)MosaicIPbus::TRIGGERDATA_SIZE;

	// check avalaible data size
	if (dataBufferUsed < numClosed * evSize){
		std::stringstream sstm;
		sstm << "TrgRecorderParser::parse() - Parser called with " << numClosed << " closed events of " <<
							evSize << " bytes but datasize is only " << 
							dataBufferUsed << " bytes";
		throw MDataParserError(sstm.str());
	}

	while (numClosed) {
		fTrgNum = buf2uint32(p);	
		fTrgTime = buf2uint64(p + 4);

		if ( GetVerboseLevel() > TVerbosity::kTERSE )
			printf("TrgRecorderParser::parse() - Trigger %d @ %ld\n", fTrgNum, fTrgTime);

		p += evSize;
		numClosed--;
	}

	return(p - dBuffer);
}

long TrgRecorderParser::ReadEventData(int &nBytes, unsigned char *buffer)
{
	unsigned char *dBuffer = (unsigned char*) &dataBuffer[0];
	unsigned char *p = dBuffer;
	long evSize = (long)MosaicIPbus::TRIGGERDATA_SIZE;
    nBytes = (int)MosaicIPbus::HEADER_SIZE + evSize;

	// check avalaible data size
	if (dataBufferUsed < evSize){
		std::stringstream sstm;
		sstm << "TrgRecorderParser::ReadEventData() - called for 1 closed event of " <<
							evSize << " bytes but datasize is only " << 
							dataBufferUsed << " bytes";
		throw MDataParserError(sstm.str());
	}

	if (numClosedData == 0)	
		return 0;

	fTrgNum = buf2uint32(p);	
	fTrgTime = buf2uint64(p + 4);

	if ( GetVerboseLevel() > TVerbosity::kULTRACHATTY )
		printf("TrgRecorderParser::ReadEventData() - Trigger %d @ %ld\n", fTrgNum, fTrgTime);

	// copy the block header to the user buffer
	memcpy(buffer, blockHeader, (int)MosaicIPbus::HEADER_SIZE);

	// copy data to user buffer
    memcpy(buffer + (int)MosaicIPbus::HEADER_SIZE, dBuffer, evSize);

	// move unused bytes to the begin of buffer
	size_t bytesToMove = dataBufferUsed - evSize;
	if (bytesToMove > 0)
		memmove(&dataBuffer[0], &dataBuffer[evSize], bytesToMove);

	dataBufferUsed -= evSize;
	numClosedData--;
	//return evSize;
	return MosaicDict::kTRGRECORDER_EVENT;
}
