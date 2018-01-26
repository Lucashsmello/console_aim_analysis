#include "gimx_api.h"
#include <stdlib.h>
#include <stdio.h>
//#include <boost/thread.hpp>

static char cmd[250];

static const char* SERVER_IP = "192.168.0.7";

int clamp(short int x, int min, int max) {
	if (x > max) {
		return max;
	}
	return x < min ? min : x;
}

void pressX() {

#ifdef _WIN32
	sprintf_s(cmd, "START /B gimx --event \"abs_axis_9(255)\" --dst %s:51914 > NUL 2> NUL", SERVER_IP);
#else
	sprintf_s(cmd, "gimx --event \"abs_axis_9(255)\" --dst %s:51914 & > /dev/null", SERVER_IP);
#endif

	int nada = system(cmd);
}

static const int L = 127;

void setAimSpeed(int x, int y, int shoot) {
	x = clamp(x, -L - 1, L);
	y = clamp(y, -L - 1, L);
//    printf(">>>%s\n",cmd);
#ifdef _WIN32
	sprintf_s(cmd,
			"START /B gimx --event \"rel_axis_2(%d)\" --event \"rel_axis_3(%d)\" --event \"abs_axis_14(%d)\" --dst %s:51914 > NUL 2> NUL",
			x, y, shoot, SERVER_IP);
#else
	sprintf_s(cmd,
			"gimx --event \"rel_axis_2(%d)\" --event \"rel_axis_3(%d)\" --event \"abs_axis_14(%d)\" --dst %s:51914 & > /dev/null",
			x, y, shoot, SERVER_IP);
#endif
	int nada = system(cmd);
}
