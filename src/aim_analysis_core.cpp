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
#include <cmath>

using namespace std;
using namespace cv;
using namespace std::chrono;
using namespace aim_analysis;

static VideoCapture VCD(0);

class MatchObserver: public EventVideoCapture::VideoListener {
//	VideoWriter vwriter;
//	const char* outfile;
	const Acquisition& acq;

	double sum_speed = 0;
//	double sumX2 = 0;
	int n = 0;
	const int N_MAXFRAMES = 5 * 60;

	double cur_angle = -1;
	double last_angle = -1;
	int last_fn = 0;
	int last_frame_index_match = 0;
	int first_fn = -1;
	double first_angle = -1;
	bool passed360 = false;

	void updateCurrentAngle() {
		cur_angle = acq.getAngle(last_frame_index_match);
//		if (last_frame_index_match == 0) {
//			last_frame_index_match = acq.frames.size() - 5;
//		} else {
//			last_frame_index_match -= 5;
//		}
	}

public:
	MatchObserver(const Acquisition& acquisition) :
			acq(acquisition) {

	}

	virtual int frameRead(cv::Mat& mat, int frame_number) {
//		cout << "frame number: " << frame_number << endl;
		double delta_angle; //temporary variable;
		double tmp; //temporary variable 2;
		Mat new_mat(mat, R);

		if (first_fn == -1) {
			last_frame_index_match = acq.getFrameIndex(new_mat, 0, 360);
			updateCurrentAngle();

			first_fn = frame_number;
			first_angle = cur_angle;
			passed360 = false;
		} else {
			try {
				if (last_frame_index_match >= 0) {
					last_frame_index_match = acq.getFrameIndex(new_mat, last_frame_index_match,
							7 * (frame_number - last_fn));
				} else {
					last_frame_index_match = acq.getFrameIndex(new_mat, 0, 360);
				}
			} catch (CharacterOutOfPositionException& ex) {
				if (ex.v > 80) {
					throw;
				}
				last_frame_index_match = -1;
				return 0;
			}
			updateCurrentAngle();

			delta_angle = cur_angle - last_angle;
			if (delta_angle < 0) {
//				if (passed360) {
//					return 1; // Two cycles completed.
//				}
				passed360 = true;
				delta_angle = cur_angle + 360 - last_angle;
			}
			if (passed360) {
//				if (cur_angle > first_angle) { //A cycle completed
//					return 1;
//				}
			}
			tmp = delta_angle / (frame_number - last_fn);
			sum_speed += tmp;
//			sumX2 += tmp * tmp;
			n++;
			if (n >= N_MAXFRAMES) {
				return 1;
			}
			if (frame_number % 45 == 0) {
				cout << "delta_angle_rel:" << tmp << "  estimated avg speed:" << getAverageSpeed()
						<< endl;
			}
		}

		last_fn = frame_number;
		last_angle = cur_angle;
		return 0;
	}

	double getAverageSpeed() const {
		return ((sum_speed / n) * FPS);
	}

//	double getStdDevSpeed() const {
//		double avg = getAverageSpeed();
//		return sqrt(sumX2 / n * FPS * FPS - avg * avg);
//	}
};

class MatchObserver2: public EventVideoCapture::VideoListener {
	const Acquisition& acq;
	int fn1 = -1;
	bool moved = false;
	int xaxis;
	int yaxis;
	double ang1 = -1;
	int jump_frames = 0;

	double sum_speed = 0;
	double sum_speed2 = 0;
	int n = 0;
	Mat newmat;

//	int last_fn = -1;

public:
	MatchObserver2(int x_axis, int y_axis, const Acquisition& acquisition) :
			acq(acquisition) {
		xaxis = x_axis;
		yaxis = y_axis;
	}

	virtual int frameRead(cv::Mat& mat, int frame_number) {
//		if (last_fn >= 0 and frame_number != last_fn + 1) {
//			cout << "frameRead() ERROR: frame number non-sequential!!! " << last_fn << ","
//					<< frame_number << endl;
//		}

//		last_fn = frame_number;
		if (jump_frames > 0) {
			jump_frames--;
			return 0;
		}

		if (not moved) {
			if (ang1 == -1) {
				newmat = Mat(mat, R);
				ang1 = acq.getAngle(newmat);
				jump_frames = 5;
				return 0;
			}
			fn1 = frame_number;
			setAimSpeed(xaxis, yaxis, 0);
			moved = true;
			return 0;
		}
		if (frame_number == fn1 + 1) {
			setAimSpeed(0, 0, 0);
			jump_frames = 20;
			fn1 = -2;
			return 0;
		}
		if (fn1 != -2) {
			cout << "frameRead() ERROR CODE 2" << endl;
		}

		newmat = Mat(mat, R);
		double dx = acq.getAngle(newmat) - ang1;
		if (dx < 0) {
			dx += 360;
		}

		sum_speed += dx;
		sum_speed2 += dx * dx;
		n++;
		if (n == 30) {
			return 1;
		}

		ang1 = -1;
		moved = false;
		return 0;
	}

