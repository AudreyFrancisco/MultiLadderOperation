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

#ifndef TRGRECORDERPARSER_H
#define TRGRECORDERPARSER_H

#include "mdatareceiver.h"
#include <stdint.h>
#include "TVerbosity.h"

class TrgRecorderParser : public MDataReceiver, public TVerbosity
{
public:
	TrgRecorderParser();
	void flush();
	uint32_t GetTriggerNum() const { return fTrgNum; }
	uint64_t GetTriggerTime() const { return fTrgTime; }

protected:
	long parse(int numClosed);

	/// value of the trigger counter read with parse of received trigger data
	uint32_t fTrgNum;

	/// value of the trigger time (in unit of clock) read with parse of received trigger data
	uint64_t fTrgTime;

	
private:
	uint32_t buf2uint32(unsigned char *buf);
	uint64_t buf2uint64(unsigned char *buf);
	static const int TRIGGERDATA_SIZE = 12;	// 4 bytes: Trigger number. 8 bytes: Time stamp
};

#endif // TRGRECORDERPARSER_H
