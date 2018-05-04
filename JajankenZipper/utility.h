#pragma once
#include <fstream>

#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

class utility
{
public:

	static bool imageComparator(const string &file1Path, const string &file2Path) {
		Mat image1 = imread(file1Path, CV_LOAD_IMAGE_GRAYSCALE);
		Mat image2 = imread(file2Path, CV_LOAD_IMAGE_GRAYSCALE);
		if (image1.rows != image2.rows || image1.cols != image2.cols)return 0;
		int height = image1.rows;
		int width = image2.cols;
		for (int i = 0; i < height; ++i)
			for (int j = 0; j < width; ++j)
				if ((int)image1.at<uchar>(i, j) > 180 && (int)image2.at<uchar>(i, j) < 180 || (int)image1.at<uchar>(i, j) < 180 && (int)image2.at<uchar>(i, j) > 180)
					return 0;
		return 1;
	}

	static int imageSize(const string &image) {
		Mat Image = imread(image, CV_LOAD_IMAGE_GRAYSCALE);
		return Image.cols * Image.rows;
	}

	static int fileSize(const string &file) {
		ifstream File(file, std::ifstream::binary);
		File.seekg(0, File.end);
		int size = File.tellg();
		File.seekg(0, File.beg);
		return size;
	}

};

