#include "mbed.h"
#include "zn.h"

#define RXBUFSIZE 64
#define MYID 0x00

Serial pc(USBTX, USBRX);
DigitalOut led(LED1);
Serial znSerial(PA_9, PA_10, 19200);
zn z(&znSerial, MYID);


int asciiHexToInt(char a, char b) {
	if(a >= '0' && a <= '9') {
		a = a - '0';
	} else if(a >= 'a' && a <= 'f') {
		a = a + 10 - 'a';
	} else if(a >= 'A' && a <= 'F') {
		a = a + 10 - 'A';
	} else {
		return -1;
	}

	if(b >= '0' && b <= '9') {
		b = b - '0';
	} else if(b >= 'a' && b <= 'f') {
		b = b + 10 - 'a';
	} else if(b >= 'A' && b <= 'F') {
		b = b + 10 - 'A';
	} else {
		return -1;
	}

	return (a*16)+b;
}

void cmdList() {
	pc.printf("\r\nChecking for devices (this could take some time)... ");

	bool deviceOnline[0xFE] = {};
	z.list(deviceOnline);

	pc.printf("Done.\r\nOnline devices:");

	int onlineCount = 0;
	for(int i = 1; i < 0xFE; i++) {
		if(deviceOnline[i]) { 
			pc.printf(" 0x%02x", i);
			onlineCount++;
		}
	}

	pc.printf("\r\nCount: %d\r\n", onlineCount);
}

void cmdPing(char* rxBuf) {
	if(rxBuf[4] == ' ' && rxBuf[5] >= '0') { // check for space then 0-9A-F
		int idToPing;
		if(rxBuf[6] >= '0') {
			idToPing = asciiHexToInt(rxBuf[5], rxBuf[6]);
		} else {
			idToPing = asciiHexToInt('0', rxBuf[5]);
		}
		if(idToPing < 0) {
			pc.printf("\r\nInvalid ID: \'%d\'\r\nExamples: \"ping e9\" or \"ping 3\"\r\n", idToPing);
		} else {
			pc.printf("\r\nSending ping to 0x%02x... ", idToPing);

			switch(z.ping(idToPing)) {
				case PING_RESPONSE_EMPTY: {
					pc.printf("Empty Response\r\n"); 
					break;
				} case PING_RESPONSE_OK: {
					pc.printf("Success\r\n"); 
					break;
				} case PING_RESPONSE_TIMEOUT: {
					pc.printf("Timeout\r\n");
					break;
				} case PING_RESPONSE_INVALID: {
					pc.printf("Invalid Response\r\n");
					break;
				} case PING_RESPONSE_CHECKSUM_ERROR: {
					pc.printf("Checksum Error\r\n");
					break;
				} default: {
					pc.printf("Unknown Error\r\n");
				}
			}
		}
	} else {
		pc.printf("\r\nInvalid ID\r\nExamples: \"ping e9\" or \"ping 3\"\r\n");
	}
}

void cmdHelp() {
	pc.printf("\r\nCOMMAND\tDESCRIPTION\r\n\
help	Show this help text\r\n\
list	List online devices\r\n\
ping 	See if node is alive\r\n");
}

// main() runs in its own thread in the OS
int main() {
	
	z.responseTimeout = 0.2;
	pc.printf("\r\n\r\nCompiled: " __DATE__ " " __TIME__);
	led = 1;

	//debugging
	z.debugInterface = &pc;
	z.debugEnable = true;

	while(1) {

		pc.printf("\r\n> ");

		char rxBuf[RXBUFSIZE];
		char rxChar;
		bool error = false;

		rxChar = pc.getc();

		// gather until you get a carriage return
		while(rxChar != '\r') {
			sprintf(rxBuf,"%s%c",rxBuf,rxChar);
			pc.printf("%c", rxChar);

			if(strlen(rxBuf) >= RXBUFSIZE) {
				error = true;
				break;
			}

			rxChar = pc.getc();
		}

		// now process what was typed and do the command
		if(!error) {
			//pc.printf("\r\nI heard: %s\r\n", rxBuf);
			if(!strcmp(rxBuf,"help")) {
				cmdHelp();
			} else if(!strcmp(rxBuf,"list")) {
				cmdList();
			} else if(!strncmp(rxBuf, "ping", 4)) {
				cmdPing(rxBuf);
			} else {
				pc.printf("\r\nError: Unrecognized command \"%s\"\r\n", rxBuf);
				cmdHelp();
			}
		} else {
			pc.printf("\r\nError: Command buffer full\r\n");
		}

		strcpy(rxBuf, "");
	}
}
