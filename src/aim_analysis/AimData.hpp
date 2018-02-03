/*
 * DataHandler.hpp
 *
 *  Created on: Feb 2, 2018
 *      Author: Lucas
 */

#ifndef SRC_AIM_ANALYSIS_AIMDATA_HPP_
#define SRC_AIM_ANALYSIS_AIMDATA_HPP_

#include <string>
#include <vector>
#include <map>

#define AIMDATA_AXIS_SIZE 128

namespace aim_analysis {
const double UNKNOWN_SPEED = -1;
//struct iPoint {
//	int x;
//	int y;
//};
//
//struct aimPoint {
//	iPoint axis;
//	double x_speed;
//	double y_speed;
//};

class AimData {
	static const char DELIM = ';';

	double axis_2_xspeed[AIMDATA_AXIS_SIZE][AIMDATA_AXIS_SIZE];
	double axis_2_yspeed[AIMDATA_AXIS_SIZE][AIMDATA_AXIS_SIZE];
	std::string file_name;
//	std::map<iPoint, aimPoint, bool (*)(iPoint, iPoint)> axis_2_speed;

public:

	AimData();
	AimData(const std::string& str);

	/**
	 *
	 * @param x in [0,127]
	 * @param y in [0,127]
	 * @param xspeed in degrees per second.
	 * @param yspeed in degrees per second.
	 */
	void addResult(int x, int y, double xspeed, double yspeed = UNKNOWN_SPEED);

	/**
	 *
	 * @param x in [0,127]
	 * @param y in [0,127]
	 * @param xspeed in degrees per second.
	 * @param yspeed in degrees per second.
	 */
	void saveResult(int x, int y, double xspeed, double yspeed = UNKNOWN_SPEED);

	/**
	 *
	 * @param x
	 * @param y
	 * @return in degrees per second.
	 */
	double getXSpeed(int x, int y) const {
		return axis_2_xspeed[x][y];
	}

	/**
	 *
	 * @param x
	 * @param y
	 * @return in degrees per second.
	 */
	double getYSpeed(int x, int y) const {
		return axis_2_yspeed[x][y];
	}

	bool hasXSpeed(int x, int y) const {
		return getXSpeed(x, y) != UNKNOWN_SPEED;
	}

};

} /* namespace aim_analysis */

#endif /* SRC_AIM_ANALYSIS_AIMDATA_HPP_ */
