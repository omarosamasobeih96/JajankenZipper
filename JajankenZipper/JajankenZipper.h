#pragma once

#include<opencv2/opencv.hpp>
using namespace cv;

#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
#include <sstream>
using namespace std;



class JajankenZipper
{
	// huffman tree node
	template <class T>
	struct node {
		node* left, *right;
		string codeWord;
		T symbol;
		node() { left = right = NULL; }
		node(T c) { symbol = c; left = right = NULL; }
	};

	struct point {
		int x, y;
		point(int x, int y) {
			this->x = x, this->y = y;
		}
		bool operator < (const point& p) {
			return x < p.x || x == p.x && y < p.y;
		}
	};

	struct line {
		int startPoint;
		int length;
		line(int startPoint, int lenght) {
			this->startPoint = startPoint;
			this->length = lenght;
		}
		bool operator == (const line& l)const {
			return (this->startPoint == l.startPoint && this->length == l.length);
		}
		bool operator != (const line& l)const {
			return !(*this == l);
		}
	};

	struct shape {
		vector<int> shapeDescription;
		int min_x, min_y;
		shape() {}
		shape(vector<int> shapeRLC, int min_x, int min_y) {
			shapeDescription = shapeRLC;
			this->min_x = min_x;
			this->min_y = min_y;
		}
		shape(vector <point> shapepixels, int height, int width) {
			min_x = height, min_y = width;
			int max_x = 0, max_y = 0;

			for (point j : shapepixels) {
				min_x = min(min_x, j.x);
				min_y = min(min_y, j.y);
				max_x = max(max_x, j.x);
				max_y = max(max_y, j.y);
			}

			height = max_x - min_x + 1;
			width = max_y - min_y + 1;

			bool **myimage = new bool*[height];
			for (int i = 0; i < height; ++i) {
				myimage[i] = new bool[width];
				for (int j = 0; j < width; ++j)
					myimage[i][j] = false;
			}


			for (point j : shapepixels) {
				myimage[j.x - min_x][j.y - min_y] = true;
			}

			RunLengthEncoding(myimage, height, width);

			for (int i = 0; i < height; ++i)delete[]myimage[i];
			delete[]myimage;
		}
		// check if two shapes are similar
		bool operator == (const shape& s)const {
			return shapeDescription == s.shapeDescription;
		}
		bool operator != (const shape& s)const {
			return shapeDescription != s.shapeDescription;
		}
		shape& operator = (const shape& s) {
			shapeDescription = s.shapeDescription;
			min_x = s.min_x;
			min_y = s.min_y;
			return *this;
		}


		void RunLengthDecoding(bool **originalimage) {
			int height = shapeDescription[0];
			int width = shapeDescription[1];

			bool **matrix = new bool*[height];
			for (int i = 0; i < height; ++i)matrix[i] = new bool[width];

			int sum = 0;
			for (int x : shapeDescription)sum += x;

			sum = height * width - sum + height + width;
			shapeDescription.push_back(sum);

			int i = 0, j = 0, l = 0;
			bool p = 1;
			for (int iter = 2; iter < shapeDescription.size(); ++iter) {
				int x = shapeDescription[iter];
				for (int c = 0; c < x; ++c) {
					matrix[i][j] = p;
					if (++j == width)j = 0, ++i;
				}
				p = 1 - p;
			}

			for (int i = 0; i < height; ++i)
				for (int j = 0; j < width; ++j)
					if (matrix[i][j])originalimage[i + min_x][j + min_y] = false;

			shapeDescription.pop_back();

			for (int i = 0; i < height; ++i)delete[]matrix[i];
			delete[]matrix;
		}
		void RunLengthEncoding(bool** matrix, const int &height, const int &width) {
			shapeDescription.push_back(height);
			shapeDescription.push_back(width);
			int i = 0, j = 0, l = 0, p = 1;
			while (i < height) {
				if (matrix[i][j] == p)++l;
				else shapeDescription.push_back(l), p = matrix[i][j], l = 1;
				if (++j == width)j = 0, ++i;
			}
		}
	};

	static vector<uchar> WriteCodeAndData(map<string, uchar>* huffmanCode, const vector<uchar> &data);
	static void ReadCodeAndData(map<string, uchar> &huffmanCode, vector<uchar> &data, const vector<uchar> &encoded);

