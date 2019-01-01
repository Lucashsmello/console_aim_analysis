/*
 * ImageUtils.h
 *
 *  Created on: 17 de set de 2017
 *      Author: lmello
 */

#ifndef CPP_UTILS_IMAGEUTILS_H_
#define CPP_UTILS_IMAGEUTILS_H_

#include <opencv2/videoio.hpp>
#include <string>

cv::Mat1b red_filter_mask(const cv::Mat& I, cv::Mat& buffer, int l1, int l2,
		int s_min, int v_min, int s_max = 255, int v_max = 255);

void red_filter(cv::Mat& I, int l1 = 8, int l2 = 165, int s = 110);
void red_filter(cv::Mat& I, int l1, int l2, int s_min, int v_min, int s_max =
		255, int v_max = 255);
void red_filter(cv::Mat& I, cv::Mat& buffer, int l1, int l2, int s_min,
		int v_min, int s_max = 255, int v_max = 255);

void apply_mask(cv::Mat& I, const cv::Mat1b& mask);

void drawGreenCross(cv::Mat& I, cv::Point p, int size = 1, int length = -1);
void drawCross(cv::Mat& I, cv::Point p,  uchar r, uchar g, uchar b, int size=1, int length=-1);

void my_imshow(const cv::Mat& m);

void makeBorder(cv::Mat& img, int top, int bottom, int left, int right);
void makeBorder(cv::Mat& img, const cv::Rect& r);

bool isValidArea(const cv::Mat& m, const cv::Rect& r);
bool isValidArea(const cv::VideoCapture& vc, const cv::Rect& r);

void apply_distmask(cv::Mat& src, const cv::Mat& mask, cv::Point p);
void apply_distmask(cv::Mat& src, const cv::Mat& mask);

cv::Mat coloredImage(const cv::Mat& r);

std::string type2str(int type);

cv::Mat1b generateRedFilterLookupTable(int l1, int l2, int s_min,
		int v_min, int s_max = 255, int v_max = 255);

void applyMaskFromLookupTable(const cv::Mat& I, cv::Mat& Idst,
		const cv::Mat1b& lookuptable);

int removeBuffer(cv::VideoCapture& vc);
cv::VideoWriter openVideoWriter(const cv::VideoCapture& vc, const char* outfile);
double mse(const cv::Mat& img1, const cv::Mat& img2);

cv::Mat equalizeIntensity(const cv::Mat& inputImage);


#endif /* CPP_UTILS_IMAGEUTILS_H_ */
