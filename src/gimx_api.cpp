#include "gimx_api.h"
#include <stdlib.h>
#include <stdio.h>
#include <boost/thread.hpp>
#include "utils/utils.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

#ifndef __MINGW32__
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#endif
//#define BUFLEN 512  //Max length of buffer
#define PORT 51914   //The port on which to listen for incoming data

static char cmd[250];

//static const char* SERVER_IP = "127.0.0.1";
static const char* SERVER_IP = "192.168.0.8";

static int clamp(short int x, int min, int max) {
	if (x > max) {
		return max;
	}
	return x < min ? min : x;
}

void pressX(bool release) {
	int v = release ? 0 : 255;

#ifdef _WIN32
	sprintf_s(cmd, "START /B gimx --event \"abs_axis_9(%d)\" --dst %s:51914 > NUL 2> NUL", v,
			SERVER_IP);
#else
	sprintf_s(cmd, "gimx --event \"abs_axis_9(%d)\" --dst %s:51914 & > /dev/null", v, SERVER_IP);
#endif

	int nada = system(cmd);
}

static char message[162];
static int32_t* message_data = 0;
static struct sockaddr_in si_other;
static int S_CODE;

void initializeGIMXConnetion() {
	WSADATA wsa;
	memset(message, 0, 162 * sizeof(char));
	message[0] = 0xFF;
	message[1] = 0xA0;
	message_data = (int32_t*) (message + 2);

	//Initialise winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Initialising Winsock Failed. Error Code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//create socket
	if ((S_CODE = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
		printf("socket() failed with error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//setup address structure
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
#ifdef __MINGW32__
	si_other.sin_addr.s_addr = inet_addr(SERVER_IP);
#else
	InetPton(AF_INET, SERVER_IP, &si_other.sin_addr.s_addr);
#endif
	int iTimeout = 15;
	int iRet = setsockopt(S_CODE,
	SOL_SOCKET,
	SO_RCVTIMEO, (const char *) &iTimeout, sizeof(iTimeout));
	if (iRet != 0) {
		printf("ERROR in initializeGIMXConnetion(): Could not set timeout for udp requests\n");
	}

//	closesocket (S_CODE);
//	WSACleanup();
}

static int sendUDPpacket(int x, int y, int shoot, unsigned int* poolTime_OUT) {
	if (message_data == 0) {
		printf("ERROR in sendUDPpacket(): initializeGIMXConnetion() was not called!\n");
		exit(EXIT_FAILURE);
	}
	int slen = sizeof(si_other);
	unsigned char buf[3];
	if (poolTime_OUT == NULL) {
		message[0] = 0xFF;
	} else {
		message[0] = 0xF7;
	}

	message_data[2] = x;
	message_data[3] = y;
	message_data[22] = shoot;
	message_data[21] = 255;

//	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

//start communication
	if (sendto(S_CODE, message, 162, 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
		printf("sendto() failed with error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//receive a reply and print it
	//clear the buffer by filling null, it might have previously received data
//	memset(buf, '\0', BUFLEN);
//	//try to receive some data, this is a blocking call
	if (poolTime_OUT != NULL) {
		if (recvfrom(S_CODE, (char*) buf, 3, 0, (struct sockaddr *) &si_other,
				&slen) == SOCKET_ERROR) {
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		poolTime_OUT[0] = (((unsigned int) buf[2]) << 8) + buf[1];
	}
//
	return 0;
}

static const int L = 127;

void setAimSpeed(int x, int y, int shoot, unsigned int* poolTime_usec_OUT) {
//	boost::thread T = boost::thread(boost::bind(&sendAimSpeed, x, y, shoot));
	x = clamp(x, -L - 1, L);
	y = clamp(y, -L - 1, L);
#ifdef _WIN32
	sendUDPpacket(x, y, shoot, poolTime_usec_OUT);
#else
	sprintf_s(cmd,
			"gimx --event \"rel_axis_2(%d)\" --event \"rel_axis_3(%d)\" --event \"abs_axis_14(%d)\" --dst %s:51914 & > /dev/null",
			x, y, shoot, SERVER_IP);
#endif

}

void startGimxServer() {
	int nada = system(
			"START /B gimx -c PS4_Overwatch_Cybereu.xml -p COM7 --src 127.0.0.1:51914 --refresh 5");
}
