/*
 * core.hpp
 *
 *  Created on: Jan 21, 2018
 *      Author: Lucas
 */
#ifndef AIM_ANALYSIS_CORE_HPP
#define AIM_ANALYSIS_CORE_HPP

#include <opencv2/videoio.hpp>
#include "Acquisition.hpp"

static const char* PIVOT_VIDEO_NAME = "pivot.avi";

double findXAimSpeed(int x_axis, int y_axis);
double findXAimSpeed(int x_axis, int y_axis, const aim_analysis::Acquisition& acq, bool fast_init, bool single_tick=false);

class InconsistentAngSpeedEstimation: public std::exception {
	char msg[110];
public:
	InconsistentAngSpeedEstimation(double a, double b) {
		sprintf_s(msg, ">>>ERROR: ang speeds differs greatly (%f,%f)!<<<", a, b);
	}
	virtual const char* what() const throw () {
		return msg;
	}
};

#endif
