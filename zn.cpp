#include "mbed.h"
#include "zn.h"

#define HIGHEST_ADDRESS 0x20

zn::zn(mbed::Serial* uartInit, int initId) {
	uart = uartInit;
	zn::id = initId;
	responseTimeout = 0.5;
	debugEnable = 0;
}

void zn::serialAbort() { uart->abort_read(); }

char zn::checksum(char* data, int dataLength) {
	char retVal = 0;
	for(int i = 0; i < dataLength; i++) {
		retVal ^= data[i];
	}
	return retVal;
}

bool zn::attemptRx(char* attemptRxBuf, int rxLength) {
	//char attemptRxBuf[32];
	char attemptRxChar;
	int rxIndex = 0;
	bool attemptTimeout = true;

	Timer t;
	t.start();

	Timeout to;
	to.attach(callback(this, &zn::serialAbort), (responseTimeout+0.1)); // in case we get stuck at a getc()

	while(t.read() < responseTimeout) {
		if(uart->readable()) {
			attemptRxChar = uart->getc();
			attemptRxBuf[rxIndex] = attemptRxChar;
			rxIndex++;

			if(rxIndex > (rxLength-1)) { 
				attemptTimeout = false;
				break;
			}
		}
	}

	t.stop();

	return !attemptTimeout;
}

int zn::ping(char pingId) {
	char responsePacket[16] = {};

	// clear the serial buffer first
	//char char1 = 0; 
	while (uart->readable()) { 
		char char1 = uart->getc(); 
		char1 += 0;
	}

	char pingPacket[6] = { pingId, zn::id, 0x03, 0x00, 0x00, 0x00 };
	pingPacket[5] = checksum(pingPacket, sizeof(pingPacket));

	uart->printf("%c%c%c%c%c%c", pingPacket[0], pingPacket[1], pingPacket[2], pingPacket[3], pingPacket[4], pingPacket[5] );

	if(!attemptRx(responsePacket, 8)) { // terminated due to timeout
		strcpy(responsePacket, "");
		return ZN_RESPONSE_TIMEOUT;
	}

	if(checksum(responsePacket, sizeof(responsePacket)) != 0 ) { 
		if(debugEnable) { debugInterface->printf("\r\nReceived: [ %02x %02x %02x %02x %02x %02x %02x %02x ]\r\n", responsePacket[0], responsePacket[1], responsePacket[2], responsePacket[3], responsePacket[4], responsePacket[5], responsePacket[6], responsePacket[7]); }
		strcpy(responsePacket, "");
		return ZN_RESPONSE_CHECKSUM_ERROR;
	}

	bool responsePacketEmpty = true;
	for(unsigned int i = 0; i < sizeof(responsePacket); i++) {
		if(responsePacket[i] != 0) {
			responsePacketEmpty = false;
			break;
		}
	}

	if(responsePacketEmpty) {
		strcpy(responsePacket, "");
		return ZN_RESPONSE_EMPTY;
	} 
	
	char responseCmp[3] = { zn::id, pingId, 0x05 };
	if(responsePacket[0] == responseCmp[0] && responsePacket[1] == responseCmp[1] && responsePacket[2] == responseCmp[2]) {
		strcpy(responsePacket, "");
		return ZN_RESPONSE_OK;
	} else { 
		if(debugEnable) { debugInterface->printf("\r\nReceived: [ %02x %02x %02x %02x %02x %02x %02x %02x ]\r\n", responsePacket[0], responsePacket[1], responsePacket[2], responsePacket[3], responsePacket[4], responsePacket[5], responsePacket[6], responsePacket[7]); }
		strcpy(responsePacket, "");
		return ZN_RESPONSE_INVALID;
	}

	strcpy(responsePacket, "");
	return ZN_RESPONSE_UNKNOWN_ERROR;
}

int zn::status(char statusId, char* chksumErrCnt, char* timeoutCnt) {
	char responsePacket[16] = {};

	// clear the serial buffer first
	//char char1 = 0; 
	while (uart->readable()) { 
		char char1 = uart->getc(); 
		char1 += 0;
	}

	char statusTxPacket[6] = { statusId, zn::id, 0x03, 0x00, 0x01, 0x00 };
	statusTxPacket[5] = checksum(statusTxPacket, sizeof(statusTxPacket));

	uart->printf("%c%c%c%c%c%c", statusTxPacket[0], statusTxPacket[1], statusTxPacket[2], statusTxPacket[3], statusTxPacket[4], statusTxPacket[5] );

	if(!attemptRx(responsePacket, 6)) { // false return means terminated due to timeout
		strcpy(responsePacket, "");
		return ZN_RESPONSE_TIMEOUT;
	}

	if(checksum(responsePacket, sizeof(responsePacket)) != 0 ) { // checksum error
		if(debugEnable) { debugInterface->printf("\r\nReceived: [ %02x %02x %02x %02x %02x %02x ]\r\n", responsePacket[0], responsePacket[1], responsePacket[2], responsePacket[3], responsePacket[4], responsePacket[5] ); }
		strcpy(responsePacket, "");
		return ZN_RESPONSE_CHECKSUM_ERROR;
	}

	bool responsePacketEmpty = true;
	for(unsigned int i = 0; i < sizeof(responsePacket); i++) {
		if(responsePacket[i] != 0) {
			responsePacketEmpty = false;
			break;
		}
	}

	if(responsePacketEmpty) { // check for empty packet
		strcpy(responsePacket, "");
		return ZN_RESPONSE_EMPTY;
	} 
	
	char responseCmp[3] = { zn::id, statusId, 0x03 }; // expected response
	if(responsePacket[0] == responseCmp[0] && responsePacket[1] == responseCmp[1] && responsePacket[2] == responseCmp[2]) {
		*chksumErrCnt = responsePacket[3];
		*timeoutCnt = responsePacket[4];
		strcpy(responsePacket, "");
		return ZN_RESPONSE_OK;
	} else { 
		if(debugEnable) { debugInterface->printf("\r\nReceived: [ %02x %02x %02x %02x %02x %02x %02x %02x ]\r\n", responsePacket[0], responsePacket[1], responsePacket[2], responsePacket[3], responsePacket[4], responsePacket[5], responsePacket[6], responsePacket[7]); }
		strcpy(responsePacket, "");
		return ZN_RESPONSE_INVALID;
	}

	strcpy(responsePacket, "");
	return ZN_RESPONSE_UNKNOWN_ERROR;
}

void zn::list(bool deviceArray[]) {
	for(int i = 1; i < HIGHEST_ADDRESS; i++) {
		if(ping(i) == ZN_RESPONSE_OK) { deviceArray[i] = true; }
		else { deviceArray[i] = false; }
	}
}

void zn::sendBadChecksum() {
	uart->printf("%c%c%c%c%c%c", 0x02, 0x00, 0x03, 0x01, 0x02, 0xbe );
}