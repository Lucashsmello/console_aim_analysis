/*
 * ImageUtils.cpp
 *
 *  Created on: 17 de set de 2017
 *      Author: lmello
 */

#include <opencv2/opencv.hpp>
#include "ImageUtils.h"
#include <vector>
#include <chrono>
#include <cmath>

using namespace std;
using namespace cv;

bool isValidArea(const cv::Mat& m, const cv::Rect& r) {
	if (r.x < 0 || r.y < 0)
		return false;
	return (r.x + r.width < m.cols) && (r.y + r.height < m.rows);
}

void makeBorder(Mat& img, int top, int bottom, int left, int right) {
	Mat_<Vec3b> _I = img;
	for (int row = top; row < bottom; row++) {
		_I(row, left)[1] = 255;
		_I(row, right)[1] = 255;
	}
	for (int col = left; col < right; col++) {
		_I(top, col)[1] = 255;
		_I(bottom, col)[1] = 255;
	}
}

void makeBorder(cv::Mat& img, const cv::Rect& r) {
	makeBorder(img, r.y, r.y + r.height, r.x, r.x + r.width);
}

void red_filter(Mat& I, int l1, int l2, int s_min, int v_min, int s_max, int v_max) {
	red_filter(I, I, l1, l2, s_min, v_min, s_max, v_max);
}

void red_filter(Mat& I, Mat& buffer, int l1, int l2, int s_min, int v_min, int s_max, int v_max) {
	Mat1b mask = red_filter_mask(I, buffer, l1, l2, s_min, v_min, s_max, v_max);
	apply_mask(I, mask);
}

void red_filter(Mat& I, int l1, int l2, int s) {
	red_filter(I, l1, l2, s, s, 255, 255);
}

static void apply_mask_singleChannel(Mat& I, const Mat1b& mask) {
	const uchar* M_P;
	uchar* I_P;
	for (int row = 0; row < I.rows; row++) {
		M_P = mask.ptr<uchar>(row);
		I_P = I.ptr<uchar>(row);
		for (int col = 0; col < I.cols; col++) {
			if (M_P[col] == 0) {
				I_P[col] = 0;
			}
		}
	}
}

void apply_mask(Mat& I, const Mat1b& mask) { //TODO: Optimize
	if (I.channels() == 1) {
		apply_mask_singleChannel(I, mask);
		return;
	}
	Mat_<Vec3b> _I = I;
	const uchar* P;
	uchar* _Iptr;
	for (int row = 0; row < I.rows; row++) {
		P = mask.ptr<uchar>(row);
		_Iptr = _I.ptr<uchar>(row);
		int j;
		for (int col = 0; col < I.cols; col++) {
			if (P[col] == 0) {
				j = 3 * col;
				_Iptr[j] = 0;
				_Iptr[j + 1] = 0;
				_Iptr[j + 2] = 0;
			}
		}
	}
}

void drawCross(cv::Mat& I, cv::Point p, uchar r, uchar g, uchar b, int size, int length) {
	if (length == -1) {
		length = I.cols;
	}
	const int n = MIN(I.rows, p.y + length);
	const int m = MIN(I.cols, p.x + length);
	cv::Mat_<cv::Vec3b> _I = I;
	for (int row = MAX(0, p.y - length); row < n; row++) {
		if (row < 0 || row >= n) {
			continue;
		}
		for (int i = -size; i <= size; i++) {
			if (p.x + i >= 0 && p.x + i < m) {
				_I(row, p.x + i)[0] = b;
				_I(row, p.x + i)[1] = g;
				_I(row, p.x + i)[2] = r;
			}
		}
	}

	for (int col = MAX(0, p.x - length); col < m; col++) {
		if (col < 0 || col >= m) {
			continue;
		}
		for (int i = -size; i <= size; i++) {
			if (p.y + i >= 0 && p.y + i < n) {
				_I(p.y + i, col)[0] = b;
				_I(p.y + i, col)[1] = g;
				_I(p.y + i, col)[2] = r;
			}
		}
	}
}