	/**
	 *
	 * @return in degrees per frame.
	 */
	double getAverageSpeed() const {
		return sum_speed / n;
	}

	/**
	 *
	 * @return in degrees per frame.
	 */
	double getSTDSpeed() const {
		return sum_speed2 / n - getAverageSpeed() * getAverageSpeed();
	}

};

static double startExperimentX(VideoCapture& vc, int x_axis, int y_axis, const Acquisition& acq,
		bool fast_init = false, bool single_tick = false) {
	cout << "Starting new experiment for (" << x_axis << "," << y_axis << ")..." << endl;
	setAimSpeed(x_axis, 127, 0);
	if (fast_init) {
		this_thread::sleep_for(chrono::milliseconds(100));
	} else {
		this_thread::sleep_for(chrono::milliseconds(1100));
	}
	if (single_tick) {
		Mat fr1, fr2;
		setAimSpeed(0, 0, 0);
		this_thread::sleep_for(chrono::milliseconds(500));
		MatchObserver2 match_obs(x_axis, y_axis, acq);
		EventVideoCapture evtvc(vc, &match_obs);
		evtvc.startReadLoop();

		cout << ">>average=" << match_obs.getAverageSpeed() << " STD_DEV="
				<< match_obs.getSTDSpeed() << endl;

		return match_obs.getAverageSpeed();
	} else {
		setAimSpeed(x_axis, y_axis, 0);
		this_thread::sleep_for(chrono::milliseconds(500));
		MatchObserver match_obs(acq);
		EventVideoCapture evtvc(vc, &match_obs);
		evtvc.startReadLoop();

		cout << ">>average=" << match_obs.getAverageSpeed() <<
		//			" STD_DEV=" << match_obs.getStdDevSpeed() <<
				endl;

		return match_obs.getAverageSpeed();
	}

}

double findXAimSpeed(int x_axis, int y_axis, const Acquisition& acq, bool fast_init,
		bool single_tick) {
	if (!VCD.isOpened()) {
		printf("Nao foi possivel abrir o VideoCapture.\n");
		return -1;
	}
	VCD.set(CV_CAP_PROP_FRAME_WIDTH, 1280); //1920x1080, 1280x720
	VCD.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	double ret = startExperimentX(VCD, x_axis, y_axis, acq, fast_init, single_tick);
	return ret;
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
//				imshow("Closest", close_frame);
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
//				imshow("Closest", close_frame);
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
		imshow("W", frame);
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
//	TTT += 10 * 1000;
	double angspeed1 = total_mvt_angle / (TTT / 1000.0) * 1000;
	double angspeed2 = (60 * total_mvt_angle) / fn;

	if (fabs(angspeed1 / angspeed2 - 1) > 0.02) {
		throw InconsistentAngSpeedEstimation(angspeed1, angspeed2);
	}
//	cout << "angspeed_time:" << angspeed1 << " angspeed_frames:" << angspeed2 << endl;

	return (angspeed1 + angspeed2) / 2;
}

static double startExperiment(VideoCapture& vc, Mat& match, double total_mvt_angle,
		const double good_mse_limit, VideoWriter& vwriter) {
	vector<Mat> m(1, match);
	return startExperiment(vc, m, total_mvt_angle, good_mse_limit, vwriter);
}

static double startExperimentX(VideoCapture& vc, int x_axis, int y_axis,
		const double good_mse_limit = 6, int num_exps = 1) {
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
	double ret = startExperiment(vc, first_frame, 360, good_mse_limit, vwriter);
	vwriter.release();
	return ret;
}

double findXAimSpeed(int x_axis, int y_axis) {
	if (!VCD.isOpened()) {
		printf("Nao foi possivel abrir o VideoCapture.\n");
		return -1;
	}
	VCD.set(CV_CAP_PROP_FRAME_WIDTH, 1280); //1920x1080, 1280x720
	VCD.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

	double ret = startExperimentX(VCD, x_axis, y_axis, 22);
//	vc.release();
	destroyWindow("W");
	destroyWindow("Match_frame0");
	return ret;
}

