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

#ifndef TALPIDEDATAPARSER_H
#define TALPIDEDATAPARSER_H

#include <stdint.h>
#include "mdatareceiver.h"
#include "TVerbosity.h"

class TAlpideDataParser : public MDataReceiver, public TVerbosity
{
public:
	TAlpideDataParser();
	void flush() {};
	long ReadEventData(int &nBytes, unsigned char *buffer);
    
	inline void SetReceiverId(const int i) { fReceiverId = i; }
	inline void SetBoardId(const int i) { fBoardId = i; }
	void SetPointerTriggerNum( uint32_t* i ) { fTrgNum = i; } 
	void SetPointerTriggerTime( uint64_t* i ) { fTrgTime = i; }  

	uint32_t GetTriggerNum() const { return *fTrgNum; }
	uint64_t GetTriggerTime() const { return *fTrgTime; }
	int GetReceiverId() const { return fReceiverId; }
	int GetBoardId() const { return fBoardId; }

protected:
	long parse(int numClosed);

private:
    int fReceiverId;
	int fBoardId;
	uint32_t* fTrgNum;
	uint64_t* fTrgTime;

	enum dataCode_e {
		DCODE_IDLE			 = 0xff,
		DCODE_CHIP_EMPTY	 = 0x0e,
		DSHIFT_CHIP_EMPTY	 = 4,
		DCODE_REGION_HEADER	 = 0x06,
		DSHIFT_REGION_HEADER = 5,
		DCODE_DATA_SHORT	 = 0x01,			// 16 bits long
		DSHIFT_DATA_SHORT	 = 6,
		DCODE_DATA_LONG		 = 0x00,			// 24 bits long
		DSHIFT_DATA_LONG	 = 6,
		DCODE_CHIP_HEADER	 = 0x0a,			// Chip Header
		DSHIFT_CHIP_HEADER	 = 4,
		DCODE_CHIP_TRAILER	 = 0x0b,			// Chip trailer
		DSHIFT_CHIP_TRAILER	 = 4
	};

private:
	long checkEvent(unsigned char *dBuffer, unsigned char *evFlagsPtr);
	
public:
	enum eventFlag_e {
		flagHeaderError			= (1 << 0),
		flagDecoder10b8bError	= (1 << 1),
    	flagOverSizeError       = (1 << 2)
	};
};

#endif // TALPIDEDATAPARSER_H
