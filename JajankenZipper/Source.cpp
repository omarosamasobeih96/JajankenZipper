#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
using namespace std;


#include "JajankenZipper.h"
#include "utility.h"



inline void getFilesInDirectory(const string& directory, vector<string>& files) {
	string s = "dir " + directory + " -b > dirs.txt";
	system(s.c_str());

	ifstream fin("dirs.txt");

	while (getline(fin, s)) {
		if (s.length() >= 4 && s.substr(s.length() - 4, 4) == ".jpg") {
			string k = "", y;
			for (int i = s.length() - 5; i >= 0 && s[i] != ' '; --i) {
				k += s[i];
			}
			reverse(k.begin(), k.end());
			files.push_back(k);
		}
	}
	remove("dirs.txt");
}


int main() {
	ofstream fout("ratios.txt");

	vector<string> files;
	getFilesInDirectory("DataSet", files);
	int totalTime = 0;
	double compressedSize = 0, originalSize = 0;

	for (string x : files) {
		int start_s = clock();

		string name = "DataSet/" + x + ".jpg";
		string newimage = "Decompressed/" + x + "new" + ".jpg";
		string data = "Compressed/" + x + "_encoded" + ".txt";

		JajankenZipper::Compress(name, data);

		JajankenZipper::Decompress(data, newimage);

		string message = utility::imageComparator(name, newimage) ? " Successful Compression " : " Incorrect compression ....  ";

		cout << x + message;
		fout << x + message;

		int stop_s = clock();
		totalTime += stop_s - start_s;
		int timeval = (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << timeval << " milliseconds ";
		fout << timeval << " milliseconds ";
		double imagesize = utility::imageSize(name);
		double filesize = utility::fileSize(data);
		originalSize += imagesize;
		compressedSize += filesize;
		cout << "Compression ratio = " << imagesize / filesize << endl;
		fout << "Compression ratio = " << imagesize / filesize << " file size = " << filesize  << " bytes"<< endl;
	}
	cout << endl << "time in milliseconds: " << totalTime << endl;
	fout << endl << "time in milliseconds: " << totalTime << endl;
	cout << "time in seconds: " << totalTime / 1000 << endl;
	fout << "time in seconds: " << totalTime / 1000 << endl;
	cout << "Average ratio: " << originalSize / compressedSize << endl;
	fout << "Average ratio: " << originalSize / compressedSize << endl;
	fout << "Total file sizes " << compressedSize << " bytes\n"; 
	return 0;
}