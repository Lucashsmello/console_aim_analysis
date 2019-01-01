/*
 * Acquisition.cpp
 *
 *  Created on: Jan 23, 2018
 *      Author: Lucas
 */

#include <assert.h>
#include "Acquisition.hpp"
#include "utils/ImageUtils.h"
#include "utils/utils.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

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

static void debug_writeFrame(VideoWriter& vw, Mat match, Mat frame) {
	const double n = match.channels() * match.total();
	char text[6];
	Mat mall;
	Mat tmp = cv::abs(match - frame);
	double error = mse(frame, match);
	hconcat(match, frame, mall);
	hconcat(mall, tmp, mall);
	Mat tmp1 = equalizeIntensity(match);
	Mat tmp2 = equalizeIntensity(frame);
	Mat tmp3 = cv::abs(tmp1 - tmp2);
	hconcat(tmp1, tmp2, tmp2);
	hconcat(tmp2, tmp3, tmp3);
	vconcat(mall, tmp3, mall);

	Scalar s = sum(tmp);
	double avg = (s.val[0] + s.val[1] + s.val[2]) / n;
	Scalar d = sum(cv::abs(tmp - avg));
	double ddd = (d.val[0] + d.val[1] + d.val[2]) / n;

	snprintf(text, 6, "%.1lf", error);
	cv::putText(mall, text, cv::Point(5, 15), cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			1.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255)); // BGR Color
	snprintf(text, 6, "%.1lf", ddd);
	cv::putText(mall, text, cv::Point(5, match.rows - 16), cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
			1.0, // Scale. 2.0 = 2x bigger
			cv::Scalar(255, 255, 255)); // BGR Color
	vw.write(mall);
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
	assert(start_index_search < frames.size());
	unsigned int end_index_search;
	if (range_angle >= 359.9) {
		start_index_search = 0;
		end_index_search = frames.size();
	} else {
		end_index_search = (range_angle / 360) * frames.size() + start_index_search;
	}

	const unsigned int end1 = MIN(end_index_search, frames.size());
	const unsigned int end2 = end_index_search - end1;

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

	if (lowest.val > 80) {
		cout << "CharacterOutOfPositionException: " << lowest.val << " | start_index_search="
				<< start_index_search << endl;
		if (lowest.val >= 100) {
			Mat mall;
			hconcat(match, frames[lowest.index], mall);
			hconcat(mall, cv::abs(match - frames[lowest.index]), mall);
			imwrite("CharacterOutOfPositionException.jpg", mall);
			imshow("CharacterOutOfPositionException", mall);
			VideoWriter vwriter;
			vwriter.open("CharacterOutOfPositionException.avi", CV_FOURCC('M', 'J', 'P', 'G'), 60,
					Size(3 * R.width, 2 * R.height), true);
			for (unsigned int i = start_index_search; i < end1; i++) {
				debug_writeFrame(vwriter, match, frames[i]);
			}
			for (unsigned int i = 0; i < end2; i++) {
				debug_writeFrame(vwriter, match, frames[i]);
			}
			vwriter.release();
			waitKey(0);
		}
		throw CharacterOutOfPositionException(lowest.val);
	}

	return lowest.index;
}

} /* namespace console_analysis */

