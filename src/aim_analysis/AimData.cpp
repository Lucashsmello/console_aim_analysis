/*
 * DataHandler.cpp
 *
 *  Created on: Feb 2, 2018
 *      Author: Lucas
 */

#include "AimData.hpp"
#include "../utils/utils.h"
#include <fstream>
#include <cstdlib>
#include <iostream>

using namespace std;

namespace aim_analysis {

AimData::AimData() {
	double* Px;
	double* Py;
	for (int i = 0; i < AIMDATA_AXIS_SIZE; i++) {
		Px = axis_2_xspeed[i];
		Py = axis_2_yspeed[i];

		for (int j = 0; j < AIMDATA_AXIS_SIZE; j++) {
			Px[j] = UNKNOWN_SPEED;
			Py[j] = UNKNOWN_SPEED;
		}
	}
}

AimData::AimData(const std::string& str) :
		AimData() {
	file_name = str;
	ifstream csv_data(str);
	string line;
	vector<string> spl;
	double x_speed, y_speed;
	int x, y;

	if (csv_data.is_open()) {
		while (getline(csv_data, line)) {
			spl = Utils::split(line, DELIM);
			if (spl.size() < 3) {
				continue;
			}
			x = atoi(spl[0].c_str());
			y = atoi(spl[1].c_str());
			x_speed = atof(spl[2].c_str());
			if (spl.size() >= 4) {
				y_speed = atof(spl[3].c_str());
				addResult(x, y, x_speed, y_speed);
			} else {
				addResult(x, y, x_speed);
			}
		}
		csv_data.close();
	}
}

void AimData::addResult(int x, int y, double xspeed, double yspeed) {
	if (hasXSpeed(x, y)) {
		throw string(
				"AimData::addResult|adding repeated experimental results are not supported yet!\n");
	}
	axis_2_xspeed[x][y] = xspeed;
	axis_2_yspeed[x][y] = yspeed;
}

void AimData::saveResult(int x, int y, double xspeed, double yspeed) {
	addResult(x, y, xspeed, yspeed);
	ofstream append_stream;
	append_stream.open(file_name, ios::out | ios::app);
	append_stream << x << DELIM << y << DELIM << xspeed;
	if (yspeed != UNKNOWN_SPEED) {
		append_stream << DELIM << yspeed;
	}
	append_stream << endl;
	append_stream.close();
}

} /* namespace aim_analysis */

