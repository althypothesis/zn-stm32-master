#ifndef ZN_H
#define ZN_H

#include "mbed.h"

#define PING_RESPONSE_EMPTY 0
#define PING_RESPONSE_OK 1
#define PING_RESPONSE_TIMEOUT 2
#define PING_RESPONSE_INVALID 3
#define PING_RESPONSE_UNKNOWN_ERROR 4

class zn {
	public:
		zn(mbed::Serial* uartInit, int initId);
		char id;
		int ping(char pingId);
		void list(bool deviceArray[]);
		double responseTimeout;

	private:
		Serial* uart;
		void serialAbort();
};

#endif