	/*Returns a MAT of the gray scale image in the path imagePath*/	
	static Mat ReadImage(const string &imagePath);

	/*Writes a binary image having height and width represented in the 2D matrix in file imagePath*/
	static void WriteImage(const string &imagePath, bool** matrix, const int &height, const int &width);

	/*Gets the dimensions of a MAT*/
	static void GetDimensions(const Mat &image, int &height, int &width);

	/*Fill a boolean matrix with pixel binary values of a grayscale image*/
	static void GetImage(const Mat &image, bool** matrix);


	/*Writes vector of unsigned chars to file*/
	static void WriteFile(const vector<uchar> &data, const string &filePath);

	/*Reads from a file to vector of unsigned chars*/
	static void ReadFile(vector<uchar> &data, const string &filePath);


	template <class T>
	static vector<uchar> WriteCodeAndData(map<string, T>* huffmanCode, const vector<uchar> &data);
	template <class T>
	static void ReadCodeAndData(map<string, T> &huffmanCode, vector<uchar> &data, const vector<uchar> &encoded);


	static vector <uchar> EncodeEncoded(vector <uchar> data);
	static void DecodeEncoded(vector<uchar> &encoded);


	template <class T>
	/*Performs Huffman Encoding to compress data and returns map code to symbol*/
	static map<string, T> HuffmanEncoding(const vector<T> &input, vector<uchar> &v);
	template <class T>
	/*Performs Huffman Decoding to decompress data using the built map*/
	static vector<T> HuffmanDecoding(const vector<uchar> &v, map<string, T>* code);

	template <class T>
	/*Read data from vector of chars*/
	static void ReadDataFromVecOfCharX(const vector<char> &data, vector<T> &v, int &height, int &width);

	template <class T>
	/*Write data to vector of chars*/
	static vector<char> PrintDataToVecOfChar(const vector<T> &v, const int &height, const int &width);

	template <class T>
	/*Read data from vector of chars*/
	static void ReadDataFromVecOfCharX(const vector<char> &data, vector<T> &v, int &length);

	template <class T>
	/*Write data to vector of chars*/
	static vector<char> PrintDataToVecOfChar(const vector<T> &v, const int &length);


	template <class T>
	/*Writes HuffmanTable and data to file*/
	static void WriteCodeAndData(map<string, T>* huffmanCode, const vector<uchar> &data, const string &dataPath);
	template <class T>
	/*Reads HuffmanTable and data from file*/
	static void ReadCodeAndData(map<string, T> &huffmanCode, vector<uchar> &data, const string &dataPath);


	/*Encode bitstring to vector of unsigned chars*/
	static void BitStringEncoding(const string &bitString, vector<uchar> &code);

	/*Process a boolean matrix and Return connected true as shapes*/
	static vector<shape> DetectShapes(bool** matrix, const int &height, const int &width);
	/*Writes a boolean matrix from shapes*/
	static void RenderShapes(vector<shape> shapes, bool** matrix, const int &height, const int &width);

	/*Compresses shapes to distinct shapes and ids*/
	static pair<vector<int>, vector<int> >ShapesCompress(const vector<shape> &data, const int &width);
	/*Decompresses shapes from distinct shapes and ids*/
	static vector<shape> ShapesDecompress(const pair<vector<int>, vector<int> > &data, const int &width);

	static bool SortDistincts(pair<shape, vector<int>> a, pair<shape, vector<int>> b);

	/*Returns vector of distinct shapes and positions of each*/
	static vector<pair<shape, vector<int>>> Distincts(const vector<shape> &shapes);

	/*Writes HuffmanTable and data to file, works better on chars*/
	static vector<uchar> S_WriteCodeAndData(map<string, char>* huffmanCode, const vector<uchar> &data);
	/*Reads HuffmanTable and data from file, works better on chars*/
	static void S_ReadCodeAndData(map<string, char> &huffmanCode, vector<uchar> &data, const vector<uchar> &encoded);


public:

	/*Compress image in imagePath, works better for scanned text*/
	static void Compress(const string & imagePath, const string &dataPath);

	/*Decompress data and write image in imagePath, works better for scanned text*/
	static void Decompress(const string & dataPath, const string &imagePath);

};