void drawGreenCross(cv::Mat& I, cv::Point p, int size, int length) {
	drawCross(I, p, 0, 200, 0, size, length);
}

void my_imshow(const cv::Mat& m) {
	namedWindow("W");
	imshow("W", m);
	waitKey(0);
}

void apply_distmask(Mat& src, const Mat& mask) {
	if (src.rows != mask.rows || src.cols != mask.cols) {
		printf("ERROR! apply_distmask():'src.rows != mask.rows || src.cols!=mask.cols'\n");
		return;
	}
	cv::Mat_<cv::Vec3b> _I = src;
	for (int i = 0; i < mask.rows; i++) {
		const float* row_vet = mask.ptr<float>(i);
		for (int j = 0; j < mask.cols; j++) {
			uchar v;
			if (row_vet[j] > 1) {
				v = 254;
			} else {
				if (row_vet[j] < 0) {
					v = 0;
				} else {
					v = row_vet[j] * 254;
				}
			}
			_I(i, j)[0] *= v / 255;
			_I(i, j)[1] *= v / 255;
			_I(i, j)[2] *= v / 255;
		}
	}
}

void apply_distmask(Mat& src, const Mat& mask, Point p) {
	Rect r(p.x, p.y, mask.cols, mask.rows);
	Mat subsrc(src, r);
	apply_distmask(subsrc, mask);
}

cv::Mat coloredImage(const cv::Mat& r) {
	vector<Mat> list;
	list.push_back(Mat(r));
	list.push_back(Mat(r));
	list.push_back(r);
	Mat color;
	merge(list, color);
	return color;
}

cv::Mat1b red_filter_mask(const cv::Mat& I, cv::Mat& buffer, int l1, int l2, int s_min, int v_min,
		int s_max, int v_max) {
	cv::cvtColor(I, buffer, cv::COLOR_BGR2HSV);
	Mat1b mask1, mask2;
	cv::inRange(buffer, cv::Scalar(0, s_min, v_min), cv::Scalar(l1, s_max, v_max), mask1);
	cv::inRange(buffer, cv::Scalar(l2, s_min, v_min), cv::Scalar(180, s_max, v_max), mask2);

	Mat1b mask = mask1 | mask2;
	if (I.data == buffer.data)
		cvtColor(buffer, buffer, cv::COLOR_HSV2BGR);
	return mask;
}

string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:
		r = "8U";
		break;
	case CV_8S:
		r = "8S";
		break;
	case CV_16U:
		r = "16U";
		break;
	case CV_16S:
		r = "16S";
		break;
	case CV_32S:
		r = "32S";
		break;
	case CV_32F:
		r = "32F";
		break;
	case CV_64F:
		r = "64F";
		break;
	default:
		r = "User";
		break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

bool isValidArea(const cv::VideoCapture& vc, const cv::Rect& r) {
	if (r.x < 0 || r.y < 0)
		return false;
	int w = vc.get(CV_CAP_PROP_FRAME_WIDTH);
	int h = vc.get(CV_CAP_PROP_FRAME_HEIGHT);

	return (r.x + r.width < w) && (r.y + r.height < h);
}

Mat1b generateRedFilterLookupTable(int l1, int l2, int s_min, int v_min, int s_max, int v_max) {
	Mat temp(256, 256, CV_8UC3);
	for (int j = 0; j < 256; j++) {
		uchar* P = temp.ptr<uchar>(j);
		for (int k = 0; k < 256; k++) {
			P[3 * k] = j;
			P[3 * k + 1] = k;
			P[3 * k + 2] = 235;
		}
	}
	Mat1b mask = red_filter_mask(temp, temp, l1, l2, s_min, v_min, s_max, v_max);
	return mask;

}

