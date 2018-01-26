/*
 * FramedVideoCapture.h
 *
 *  Created on: 10 de set de 2017
 *      Author: lmello
 */

#ifndef CPP_UTILS_FRAMEDVIDEOCAPTURE_H_
#define CPP_UTILS_FRAMEDVIDEOCAPTURE_H_

#include "opencv2/highgui/highgui.hpp"
#include <cstdio>

class FramedVideoCapture {
	int frame_number = 0;
	std::string fname;
public:
	cv::VideoCapture video;

	FramedVideoCapture(std::string s) :
			video(s) {
		fname = s;
	}

	FramedVideoCapture(cv::VideoCapture& vc) :
			video(vc) {
	}

	bool read(cv::Mat& image) {
		frame_number++;
		return video.read(image);
	}

	bool grab() {
		frame_number++;
		return video.grab();
	}

	bool read(cv::Mat& image, int n) {
		if (n < frame_number) {
			video = cv::VideoCapture(fname);
			frame_number = 0;
		}
		while (frame_number < n) {
			if (!grab()) {
				return false;
			}
		}
		return read(image);
	}

	void release() {
		video.release();
	}

	const std::string& getFileName() const {
		return fname;
	}
};

#endif /* CPP_UTILS_FRAMEDVIDEOCAPTURE_H_ */
