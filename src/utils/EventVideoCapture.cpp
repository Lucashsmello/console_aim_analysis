/*
 * BufferredVideoCapture.cpp
 *
 *  Created on: Jan 21, 2018
 *      Author: Lucas
 */

#include "EventVideoCapture.hpp"

#include <iostream>
#include <boost/thread.hpp>
#include "ImageUtils.h"

using namespace cv;
using namespace std;

void EventVideoCapture::stopReadLoop() {
	abort = true;
}

EventVideoCapture::~EventVideoCapture() {
	stopReadLoop();
}

void EventVideoCapture::startReadLoop() {
	abort = false;
	new_frame = false;
//	cout << "Starting ReadLoop..." << endl;
	boost::thread T = boost::thread(boost::bind(&EventVideoCapture::readLoop, this));
//	boost::thread T2;
//	if (vlistener2 != NULL) {
//		T2 = boost::thread(&listenerLoop, boost::ref(this), vlistener2);
//	}
	listenerLoop(vlistener1);
	stopReadLoop();
	T.join();
//	if (vlistener2 != NULL) {
//		T2.join();
//	}
}

void EventVideoCapture::readLoop() {
	fn = -1;
	removeBuffer(video);
	Mat tmp_frame;
	while (not abort) {
		video >> tmp_frame;
		Frame_mutex.lock();
		cur_frame = tmp_frame.clone();
		fn++;
		Frame_mutex.unlock();

//		{
//			boost::unique_lock<boost::mutex> lock(m_pause_mutex);
		new_frame = true;
		boost::this_thread::yield();
//		}

//		m_pause_changed.notify_all();

	}
	new_frame = true; // ends listenerLoop.

}

void EventVideoCapture::listenerLoop(VideoListener* l) {
	Mat cloned_frame;
	int cur_fn;
	while (abort == false) {
//		{
//			boost::unique_lock<boost::mutex> lock(m_pause_mutex);
//			while (new_frame == false) {
//				m_pause_changed.wait(lock);
//			}
//		}
		while (new_frame == false) {
			boost::this_thread::yield();
		}
		new_frame = false;
		if (abort)
			break;
		Frame_mutex.lock();
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
