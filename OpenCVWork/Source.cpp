#include <iostream>
#include <cv.hpp>
#include <highgui\highgui.hpp>

using namespace std;
using namespace cv;

const int FILE_NUMBER = 9;

#define __RATIO__
#define __INVERT__
#define __CONTRAST__
#define __HISTOGRAM__
#define __THRESHOLD__

#define str(n) #n
#define getWinName(num) str(Window##num)

const string PATH = "./";
const string FILE_NAME[] = {"Baboon.BMP", "color bar.bmp", "FRUIT.BMP", "lya-lc.bmp", "lya-lc-bright.bmp", "lya-lc-dark.bmp", "lya-lc-안되는예.bmp", "RGB_Colorcube_Corner_White.bmp", "지문-gray.bmp"};

const int ContrastMin = 10, ContrastMax = 245;
const int ContrastIgnoreMin = 10, ContrastIgnoreMax = 245;

const int ThresholdValue = 170;

const float Ratio[2] = { 0.7 , 0.2};

void main(void) {

	Mat Image[FILE_NUMBER];
	Mat temp, temp2;
	int max, min;

	//load image
	int i = 0;
	for (auto str : FILE_NAME) {
		str = PATH + str;
		cout << "Load image at : " << str << endl;
		Image[i] = cvLoadImage(str.c_str(), CV_LOAD_IMAGE_UNCHANGED);
		++i;
	}
	cout << endl;

#ifdef __RATIO__
	//calc
	temp = Image[1].clone();
	//cvtColor(temp, temp, CV_RGB2GRAY);
	temp *= Ratio[0];
	temp2 = Image[7].clone();
	//cvtColor(temp2, temp2, CV_RGB2GRAY);
	temp2 *= Ratio[1];

	cout << "Add two image with ratio" << endl;
	//draw
	imshow(getWinName(0), temp + temp2);
	waitKey(0);

	cout << "Subtract two image with ratio" << endl;
	//calc
	temp = Image[3].clone();
	cvtColor(temp, temp, CV_RGB2GRAY);
	temp *= Ratio[0];
	temp2 = Image[6].clone();
	cvtColor(temp2, temp2, CV_RGB2GRAY);
	temp2 *= 1.0 - Ratio[1];

	//draw
	imshow(getWinName(0), temp - temp2);
	waitKey(0);
#endif

#ifdef __INVERT__
	//invert
	cout << "Invert image" << endl << endl;
	temp = Image[7].clone();

	//processing
	for (int i = 0; i < temp.cols; ++i) {
		for (int j = 0; j < temp.rows; ++j) {
		temp.at<Vec3b>(i, j)[0] = 255 - temp.at<Vec3b>(i, j)[0];
		temp.at<Vec3b>(i, j)[1] = 255 - temp.at<Vec3b>(i, j)[1];
		temp.at<Vec3b>(i, j)[2] = 255 - temp.at<Vec3b>(i, j)[2];
		}
	}

	//draw
	imshow(getWinName(0), Image[7]);
	imshow(getWinName(1), temp);
	waitKey(0);
#endif

#ifdef __CONTRAST__
	//contrast stretching	image from 3 to 6
	cout << "Stretching contrast" << endl;
	temp = Image[6].clone();
	cv::cvtColor(temp, temp, CV_RGB2GRAY);
	cv::imshow(getWinName(0), temp);
	min = 256, max = -1;
	int tempvalue;

	//processing

	//calcurate minimum contrast
	for (int i = 0; i < temp.rows; ++i) {
		for (int j = 0; j < temp.cols; ++j) {
			tempvalue = temp.at<uchar>(i, j);

			if (tempvalue <= ContrastIgnoreMin) {
				continue;
			}

			if (tempvalue <= ContrastMin) {
				min = ContrastMin;
				i = temp.cols;
				break;
			}

			min = (min < tempvalue) ? min : tempvalue;
		}
	}

	//contrast maximum
	for (int i = 0; i < temp.rows; ++i) {
		for (int j = 0; j < temp.cols; ++j) {
			tempvalue = temp.at<uchar>(i, j);

			if (tempvalue >= ContrastIgnoreMax) {
				continue;
			}

			if (tempvalue >= ContrastMax) {
				max = ContrastMax;
				i = temp.cols;
				break;
			}

			max = (max > tempvalue) ? max : tempvalue;
		}
	}

	//change contrast with calcurated contrast
	for (int i = 0; i < temp.rows; ++i) {
		for (int j = 0; j < temp.cols; ++j) {
			tempvalue = temp.at<uchar>(i, j);
			if (tempvalue <= min) {
				temp.at<uchar>(i, j) = 0;
			} else if (tempvalue < max) {
				temp.at<uchar>(i, j) = (tempvalue - min) * 255 / (max - min);
			} else {
				temp.at<uchar>(i, j) = 255;
			}
		}
	}

	//draw
	cout << "Contrast detected from " << min << " to " << max << endl << endl;
	
	cv::imshow(getWinName(1), temp);
	cv::waitKey(0);
#endif

#ifdef __HISTOGRAM__
	cout << "calculate histogram" << endl;
	temp = Image[2].clone();
	cv::cvtColor(temp, temp, CV_RGB2GRAY);

	int histogram[256] = { 0 };
	int hist_w = 256, hist_h = 400;
	int bin_w = cvRound((double)hist_w / 256);

	Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(255));

	//calc histogram
	for (int i = 0; i < temp.rows; ++i) {
		for (int j = 0; j < temp.cols; ++j) {
			histogram[(int)temp.at<uchar>(i, j)]++;
		}
	}

	//find maximum
	max = histogram[0];
	for (int i = 1; i < 257; ++i) {
		if (max < histogram[i]) {
			max = histogram[i];
		}
	}

	//visualize
	for (int i = 0; i < 256; ++i) {
		line(histImage, Point(bin_w*i, hist_h), Point(bin_w*i, (int)(hist_h - ((double)histogram[i] / max)*histImage.rows)), Scalar(0));
	}

	//draw
	imshow(getWinName(0), histImage);
	imshow(getWinName(1), temp);
	waitKey(0);

	//calc equalization
	cout << "Equalization start" << endl;
	float N = (float)(temp.rows * temp.cols) / 256;
	int Hsum[256] = { 0 };
	int acc_hist = 0;

	for (int i = 0; i < 256; ++i) {
		acc_hist += histogram[i];
		Hsum[i] = (int)((int)acc_hist / N);
	}

	for (int i = 0; i < temp.rows; ++i) {
		for (int j = 0; j < temp.cols; ++j) {
			temp.at<uchar>(i, j) = Hsum[temp.at<uchar>(i, j)];
		}
	}

	cout << "Recalcurate histogram" << endl;
	//calc histogram
	for (int i = 0; i < temp.rows; ++i) {
		for (int j = 0; j < temp.cols; ++j) {
			histogram[(int)temp.at<uchar>(i, j)]++;
		}
	}

	//find maximum
	max = histogram[0];
	for (int i = 1; i < 257; ++i) {
		if (max < histogram[i]) {
			max = histogram[i];
		}
	}

	//visualize
	for (int i = 0; i < 256; ++i) {
		line(histImage, Point(bin_w*i, hist_h), Point(bin_w*i, (int)(hist_h - ((double)histogram[i] / max)*histImage.rows)), Scalar(0));
	}

	cout << endl;
	//draw
	imshow(getWinName(0), histImage);
	imshow(getWinName(1), temp);
	waitKey(0);
#endif

#ifdef __THRESHOLD__
	cout << "Calculate threshold" << endl << endl;
	//load
	temp = Image[8].clone();
//	cvtColor(temp, temp, CV_RGB2GRAY);

	//draw
	imshow(getWinName(0), temp);

	//calc
	for (int i = 0; i < temp.rows; ++i) {
		for (int j = 0; j < temp.cols; ++j) {
			if (temp.at<uchar>(i, j) <= ThresholdValue) {
				temp.at<uchar>(i, j) = 0;
			} else {
				temp.at<uchar>(i, j) = 255;
			}
		}
	}

	//draw
	imshow(getWinName(1), temp);
	waitKey(0);
#endif

	destroyAllWindows();
}
