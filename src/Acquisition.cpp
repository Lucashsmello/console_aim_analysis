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
	return getAngle(getFrameIndex(match, 0, 360));
}

struct Compare {
	double val;
	unsigned int index;
};
#pragma omp declare reduction(minimum : struct Compare : omp_out = omp_in.val < omp_out.val ? omp_in : omp_out)

unsigned int Acquisition::getFrameIndex(const cv::Mat& match, unsigned int start_index_search,
		double range_angle) const {
	if (match.rows != frames[0].rows || match.cols != frames[0].cols) {
		cout << "ERROR: images sizes does not match! " << match.rows << "x" << match.cols << "   "
				<< frames[0].rows << "x" << frames[0].cols << endl;
		return 0;
	}
	unsigned int end_index_search;
	if (range_angle >= 359.9) {
		start_index_search = 0;
		end_index_search = frames.size();
	} else {
		end_index_search = (range_angle / 360) * frames.size() + start_index_search;
	}

	const unsigned int end1 = MIN(end_index_search, frames.size());
	const unsigned int end2 = end_index_search - end1;

//	double low_mse = mse(frames[start_index_search], match);
//	unsigned int low_index = start_index_search;
	Compare lowest;
	lowest.val = mse(frames[start_index_search], match);
	lowest.index = start_index_search;

//	#pragma omp parallel for reduction(minimum:lowest)
	#pragma omp parallel for
	for (unsigned int i = start_index_search + 1; i < end1; i++) {
		double cur_mse = mse(frames[i], match);
	#pragma omp critical
		{
			if (cur_mse < lowest.val) {
				lowest.index = i;
				lowest.val = cur_mse;
			}
		}
	}

//	#pragma omp parallel for reduction(minimum:lowest)
	#pragma omp parallel for
	for (unsigned int i = 0; i < end2; i++) {
		double cur_mse = mse(frames[i], match);
	#pragma omp critical
		{
			if (cur_mse < lowest.val) {
				lowest.index = i;
				lowest.val = cur_mse;
			}
		}
	}

//	imshow("MATCH", match);
//	imshow("NEAR", frames[low_index]);
//	imshow("N1", frames[low_index + 1]);
//	imshow("N2", frames[low_index + 2]);
//	imshow("N3", frames[low_index + 3]);
//	waitKey(1);

	if (lowest.val > 10) {
		throw CharacterOutOfPositionException();
	}

	return lowest.index;
}

} /* namespace console_analysis */

