#include "gimx_api.h"
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <thread>
//#include <boost/thread.hpp>

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

void setAimSpeed(int x, int y, int shoot) {
	x = clamp(x, -L - 1, L);
	y = clamp(y, -L - 1, L);
//    printf(">>>%s\n",cmd);
#ifdef _WIN32
	sprintf_s(cmd,
			"START /B gimx --event \"rel_axis_2(%d)\" --event \"rel_axis_3(%d)\" --event \"abs_axis_14(%d)\" --event \"abs_axis_13(255)\" --dst %s:51914 > NUL 2> NUL",
			x, y, shoot, SERVER_IP);
#else
	sprintf_s(cmd,
			"gimx --event \"rel_axis_2(%d)\" --event \"rel_axis_3(%d)\" --event \"abs_axis_14(%d)\" --dst %s:51914 & > /dev/null",
			x, y, shoot, SERVER_IP);
#endif
	int nada = system(cmd);
}

void startGimxServer() {
	int nada = system("START /B gimx -c exp_config.xml -p COM3 --src 127.0.0.1:51914 --refresh 5");
	std::this_thread::sleep_for(std::chrono::seconds(8));
	system("gimx --event \"abs_axis_2(255)\" --dst 127.0.0.1:51914");
	std::this_thread::sleep_for(std::chrono::seconds(1));
	system("gimx --event \"abs_axis_2(0)\" --dst 127.0.0.1:51914");
}
