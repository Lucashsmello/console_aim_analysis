/*
 * utils.h
 *
 *  Created on: 10 de set de 2017
 *      Author: lmello
 */

#ifndef CPP_UTILS_UTILS_H_
#define CPP_UTILS_UTILS_H_

#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <time.h>

//#define PROFILE_TIME_G(LABEL,TTIME,X) X;

#define PROFILE_TIME_G(LABEL,TTIME,X) auto start = std::chrono::high_resolution_clock::now(); \
		X;\
		{\
		auto finish = std::chrono::high_resolution_clock::now(); \
		int TTT=std::chrono::duration_cast<TTIME>(finish - start).count(); \
		std::cout << LABEL << TTT << "ms" << std::endl;}

#define PROFILE_TIME(LABEL,X) PROFILE_TIME_G(LABEL,std::chrono::milliseconds,X);

#define PROFILE_TIME_G_C(LABEL,TTIME,X) {PROFILE_TIME_G(LABEL,TTIME,X);}

#define PROFILE_TIME_C(LABEL,X) {PROFILE_TIME(LABEL,X);}

class Utils {
	template<typename Out>
	static void split(const std::string &s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}

public:
	static std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}

	static double printTime(const clock_t& t, const char* name) {
		double time_taken = ((double) clock() - t) / CLOCKS_PER_SEC; // in seconds
		if (time_taken >= 100) {
			printf("%s:%.1fmin\n", name, time_taken / 60);
		} else {
			if (time_taken > 2) {
				printf("%s:%.1fsec\n", name, time_taken);
			} else {
				printf("%s:%dms\n", name, (int) (time_taken * 1000));
			}
		}
		return time_taken;
	}

	template<typename T>
	static double average(const std::vector<T>& vet) {
		double s = vet[0];
		for (int i = 1; i < vet.size(); i++) {
			s += vet[i];
		}
		return s / vet.size();
	}

	template<typename T>
	static double std(const std::vector<T>& vet) {
		return std(vet, average(vet));
	}

	template<typename T>
	static double std(const std::vector<T>& vet, double avg) {
		double s = (vet[0] - avg);
		s *= s;
		for (int i = 1; i < vet.size(); i++) {
			s += (vet[i] - avg) * (vet[i] - avg);
		}
		return sqrt(s / vet.size());
	}

};

#endif /* CPP_UTILS_UTILS_H_ */
