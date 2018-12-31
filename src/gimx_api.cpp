#include "gimx_api.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <boost/thread.hpp>
#include "utils/utils.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

#define MAX_AXES_REPORT 6 //max number of axes to report.

typedef enum {
	E_NETWORK_PACKET_CONTROLLER, E_NETWORK_PACKET_IN_REPORT,
} e_network_packet_type;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct {
	uint8_t index; // 1 MSB: 0 = relative axis, 1 = absolute axis, 7 LSB = index
	uint32_t value; // network byte order
} message_data_gimx;
#pragma pack(pop)   /* restore original alignment from stack */

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct {
	uint8_t packet_type; // E_NETWORK_PACKET_IN_REPORT
	uint8_t nbAxes; // number of elements in the following array
	message_data_gimx axes[MAX_AXES_REPORT];
} message_report_gimx;
#pragma pack(pop)   /* restore original alignment from stack */

#define MSG_DONTWAIT 0

#ifndef __MINGW32__
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#endif
//#define BUFLEN 512  //Max length of buffer
#define PORT 51914   //The port on which to listen for incoming data

static char cmd[250];

//static const char* SERVER_IP = "127.0.0.1";
static const char* SERVER_IP = "192.168.1.100";

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

static message_report_gimx message;

#ifdef _MSC_VER
static SOCKET S_CODE = SOCKET_ERROR;
#else
static int S_CODE = SOCKET_ERROR;
#endif

void initializeGIMXConnetion() {
	WSADATA wsa;
	struct sockaddr_in si_other;
	memset(message.axes, 0, sizeof(message.axes));

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

	if (connect(S_CODE, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) {
		printf("connect() failed with error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	int iTimeout = 15;
	if (setsockopt(S_CODE, SOL_SOCKET, SO_RCVTIMEO, (const char *) &iTimeout, sizeof(iTimeout))
			!= 0) {
		printf("ERROR in initializeGIMXConnetion(): Could not set timeout for udp requests\n");
	}

//	closesocket (S_CODE);
//	WSACleanup();
}

static int sendUDPpacket(int x, int y, int shoot, unsigned int* poolTime_OUT) {
	if (S_CODE == SOCKET_ERROR) {
		printf(
				"ERROR in sendUDPpacket(): initializeGIMXConnetion() was not called or an error occurred!\n");
		exit(EXIT_FAILURE);
	}
	unsigned char buf[3];
	if (poolTime_OUT == NULL) {
		message.packet_type = 0x01;
	} else {
		message.packet_type = 0xF7;
	}
	message.nbAxes = 3;
	message.axes[0].index = 2;
	message.axes[0].value = htonl(x);
	message.axes[1].index = 3;
	message.axes[1].value = htonl(y);
	message.axes[2].index = 142;
	message.axes[2].value = htonl(shoot);

//	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

//start communication
	if (send(S_CODE, (char*) &message,
			sizeof(message.nbAxes) + sizeof(message.packet_type)
					+ sizeof(message_data_gimx) * message.nbAxes,
			MSG_DONTWAIT) == SOCKET_ERROR) {
		printf("sendto() failed with error code : %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

//	memset(buf, '\0', BUFLEN);
//	//try to receive some data, this is a blocking call
	if (poolTime_OUT != NULL) {
		if (recv(S_CODE, (char*) buf, 3, 0) == SOCKET_ERROR) {
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		poolTime_OUT[0] = (((unsigned int) buf[2]) << 8) + buf[1];
	}

	return 0;
}

static const int L = 127;
void setAimSpeed(int x, int y, int shoot, unsigned int* pollTime_usec_OUT) {
//	boost::thread T = boost::thread(boost::bind(&sendAimSpeed, x, y, shoot));
	x = clamp(x, -L - 1, L);
	y = clamp(y, -L - 1, L);
#ifdef _WIN32
	sendUDPpacket(x, y, shoot, pollTime_usec_OUT);
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
