#include "gimx_api.h"
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <winsock2.h>
#include <Ws2tcpip.h>

//#pragma comment(lib,"ws2_32.lib") //Winsock Library
//#define BUFLEN 512  //Max length of buffer
#define PORT 51914   //The port on which to listen for incoming data

static char cmd[250];

static const char* SERVER_IP = "192.168.0.8";
//static const char* SERVER_IP = "127.0.0.1";

int clamp(short int x, int min, int max) {
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

static const int L = 127;

void pressButton(int b_id, bool press) {
	int v = press ? 255 : 0;
#ifdef _WIN32
	sprintf_s(cmd, "START /B gimx --event \"abs_axis_%d(%d)\" --dst %s:51914 > NUL 2> NUL", b_id, v,
			SERVER_IP);
#else
	sprintf_s(cmd,
			"gimx --event \"abs_axis_%d(%d)\" --dst %s:51914 & > /dev/null", b_id,v, SERVER_IP);
#endif
	int nada = system(cmd);
}

static char message[162];
static int32_t* message_data;
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
		printf("Initialising Winsock Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//create socket
	if ((S_CODE = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//setup address structure
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
//	InetPton(AF_INET, SERVER_IP, &si_other.sin_addr.s_addr);
	si_other.sin_addr.s_addr = inet_addr(SERVER_IP);
//	closesocket (S_CODE);
//	WSACleanup();
}

static int sendUDPpacket(int x, int y, int shoot) {

	int slen = sizeof(si_other);

	message_data[2] = x;
	message_data[3] = y;
	message_data[22] = shoot;

//start communication
	if (sendto(S_CODE, message, 162, 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR) {
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	return 0;
}

void setAimSpeed(int x, int y, int shoot) {
	x = clamp(x, -L - 1, L);
	y = clamp(y, -L - 1, L);
//    printf(">>>%s\n",cmd);
#ifdef _WIN32
	sendUDPpacket(x, y, shoot);
#else
	sprintf_s(cmd,
			"gimx --event \"rel_axis_2(%d)\" --event \"rel_axis_3(%d)\" --event \"abs_axis_14(%d)\" --dst %s:51914 & > /dev/null",
			x, y, shoot, SERVER_IP);
#endif
	int nada = system(cmd);
}

