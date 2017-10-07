#include "mbed.h"
#include "zn.h"

#define HIGHEST_ADDRESS 0x20

zn::zn(mbed::Serial* uartInit, int initId) {
	uart = uartInit;
	zn::id = initId;
	responseTimeout = 0.5;
}

void zn::serialAbort() { uart->abort_read(); }

int zn::ping(char pingId) {
	bool gotPong = false;
	char pingRxBuf[32];
	char pingRxChar;
	int rxIndex = 0;
	bool timeout = true;

	// clear the serial buffer first
	//char char1 = 0; 
	while (uart->readable()) { 
		char char1 = uart->getc(); 
		char1 += 0;
	}

	char pingPacket[5] = { pingId, zn::id, 0x02, 0x00, 0x00 };

	uart->printf("%c%c%c%c%c", pingPacket[0], pingPacket[1], pingPacket[2], pingPacket[3], pingPacket[4] );

	Timer t;
	t.start();

	Timeout to;
	to.attach(callback(this, &zn::serialAbort), (responseTimeout+0.1));

	while(t.read() < responseTimeout) {
		if(uart->readable()) {
			pingRxChar = uart->getc();
			pingRxBuf[rxIndex] = pingRxChar;
			rxIndex++;

			if(rxIndex > 6) { 
				timeout = false;
				break;
			}
		}
	}

	t.stop();

	if(timeout) { // terminated due to timeout
		strcpy(pingRxBuf, "");
		return PING_RESPONSE_TIMEOUT;
	}

	char responseCmp[3] = { zn::id, pingId, 0x04 };
	if(pingRxBuf[0] == responseCmp[0] && pingRxBuf[1] == responseCmp[1] && pingRxBuf[2] == responseCmp[2]) { gotPong = true; }
	else if(strlen(pingRxBuf) < 1) {
		strcpy(pingRxBuf, "");
		return PING_RESPONSE_EMPTY;
	} else { 
		strcpy(pingRxBuf, "");
		return PING_RESPONSE_INVALID;
	}
	
	if(gotPong) {
		strcpy(pingRxBuf, "");
		return PING_RESPONSE_OK;
	}

	strcpy(pingRxBuf, "");
	return PING_RESPONSE_UNKNOWN_ERROR;
}

void zn::list(bool deviceArray[]) {
	for(int i = 1; i < HIGHEST_ADDRESS; i++) {
		if(ping(i) == PING_RESPONSE_OK) { deviceArray[i] = true; }
		else { deviceArray[i] = false; }
	}
}