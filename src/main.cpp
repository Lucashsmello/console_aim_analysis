#include <iostream>
#include "aim_analysis_core.h"
#include "utils/utils.h"
#include "Acquisition.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include "aim_analysis/AimData.hpp"
#include "gimx_api.h"
#include <thread>
#include <chrono>

using namespace cv;
using namespace std;
using namespace aim_analysis;

int main(int argc, char** argv) {
	AimData aimdata("aimdata.csv");
	initializeGIMXConnetion();

//	pressButton(13,true);
//	this_thread::sleep_for(chrono::milliseconds(1000));

//	double angspeed = findXAimSpeed(40, 0);
//	cout << "pivot_ang_speed:" << angspeed << endl;
//	Acquisition acq(PIVOT_VIDEO_NAME, angspeed);

	int bx = 38, ex = 55;
	int by = 0, ey = 0;
	bool fast_init = false;

	try {
		for (int y = by; y <= ey; y++) {
			for (int x = bx; x <= ex; x++) {
				if (aimdata.hasXSpeed(x, y))
					continue;
//				double angspeed = findXAimSpeed(x, y, acq, false, false); // esse metodo so funciona se o video do pivot eh relativamente pequeno???
				double angspeed = findXAimSpeed(x, y);
				aimdata.saveResult(x, y, angspeed);
//				fast_init = true;
			}
		}
	} catch (CharacterOutOfPositionException& ex) {
		cout << ex.what() << endl;
	}

	return 0;
}
