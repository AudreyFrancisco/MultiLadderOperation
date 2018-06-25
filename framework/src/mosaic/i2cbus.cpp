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
#include <stdio.h>
#include <stdlib.h>
#include "mexception.h"
#include "i2cbus.h"

I2Cbus::I2Cbus(WishboneBus *wbbPtr, uint32_t baseAdd) : 
			MWbbSlave(wbbPtr, baseAdd)
{
}

//
//	Initial fase of I2C transaction:
//	Start, slave address, R/Wn
//
void I2Cbus::addAddress(uint8_t address, MosaicReadWriteN rw)
{
	// add the slave address with Start condition
	wbb->addWrite(baseAddress + regWriteAdd,
					(address << 1) | 
					((rw == MosaicReadWriteN::I2C_Write) ? I2C_WRITE_BIT : I2C_READ_BIT) | 
					(I2C_START_BIT)
					);	
}


//
//	Write data to the slave
//
void I2Cbus::addWriteData(uint8_t d, uint32_t flags)
{
	// add the data to write
	wbb->addWrite(baseAddress + regWriteAdd,
					(d) | 
					((flags & MosaicDict::instance().readWriteFlags(MosaicReadWriteFlags::RWF_stop)) ? I2C_STOP_BIT : 0)
					);	
}

//
//	Read data from slave
//
void I2Cbus::addRead(uint32_t *d, uint32_t flags)
{
	// add the data to read
	wbb->addWrite(baseAddress + regWriteAdd,
					(0xff) | 
					((flags & MosaicDict::instance().readWriteFlags(MosaicReadWriteFlags::RWF_start))   ? I2C_START_BIT : 0) |
					((flags & MosaicDict::instance().readWriteFlags(MosaicReadWriteFlags::RWF_stop))    ? I2C_STOP_BIT  : 0) |
					((flags & MosaicDict::instance().readWriteFlags(MosaicReadWriteFlags::RWF_dontAck)) ? I2C_IGNORE_ACK_BIT : I2C_MASTER_ACK_BIT)
					);	

	wbb->addRead(baseAddress+regReadAdd, d);	
}

//
//	Read parallel input register
//
void I2Cbus::addReadParIn(uint32_t *d)
{
	wbb->addRead(baseAddress + regParInAdd, d);	
}

void I2Cbus::execute()
{
	try {
		MWbbSlave::execute();
	} catch (MIPBusErrorWrite) {
		throw MIPBusErrorWrite("I2Cbus::execute() - Remote bus error in write - No acknowledge from I2C slave?");
	}
}



