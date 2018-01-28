/*
 * core.cpp
 *
 *  Created on: Jan 21, 2018
 *      Author: Lucas
 */

#include "aim_analysis_core.h"
#include <opencv2/highgui.hpp>

#include "gimx_api.h"
#include "utils/utils.h"
#include "utils/ImageUtils.h"
#include "utils/EventVideoCapture.hpp"
#include "Acquisition.hpp"
#include <thread>
#include <cstdlib>
#include <vector>
#include <cstdio>
#include <chrono>
#include <iostream>

using namespace std;
using namespace cv;
using namespace std::chrono;
using namespace aim_analysis;

class MatchObserver: public EventVideoCapture::VideoListener {
//	VideoWriter vwriter;
//	const char* outfile;
	const Acquisition& acq;

	double sum_speed = 0;
	double sumX2 = 0;
	int n = 0;

	double cur_angle = -1;
	double last_angle = -1;
	int last_fn = 0;
	int first_fn = -1;
	double first_angle = -1;
	bool passed360 = false;

	double delta_angle; //temporary variable;
	double tmp; //temporary variable 2;

public:
	MatchObserver(const Acquisition& acquisition) :
			acq(acquisition) {

	}

	virtual int frameRead(cv::Mat& mat, int frame_number) {
//		cout << "frameRead() called" << endl;
//		cout << "size=" << mat.rows << "x" << mat.cols << endl;
		Mat new_mat(mat, R);
//		Mat new_mat = tmp_mat.clone();

		imshow("W", new_mat);
		waitKey(1);
		cur_angle = acq.getAngle(new_mat);
//		cout << "cur_angle:" << cur_angle << endl;
		if (first_fn == -1) {
			first_fn = frame_number;
			first_angle = cur_angle;
			passed360 = false;
		} else {
			delta_angle = cur_angle - last_angle;
			if (delta_angle < 0) {
				if(passed360){
					return 1; // Two cycles completed.
				}
				passed360 = true;
				delta_angle = cur_angle + 360 - last_angle;
			}
			if (passed360) {
				if (cur_angle > first_angle) { //A cycle completed
					return 1;
				}
			}
			tmp = delta_angle / (frame_number - last_fn);
			sum_speed += tmp;
			sumX2 += tmp * tmp;
			n++;
//			if (frame_number % 45) {
//				cout << "delta_angle_rel:" << tmp << "  estimated avg speed:"
//						<< getAverageSpeed() << endl;
//			}
		}

		last_fn = frame_number;
		last_angle = cur_angle;
		return 0;
	}

	double getAverageSpeed() const {
		return ((sum_speed / n) * FPS);
	}

	double getStdDevSpeed() const {
		double avg = getAverageSpeed();
		return sqrt(sumX2 / n * FPS * FPS - avg * avg);
	}
};

static double startExperimentX(VideoCapture& vc, int x_axis, int y_axis, const Acquisition& acq,
		bool fast_init = false) {
	cout << "Starting new experiment for (" << x_axis << "," << y_axis << ")..." << endl;
	setAimSpeed(x_axis, 127, 0);
	if (fast_init) {
		this_thread::sleep_for(chrono::milliseconds(200));
	} else {
		this_thread::sleep_for(chrono::milliseconds(1100));
	}
	setAimSpeed(x_axis, y_axis, 0);
	this_thread::sleep_for(chrono::milliseconds(500));
	removeBuffer(vc);

	MatchObserver match_obs(acq);
	EventVideoCapture evtvc(vc, &match_obs);
	evtvc.startReadLoop();

	cout << ">>average=" << match_obs.getAverageSpeed() << " STD_DEV=" << match_obs.getStdDevSpeed()
			<< endl;

	return match_obs.getAverageSpeed();
}

double findXAimSpeed(int x_axis, int y_axis, const Acquisition& acq, bool fast_init) {
	VideoCapture vc(0);

	if (!vc.isOpened()) {
		printf("Nao foi possivel abrir o VideoCapture.\n");
		return -1;
	}
	vc.set(CV_CAP_PROP_FRAME_WIDTH, 1280); //1920x1080, 1280x720
	vc.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	return startExperimentX(vc, x_axis, y_axis, acq, fast_init);
}

//================================================//
//=================OLD METHOD=====================//
//================================================//

