/*
 * BufferredVideoCapture.h
 *
 *  Created on: Jan 21, 2018
 *      Author: Lucas
 */

#ifndef SRC_UTILS_EVENTVIDEOCAPTURE_HPP_
#define SRC_UTILS_EVENTVIDEOCAPTURE_HPP_

#include <opencv2/videoio.hpp>
#include <boost/thread.hpp>
#include <cstdlib>

class EventVideoCapture {
	boost::mutex Frame_mutex;
//	boost::mutex m_pause_mutex;
//	boost::condition_variable m_pause_changed;

public:
	class VideoListener {
	public:
		/**
		 * When 1 is returned, the video read loop should stop.
		 */
		virtual int frameRead(cv::Mat& mat, int frame_number)=0;
	};
	cv::VideoCapture video;

	EventVideoCapture(cv::VideoCapture& vc, VideoListener* listener1) :
			video(vc) {
		fn = -1;
		vlistener1 = listener1;
	}

	EventVideoCapture(cv::VideoCapture& vc, VideoListener* listener1, VideoListener* listener2) :
			video(vc) {
		fn = -1;
		vlistener1 = listener1;
		vlistener2 = listener2;
	}

	void startReadLoop();
	void stopReadLoop();

	virtual ~EventVideoCapture();

private:
	cv::Mat cur_frame;
	VideoListener* vlistener1 = NULL;
	VideoListener* vlistener2 = NULL;

	bool new_frame = false;
	bool abort = false;
	int fn;

	void listenerLoop(VideoListener* l);
	void readLoop();
};

#endif /* SRC_UTILS_EVENTVIDEOCAPTURE_HPP_ */
