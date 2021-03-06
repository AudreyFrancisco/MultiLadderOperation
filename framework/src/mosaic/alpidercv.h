/*
 * Copyright (C) 2014
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
 * Written by Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it>, 2014.
 *
 */

#ifndef ALPIDERCV_H
#define ALPIDERCV_H

#include <stdint.h>
#include "mwbbslave.h"



class ALPIDErcv: public MWbbSlave
{
public:
    ALPIDErcv();
    ALPIDErcv(WishboneBus *wbbPtr, uint32_t baseAddress);
	~ALPIDErcv();
	void setBusAddress(WishboneBus *wbbPtr, uint32_t baseAddress);
	void addSetReg(uint16_t address, uint16_t val);
	void addGetReg(uint16_t address, uint32_t *val);
	void addEnable(bool d);
	void addInvertInput(bool d);
	void addSetRDPReg(uint16_t address, uint16_t val);
	void addGetRDPReg(uint16_t address, uint32_t *val);
	void addSetRDPRegField(uint16_t address, uint16_t size, uint16_t offset, uint16_t val);

private:					// WBB Slave registers map 
	enum regAddress_e {
		regOpMode		= 0,
		rdpBase			= 0x00800000
		};

private:
	enum opModeBits_e {
		OPMODE_RCVENABLE			= (1<<0),
		OPMODE_INVERT_POLARITY		= (1<<1)
	};
};



#endif // ALPIDERCV_H
