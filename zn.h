#ifndef ZN_H
#define ZN_H

#include "mbed.h"

#define PING_RESPONSE_EMPTY 0
#define PING_RESPONSE_OK 1
#define PING_RESPONSE_TIMEOUT 2
#define PING_RESPONSE_INVALID 3
#define PING_RESPONSE_UNKNOWN_ERROR 4
#define PING_RESPONSE_CHECKSUM_ERROR 5

class zn {
	public:
		zn(mbed::Serial* uartInit, int initId);
		char id;
		int ping(char pingId);
		void list(bool deviceArray[]);
		double responseTimeout;
		Serial* debugInterface;
		bool debugEnable;

	private:
		Serial* uart;
		void serialAbort();
		char checksum(char* data, int dataLength);
		bool attemptRx(char* buffer, int rxLength);
};

#endif