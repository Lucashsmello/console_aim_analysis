/*
 * test.cpp
 *
 *  Created on: Feb 3, 2018
 *      Author: Lucas
 */

#include <iostream>
#include "aim_analysis/AimData.hpp"

using namespace std;
using namespace aim_analysis;

int main(int argc, char** argv) {
	try {
		AimData aimdata("aimdata.csv");
		cout << ">>" << aimdata.getXSpeed(30, 20) << endl;
	} catch (string& ex) {
		cout << "ERROR:" << ex << endl;
	}

	return 0;
}
