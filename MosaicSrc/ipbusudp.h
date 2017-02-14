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

#ifndef IPBUSUDP_H
#define IPBUSUDP_H

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include "ipbus.h"

class IPbusUDP : public IPbus
{
public:
    IPbusUDP(int pktSize=MosaicIPbus::DEFAULT_PACKET_SIZE);
    IPbusUDP(const char *brdName, int port=MosaicIPbus::DEFAULT_UDP_PORT, int pktsize=MosaicIPbus::DEFAULT_PACKET_SIZE);
    ~IPbusUDP();
    void setIPaddress(const char *brdName, int port=MosaicIPbus::DEFAULT_UDP_PORT);
	void execute();

private:
	void testConnection();
	void sockRead();
	void sockWrite();

private:
	int port;
	int sockfd;
	struct sockaddr_in sockAddress;
	int rcvTimoutTime;
    static const int RCV_LONG_TIMEOUT = 2000; // timeout in ms for the first rx datagrams
    static const int RCV_SHORT_TIMEOUT = 100; // timeout in ms for rx datagrams
};

#endif // IPBUSUDP_H
