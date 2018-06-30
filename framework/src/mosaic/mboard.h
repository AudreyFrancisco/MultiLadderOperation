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

#ifndef MBOARD_H
#define MBOARD_H

#include "i2cbus.h"
#include "i2csyspll.h"
#include "ipbus.h"
#include "ipbusudp.h"
#include "mdatagenerator.h"
#include "mruncontrol.h"
#include "mtriggercontrol.h"
#include "mwbb.h"
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

class MDataReceiver;

class MBoard
{
public:
	MBoard();
    MBoard(const char *IPaddr, const int myBoardId = 0, int UDPport = (int)MosaicIPbus::DEFAULT_UDP_PORT);
    ~MBoard();

	void setIPaddress(const char *IPaddr, int UDPport = (int)MosaicIPbus::DEFAULT_UDP_PORT);
	void setBoardId(const int number = 0);
	void initHardware();
    void connectTCP(int port = (int)MosaicIPbus::DEFAULT_TCP_PORT, int rcvBufferSize = (int)MosaicIPbus::DEFAULT_TCP_BUFFER_SIZE);
	void closeTCP();
	long pollTCP(int timeout, MDataReceiver **dr);
	long pollData(int timeout);
	void addDataReceiver(int id, MDataReceiver *dc);
	void flushDataReceivers();
	static unsigned int buf2ui(unsigned char *buf);
	inline int getBoardId() const { return fBoardId; }
	uint32_t GetTriggerNum() const { return fTrgNum; }
	uint64_t GetTriggerTime() const { return fTrgTime; }
	uint32_t* GetPointerTriggerNum() { return &fTrgNum; }
	uint64_t* GetPointerTriggerTime() { return &fTrgTime; }

public:
	MDataGenerator *mDataGenerator;
	IPbusUDP 		*mIPbus;
	MRunControl 	*mRunControl;
	MTriggerControl *mTriggerControl;
	I2CSysPll		*mSysPLL;

protected:
  	uint32_t fTrgNum;
	uint64_t fTrgTime;

private:
	void init();
	ssize_t recvTCP(void *buffer, size_t count, int timeout);
	ssize_t readTCPData(void *buffer, size_t count, int timeout);
	int fBoardId;

// private:
public:
	int				tcp_sockfd;
	int				numReceivers;
	std::vector<MDataReceiver *> receivers;


public:
	enum dataBlockFlag_e {
		flagClosedEvent			= (1 << 0),
		flagOverflow			= (1 << 1),
		flagTimeout			    = (1 << 2),
		flagCloseRun			= (1 << 3)
		};

    std::string IPaddress;
};

#endif // MBOARD_H