static int waitForMatch(VideoCapture& vc, const vector<Mat>& match_list,
		high_resolution_clock::time_point* end_time_OUT, const double good_mse_limit,
		vector<Mat>* frames_read_OUT, VideoWriter& vwriter) {
	int state = 0;
	int fn = 0;
	int cfn = -1;
	Mat close_frame;
	double close_frame_mse = 1000;
	double cur_mse, last_mse = -1;
	const double bad_mse_limit = good_mse_limit * 1.5;
	Mat frame;
	Mat last_frame;
	high_resolution_clock::time_point finish;

	while (true) {
		vc >> frame;
		finish = chrono::high_resolution_clock::now();
		fn++;
		frame = Mat(frame, R);
		if (frames_read_OUT != NULL) {
			frames_read_OUT->push_back(frame);
		}
		cur_mse = mse(frame, match_list[0]);
		for (int i = 1; i < match_list.size(); i += 30) {
			double m = mse(frame, match_list[i]);
			if (m < cur_mse) {
				cur_mse = m;
			}
		}

		switch (state) {
		case 0:
			if (cur_mse > bad_mse_limit) {
				state = 1;
			}
			break;
		case 1:
			if (cur_mse <= good_mse_limit) {
				state = 2;
				close_frame = frame;
				close_frame_mse = cur_mse;
				cfn = fn;
				imshow("Closest", close_frame);
//				cout << "C:" << cur_mse << endl;
				if (end_time_OUT != NULL)
					*end_time_OUT = finish;
			}
			break;
		case 2:
			if (cur_mse < close_frame_mse) {
				if (end_time_OUT != NULL)
					*end_time_OUT = finish;
				close_frame = frame;
				close_frame_mse = cur_mse;
				cfn = fn;
				imshow("Closest", close_frame);
//				cout << "C:" << cur_mse << endl;

			}
			if (cur_mse > bad_mse_limit) {
				state = 3;
				setAimSpeed(0, 0, 0);
				return cfn;
			}
			if (fabs(last_mse - cur_mse) < 0.1) {
				return fn - 1;
			}
			break;
		}

		vwriter.write(frame);
		imshow("W1", frame);
		waitKey(1);
		last_frame = frame;
		last_mse = cur_mse;

	}
	return cfn;
}

static int waitForMatch(VideoCapture& vc, Mat& match,
		high_resolution_clock::time_point* end_time_OUT, const double good_mse_limit,
		vector<Mat>* frames_read_OUT, VideoWriter& vwriter) {
	vector<Mat> m(1, match);
	return waitForMatch(vc, m, end_time_OUT, good_mse_limit, frames_read_OUT, vwriter);
}

static double startExperiment(VideoCapture& vc, const vector<Mat>& match_list,
		double total_mvt_angle, const double good_mse_limit, VideoWriter& vwriter) {
	auto start = chrono::high_resolution_clock::now();

	imshow("Match_frame0", match_list[0]);
//	this_thread::sleep_for(chrono::milliseconds(20));
	int fn = removeBuffer(vc);

	high_resolution_clock::time_point finish;
	fn += waitForMatch(vc, match_list, &finish, good_mse_limit, NULL, vwriter);
	int TTT = chrono::duration_cast<chrono::microseconds>(finish - start).count();
	TTT += 10 * 1000;
	double angspeed1 = total_mvt_angle / (TTT / 1000.0) * 1000;
	double angspeed2 = (60 * total_mvt_angle) / fn;

	if (fabs(angspeed1 / angspeed2 - 1) > 0.03) {
		throw InconsistentAngSpeedEstimation(angspeed1, angspeed2);
	}
	return (angspeed1 + angspeed2) / 2;
}

static double startExperiment(VideoCapture& vc, Mat& match, double total_mvt_angle,
		const double good_mse_limit, VideoWriter& vwriter) {
	vector<Mat> m(1, match);
	return startExperiment(vc, m, total_mvt_angle, good_mse_limit, vwriter);
}

static double startExperimentX(VideoCapture& vc, int x_axis, int y_axis,
		const double good_mse_limit = 5, int num_exps = 1) {
	VideoWriter vwriter;
	vwriter.open(PIVOT_VIDEO_NAME, CV_FOURCC('M', 'J', 'P', 'G'), 60, Size(R.width, R.height),
			true);
	Mat first_frame;

	cout << "Starting experiment for (" << x_axis << "," << y_axis << ")..." << endl;

	setAimSpeed(x_axis, 127, 0);
	this_thread::sleep_for(chrono::milliseconds(1100));
	setAimSpeed(x_axis, y_axis, 0);
	this_thread::sleep_for(chrono::milliseconds(1100));
	removeBuffer(vc);
	vc >> first_frame;
	first_frame = Mat(first_frame, R);
	vwriter.write(first_frame);
	double ret = startExperiment(vc, first_frame, 360, 6, vwriter);
	vwriter.release();
	return ret;
}

double findXAimSpeed(int x_axis, int y_axis) {
	VideoCapture vc(0);
	if (!vc.isOpened()) {
		printf("Nao foi possivel abrir o VideoCapture.\n");
		return -1;
	}
	vc.set(CV_CAP_PROP_FRAME_WIDTH, 1280); //1920x1080, 1280x720
	vc.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	double ret = startExperimentX(vc, x_axis, y_axis, 5, 5);
	vc.release();
	return ret;
//	setAimSpeed(x_axis, 127, 0);
//	this_thread::sleep_for(chrono::milliseconds(1100));
//	setAimSpeed(x_axis, y_axis, 0);
//	this_thread::sleep_for(chrono::milliseconds(20));
//	removeBuffer(vc);
//	vc >> first_frame;
//	first_frame = Mat(first_frame, R);
//	return startExperiment(vc, first_frame, 360);
}

