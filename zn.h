#ifndef ZN_H
#define ZN_H

#include "mbed.h"

#define ZN_RESPONSE_EMPTY 0
#define ZN_RESPONSE_OK 1
#define ZN_RESPONSE_TIMEOUT 2
#define ZN_RESPONSE_INVALID 3
#define ZN_RESPONSE_UNKNOWN_ERROR 4
#define ZN_RESPONSE_CHECKSUM_ERROR 5

class zn {
	public:
		zn(mbed::Serial* uartInit, int initId);
		char id;
		double responseTimeout;

		int ping(char pingId);
		void list(bool deviceArray[]);
		int status(char statusId, char* chksumErrCnt, char* timeoutCnt);
		int inputs(char inputsId, char* inputState);
		void sendBadChecksum();
		
		Serial* debugInterface;
		bool debugEnable;

	private:
		Serial* uart;
		void serialAbort();
		char checksum(char* data, int dataLength);
		bool attemptRx(char* buffer, int rxLength);
};

#endif