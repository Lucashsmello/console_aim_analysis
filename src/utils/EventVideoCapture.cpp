/*
 * BufferredVideoCapture.cpp
 *
 *  Created on: Jan 21, 2018
 *      Author: Lucas
 */

#define EVC_WARN_ON

#include "EventVideoCapture.hpp"

#include <iostream>
#include <boost/thread.hpp>
#include "ImageUtils.h"
#include <chrono>

using namespace cv;
using namespace std;
using namespace std::chrono;
static boost::mutex Frame_mutex;
static boost::mutex new_frame_mutex;
static boost::condition_variable condition;

void EventVideoCapture::stopReadLoop() {
	abort = true;
}

EventVideoCapture::~EventVideoCapture() {
	stopReadLoop();
}

void EventVideoCapture::startReadLoop() {
	abort = false;
	new_frame = false;

#pragma omp parallel sections // starts a new team
	{
		{
			readLoop();
		}
#pragma omp section
		{
			listenerLoop(vlistener1);
		}
	}
//	boost::thread T = boost::thread(boost::bind(&EventVideoCapture::readLoop, this));
//	listenerLoop(vlistener1);
	stopReadLoop();
//	T.join();
}

void EventVideoCapture::readLoop() {
#ifdef EVC_WARN_ON
	high_resolution_clock::time_point T1, T2;
#endif
	int TTT;
	fn = -1;
	removeBuffer(video);
	Mat tmp_frame;
	while (not abort) {
		video >> tmp_frame;
#ifdef EVC_WARN_ON
		T1 = high_resolution_clock::now();
		if (fn >= 0) {
			TTT = duration_cast<milliseconds>(T1 - T2).count();
			if (TTT <= 13) {
				cout << "EventVideoCapture::readLoop() WARNING: reading next frame was too fast! "
						<< TTT << "ms"  << " | fn=" << fn << endl;
			} else {
				if (TTT >= 30) {
					cout
							<< "EventVideoCapture::readLoop() WARNING: reading next frame was too slow! "
							<< TTT << "ms" << " | fn=" << fn << endl;
				}
			}
		}
		T2 = T1;
#endif

		Frame_mutex.lock();
		cur_frame = tmp_frame.clone();
		fn++;
		new_frame = true;
		Frame_mutex.unlock();
		condition.notify_one();

//		boost::this_thread::yield();

	}
	new_frame = true; // ends listenerLoop.

}

void EventVideoCapture::listenerLoop(VideoListener* l) {
	boost::mutex::scoped_lock lock(new_frame_mutex);
	Mat cloned_frame;
	int cur_fn = 0;
	while (abort == false) {
//		{
//			boost::unique_lock<boost::mutex> lock(m_pause_mutex);
//			while (new_frame == false) {
//				m_pause_changed.wait(lock);
//			}
//		}
		while (new_frame == false) {
			condition.wait(lock);
//			boost::this_thread::yield();
		}

		if (abort)
			break;
		Frame_mutex.lock();
		new_frame = false;
		cloned_frame = cur_frame.clone();
		cur_fn = fn;
		Frame_mutex.unlock();
		if (l->frameRead(cloned_frame, cur_fn) == 1) {
			stopReadLoop();
			break;
		}

//		if (new_frame == true) {
//			cout << "====WARNING: vlistener.frameRead(cur_frame) took too much time!====" << endl;
//		}
	}
}
