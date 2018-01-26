/*
 * Acquisition.cpp
 *
 *  Created on: Jan 23, 2018
 *      Author: Lucas
 */

#include "Acquisition.hpp"
#include "utils/ImageUtils.h"
#include "utils/utils.h"
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

namespace aim_analysis {

Acquisition::Acquisition(const string& video_file, double ang_speed) :
		angspeed(ang_speed) {
	VideoCapture vc(video_file);
	if (!vc.isOpened()) {
		throw string("Nao foi possivel abrir o VideoCapture.\n");
	}
	Mat frame;
	while (vc.isOpened()) {
		vc >> frame;
		if (frame.empty())
			break;
//		frames.push_back(Mat(frame, R));
		frames.push_back(frame.clone());
	}
	vc.release();
	cout << "End reading acquisition..." << endl;
}

double Acquisition::getAngle(const cv::Mat& match) const {
	if (match.rows != frames[0].rows || match.cols != frames[0].cols) {
		cout << "ERROR: images sizes does not match! " << match.rows << "x" << match.cols << "   "
				<< frames[0].rows << "x" << frames[0].cols << endl;
		return 0;
	}
	double low_mse = mse(frames[0], match);
	int low_index = 0;
	double cur_mse;
	for (unsigned int i = 1; i < frames.size(); i++) {
		cur_mse = mse(frames[i], match);
		if (cur_mse < low_mse) {
			low_index = i;
			low_mse = cur_mse;
		}
	}

	if (low_mse > 10) {
		throw CharacterOutOfPositionException();
	}

	return getAngle(low_index);
}

} /* namespace console_analysis */

