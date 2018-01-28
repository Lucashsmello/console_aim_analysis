#include <iostream>
#include <fstream>
#include "aim_analysis_core.h"
#include "utils/utils.h"
#include "Acquisition.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;
using namespace aim_analysis;

//int main(int argc, char** argv) {
////	for (int i = 0; i <= 127; i++)
////		setAimSpeed(30, 30, 0);
////	return 0;
//	int bx = 20, ex = 50;
//	int by = 20, ey = 50;
//
//	try {
//		ofstream myfile;
//		myfile.open("map2.txt", ios::out | ios::app);
//		for (int i = by; i <= ey; i++) {
//			for (int j = bx; j <= ex; j++) {
//				if (i == 20 and j <= 24)
//					continue;
//				double angspeed = findXAimSpeed(0, j, i);
//				myfile << j << ";" << i << ";" << angspeed << endl;
//			}
//			myfile.flush();
//		}
//		myfile.close();
//	} catch (InconsistentAngSpeedEstimation& ex) {
//		cout << ex.what() << endl;
//	}
//	cout << "DONE!" << endl;
//	waitKey(0);
//	return 0;
//}

int main(int argc, char** argv) {
	double angspeed = findXAimSpeed(18, 0);
	cout << "pivot_ang_speed:" << angspeed << endl;
	Acquisition acq(PIVOT_VIDEO_NAME, angspeed);

	int bx = 18, ex = 50;
	int by = 20, ey = 50;
	bool fast_init = false;

	ofstream myfile;
	myfile.open("map3.txt", ios::out | ios::app);
	try {
		for (int y = by; y <= ey; y++) {
			for (int x = bx; x <= ex; x++) {
				if (y + x < 18)
					continue;
				double angspeed = findXAimSpeed(x, y, acq, fast_init);
				myfile << x << ";" << y << ";" << angspeed << endl;
				fast_init = true;
			}
			myfile.flush();
		}
	} catch (CharacterOutOfPositionException& ex) {
		cout << ex.what() << endl;
	}
	myfile.close();

	waitKey(0);

	return 0;
}