//Mat1b** generateRedFilterLookupTable(int l1, int l2, int s_min, int v_min,
//		int s_max, int v_max) {
//	vector<Mat> templ(256);
//
//	for (int i = 0; i < 256; i++) {
//		Mat temp(256, 256, CV_8UC3);
//		for (int j = 0; j < 256; j++) {
//			uchar* P = temp.ptr<uchar>(j);
//			for (int k = 0; k < 256; k++) {
//				P[3 * k] = i;
//				P[3 * k + 1] = j;
//				P[3 * k + 2] = k;
//			}
//		}
//		templ[i] = temp;
//	}
//	Mat1b** masks = (Mat1b**) malloc(sizeof(Mat1b*) * 256);
//	for (int i = 0; i < templ.size(); i++) {
//		masks[i] = &red_filter_mask(templ[i], templ[i], l1, l2, s_min, v_min,
//				s_max, v_max);
//	}
//	return masks;
//
//}

/**
 * Idst must be a single channel matrix.
 */
void applyMaskFromLookupTable(const Mat& I, Mat& Idst, const Mat1b& lookuptable) {

#pragma omp parallel for
	for (int i = 0; i < I.rows; i++) {
		uchar v;
		uchar r, g, b;
		const uchar* p = I.ptr<uchar>(i);
		uchar* pdst = Idst.ptr<uchar>(i);
		for (int j = 0; j < I.cols; j++) {
			r = p[3 * j + 2];
			if (r < 235) {
				pdst[j] = 0;
			} else {
				b = p[3 * j];
				g = p[3 * j + 1];
				v = lookuptable.at<uchar>(b, g);
				if (v == 0) {
					pdst[j] = 0;
				}
			}
		}
	}
}
//
///**
// * Idst must be a single channel matrix.
// */
//void applyMaskFromLookupTable(const Mat3b& I, Mat1b& Idst,
//		Mat1b** lookuptable) {
//	uchar v;
//	for (int i = 0; i < I.rows; i++) {
//		const uchar* p = I.ptr<uchar>(i);
//		uchar* pdst = Idst.ptr<uchar>(i);
//		for (int j = 0; j < I.cols; j++) {
//			Mat1b B = *(lookuptable[p[3 * j]]);
//			v = B.at<uchar>(p[3 * j + 1], p[3 * j + 2]);
//			if (v == 0) {
//				pdst[j] = 0;
//			}
//		}
//	}
//}

int removeBuffer(VideoCapture& vc) {
	int fc = 0;
	Mat src;
	while (true) {
		auto start = std::chrono::high_resolution_clock::now();
		vc >> src;
		fc++;
		auto finish = std::chrono::high_resolution_clock::now();
		int TTT = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
		if (TTT > 8) {
			break;
		}
		if (fc > 8) {
			cout << "WARNING: removeBuffer 'had removed' more than 8 frames from buffer! " << fc
					<< endl;
		}
		if (fc > 100) {
			break;
		}
	}
	return fc;
}

double mse(const Mat& img1, const Mat& img2) {
	Mat tmp;
	const double n = img1.channels() * img1.total();
	Scalar d;

	absdiff(img1, img2, tmp);
//	Scalar s = sum(tmp);
//	double avg = (s.val[0] + s.val[1] + s.val[2]) / n;
//	d = sum(cv::abs(tmp - avg));
//	double std = (d.val[0] + d.val[1] + d.val[2]) / n;

	tmp.convertTo(tmp, CV_32F);
	tmp = tmp.mul(tmp);
	Scalar s = sum(tmp);
	double sse = (s.val[0] + s.val[1] + s.val[2]) / n; // sum channels

	return sqrt(sse);
//	return sqrt(sse) * std / (avg + 1);
}

VideoWriter openVideoWriter(const VideoCapture& vc, const char* outfile) {
	Size S = Size((int) vc.get(CAP_PROP_FRAME_WIDTH), // Acquire input size
	(int) vc.get(CAP_PROP_FRAME_HEIGHT));
	int ex = static_cast<int>(vc.get(CAP_PROP_FOURCC)); // Get Codec Type- Int form
	cout << "Resolution:" << S << endl;
	cout << "codec type:" << ex << endl;
	VideoWriter outputVideo;
//	outputVideo.open(outfile, ex, vc.get(CAP_PROP_FPS), S, true);
	outputVideo.open(outfile, CV_FOURCC('M', 'J', 'P', 'G'), 60, S, true);
//	outputVideo.open(outfile, ex, 60, S, true);
	return outputVideo;
}
