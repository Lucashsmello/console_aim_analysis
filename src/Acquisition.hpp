/*
 * Acquisition.h
 *
 *  Created on: Jan 23, 2018
 *      Author: Lucas
 */

#ifndef SRC_ACQUISITION_HPP_
#define SRC_ACQUISITION_HPP_

#include <opencv2/core.hpp>
#include <vector>
#include <string>

namespace aim_analysis {

static const unsigned int FPS = 60;
static const double FRAME_PERIOD_SEC = 1.0 / FPS;
static const int RESX = 1280;
static const int RESY = 720;
static const int DX = RESX / 12;
static const int DY = RESY / 12;
//static const cv::Rect R(RESX / 2 - DX, RESY / 2 - DY, 1.22 * DX, 1.75 * DY); //OVERWATCH
//static const cv::Rect R(RESX / 2 - 0.62 * DX, RESY / 2 - DY, 1.22 * DX, 1.72 * DY); //HORIZON
//static const cv::Rect R(240, 190, 110, 110); //DESTINY2
static const cv::Rect R(RESX / 2, RESY / 2 + 40, 110, 110); //Red Dead R2

class Acquisition {
public:
	Acquisition(const std::string& video_file, double ang_speed);

	std::vector<cv::Mat> frames;
	const double angspeed; // degrees per second.

	/**
	 * Returns in seconds.
	 */
	double getDuration(int frame_index) const {
		return FRAME_PERIOD_SEC * frame_index;
	}

	/**
	 * Returns in seconds.
	 */
	double getDuration() const {
		return getDuration(frames.size() - 1);
	}

	/**
	 * Returns in degrees.
	 */
	double getAngle(unsigned int frame_index) const {
		return getDuration(frame_index) * angspeed;
	}

	/**
	 * Returns in degrees.
	 */
	double getAngle(const cv::Mat& match) const;

	unsigned int getFrameIndex(const cv::Mat& match, unsigned int start_index_search,
			double range_angle) const;

//	unsigned int getFrameIndex(const cv::Mat& match) const {
//		return getFrameIndex(match, 0, 360);
//	}

};

class CharacterOutOfPositionException: public std::exception {
	char msg[110];
public:
	const double v;
	CharacterOutOfPositionException(double val) : v(val){
		sprintf_s(msg,
				">>>ERROR: It seems that the aim or position has moved by an external agent during experiment!<<<");
	}

	virtual const char* what() const throw () {
		return msg;
	}
};

} /* namespace console_analysis */

#endif /* SRC_ACQUISITION_HPP_ */
