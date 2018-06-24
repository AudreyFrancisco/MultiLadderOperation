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

#ifndef IPBUS_H
#define IPBUS_H

#include "wishbonebus.h"
#include "mdictionary.h"
#include <mutex>
#include <stdint.h>
#include <string>

class IPbusTransaction
{
public:
	IPbusTransaction();
	virtual ~IPbusTransaction();

	inline void SetVersion( const uint8_t aVersion ){ fVersion = aVersion; }
	inline void SetWords( const uint16_t someWords ) { fWords = someWords; }
	inline void SetTypeId( const MosaicIPbusTransaction aTypeId ) { fTypeId = aTypeId; }
	inline void SetTransactionId( const uint8_t aTransactionId ) { fTransactionId = aTransactionId; }
	inline void SetInfoCode( const MosaicIPbusInfoCode anInfoCode ) { fInfoCode = anInfoCode; }
	inline void SetReadDataPtr( uint32_t* aReadDataPtr ) { fReadDataPtr = aReadDataPtr; }

	inline uint8_t GetVersion() const { return fVersion; }
	inline uint16_t GetWords() const { return fWords; }
	inline MosaicIPbusTransaction GetTypeId() const { return fTypeId; }
	inline uint8_t GetTransactionId() const { return fTransactionId; }
	inline MosaicIPbusInfoCode GetInfoCode() const { return fInfoCode; }
	inline uint32_t* GetReadDataPtr() { return fReadDataPtr; }

private:
	uint8_t fVersion;
	uint16_t fWords;
	MosaicIPbusTransaction fTypeId;
	uint8_t fTransactionId;
	MosaicIPbusInfoCode fInfoCode;		
	uint32_t* fReadDataPtr;
};

class IPbus : public WishboneBus
{
public:
    IPbus( int pktSize = (int)MosaicIPbus::DEFAULT_PACKET_SIZE );
    virtual ~IPbus();
	void addIdle();
	void addWrite(uint32_t address, uint32_t data);
	void addWrite(int size, uint32_t address, uint32_t *data);
	void addNIWrite(int size, uint32_t address, uint32_t *data);
	void addRead(int size, uint32_t address, uint32_t *data);
	void addRead(uint32_t address, uint32_t *data) { addRead(1, address, data); }
	void addNIRead(int size, uint32_t address, uint32_t *data);
	void addRMWbits(uint32_t address, uint32_t mask, uint32_t data, uint32_t *rData = NULL);
	void addRMWsum(uint32_t address, uint32_t data, uint32_t *rData = NULL);
	virtual void execute() = 0;
	int  getBufferSize() { return bufferSize; }
	virtual const std::string name() = 0;

	// test functions
  	void addBadIdle(bool sendWrongVersion = false, bool sendWrongInfoCode = false);
  	void setBufferSize(int s) { bufferSize = s; }
  	void cutTX(int size) { txSize -= size; }

protected:
	bool duplicatedRxPkt();
	void processAnswer();
	int  getExpectedRxSize() { return expectedRxSize; }
	
private:
	void chkBuffers(int txTransactionSize, int rxTransactionSize);
	void addWord(uint32_t w);
	uint32_t getWord();
	void addHeader(uint16_t words, MosaicIPbusTransaction typeId, uint32_t *readDataPtr);
	void getHeader(IPbusTransaction *tr);
	void clearList();
	void dumpRxData();
	    
protected:
	uint8_t* txBuffer;
	uint8_t* rxBuffer;
	int txSize;
	int rxSize;
	int errorCode;
	std::recursive_mutex mutex;
	
private:
	int bufferSize;
	IPbusTransaction* transactionList;
	int numTransactions;
	uint8_t transactionId;
	int expectedRxSize;
	int rxPtr;
	int lastRxPktId;
};

#endif // IPBUS_H
