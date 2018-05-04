#include "JajankenZipper.h"

void JajankenZipper::Compress(const string & imagePath, const string &dataPath) {
	Mat image = ReadImage(imagePath);
	int height, width;
	GetDimensions(image, height, width);
	bool **matrix = new bool*[height];
	for (int i = 0; i < height; ++i)matrix[i] = new bool[width];
	GetImage(image, matrix);

	vector<shape> myImage = DetectShapes(matrix, height, width);

	pair<vector<int>, vector<int> > compressed = ShapesCompress(myImage, width);

	vector<uchar> finaldata;

	vector<uchar> data1;
	map<string, int> HuffmanMapI = HuffmanEncoding(compressed.first, data1);
	vector<uchar> file1 = WriteCodeAndData(&HuffmanMapI, data1);

	vector<uchar> data2;
	map<string, char> HuffmanMap = HuffmanEncoding(PrintDataToVecOfChar(compressed.second, height, width), data2);
	vector<uchar> file2 = S_WriteCodeAndData(&HuffmanMap, data2);

	int val = file2.size();
	uchar c = (val & 0xff000000) >> 24;
	finaldata.push_back(c);
	c = (val & 0x00ff0000) >> 16;
	finaldata.push_back(c);
	c = (val & 0x0000ff00) >> 8;
	finaldata.push_back(c);
	c = (val & 0x000000ff);
	finaldata.push_back(c);

	for (uchar x : file2)finaldata.push_back(x);
	for (uchar x : file1)finaldata.push_back(x);


	WriteFile(EncodeEncoded(finaldata), dataPath);

	for (int i = 0; i < height; ++i)delete[]matrix[i];
	delete[]matrix;
}

void JajankenZipper::Decompress(const string & dataPath, const string &imagePath) {
	int height, width;

	vector<uchar> data, data2, encoded;
	vector<int> decompressed, decompressed2;

	map<string, char> HuffmanMap;
	map<string, int> HuffmanMapI;

	ReadFile(encoded, dataPath);
	DecodeEncoded(encoded);


	vector <uchar> code1, code2;
	int codeSize = encoded[0] << 24;
	codeSize |= encoded[1] << 16;
	codeSize |= encoded[2] << 8;
	codeSize |= encoded[3];
	for (int i = 0; i < codeSize; ++i)code1.push_back(encoded[i + 4]);
	for (int i = codeSize + 4; i < encoded.size(); ++i)code2.push_back(encoded[i]);


	S_ReadCodeAndData(HuffmanMap, data, code1);
	ReadDataFromVecOfCharX(HuffmanDecoding(data, &HuffmanMap), decompressed, height, width);

	ReadCodeAndData(HuffmanMapI, data2, code2);
	decompressed2 = HuffmanDecoding(data2, &HuffmanMapI);

	bool **matrix = new bool*[height];
	for (int i = 0; i < height; ++i)matrix[i] = new bool[width];

	RenderShapes(ShapesDecompress({ decompressed2, decompressed }, width), matrix, height, width);

	WriteImage(imagePath, matrix, height, width);

	for (int i = 0; i < height; ++i)delete[]matrix[i];
	delete[]matrix;
}

Mat JajankenZipper::ReadImage(const string &imagePath) {
	return imread(imagePath, CV_LOAD_IMAGE_GRAYSCALE);
}

void JajankenZipper::WriteImage(const string &imagePath, bool** matrix, const int &height, const int &width) {
	Mat Image(height, width, CV_8UC1, 255);
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j)
			Image.at<uchar>(i, j) = matrix[i][j] ? 255 : 0;
	imwrite(imagePath, Image);
}

void JajankenZipper::GetDimensions(const Mat &image, int &height, int &width) {
	height = image.rows;
	width = image.cols;
}

void JajankenZipper::GetImage(const Mat &image, bool** matrix) {
	int height = image.rows;
	int width = image.cols;
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j)
			matrix[i][j] = (int)image.at<uchar>(i, j) > 180;
}
template <class T>
map<string, T> JajankenZipper::HuffmanEncoding(const vector<T> &input, vector<uchar> &v) {
	node<T> *front;
	{
		priority_queue<pair<int, node<T>*>>q;
		{
			map<T, int> mp;
			for (T x : input)mp[x]++;
			map<T, int>::iterator it;
			for (it = mp.begin(); it != mp.end(); ++it) {
				node<T> *n = new node<T>(it->first);
				q.push({ -it->second, n });
			}
		}

		{
			int size = q.size();
			while (--size) {
				node<T> *s1 = q.top().second;
				int f = q.top().first;
				q.pop();
				node<T> *s2 = q.top().second;
				f += q.top().first;
				q.pop();
				node<T> *n = new node<T>();
				n->right = s1, n->left = s2;
				q.push({ f, n });
			}
			front = q.top().second;
			q.pop();
			front->codeWord = "";
		}
	}

	map<T, string>code;
	{
		queue<node<T>*>t;
		t.push(front);
		while (!t.empty()) {
			node<T> * temp = t.front();
			if (temp->left == NULL || temp->right == NULL) {
				code[temp->symbol] = temp->codeWord;
				t.pop();
				delete temp;
				continue;
			}
			temp->left->codeWord = temp->codeWord + '0';
			t.push(temp->left);
			temp->right->codeWord = temp->codeWord + '1';
			t.push(temp->right);
			t.pop();
			delete temp;
		}
	}

	{
		short l = 0, r = 0;
		for (T x : input) {
			string temp = code[x];
			for (char y : temp) {
				bool i = y == '1';
				r |= (i << (7 - l));
				if (++l == 8) { l = 0, v.push_back(r); r = 0; }
			}
		}
		if (l)v.push_back(r);
		if (l)v.push_back(l + '0');
		else v.push_back('8');
	}

	map<string, T> ret;
	{
		map<T, string>::iterator it;
		for (it = code.begin(); it != code.end(); ++it)ret[it->second] = it->first;
	}

	return ret;
}

template <class T>
vector<T> JajankenZipper::HuffmanDecoding(const vector<uchar> &v, map<string, T>* code) {
	// Function Not Used
	string k = "";
	vector<T> ret;
	for (int j = 0; j + 1 < v.size(); ++j) {
		unsigned char x = v[j];
		short u = j + 2 < v.size() ? 0 : 8 - v[v.size() - 1] + '0';
		for (short i = 7; i >= u; --i) {
			bool b = (1 << i)&x;
			k += b + '0';
			if (code->count(k))
				ret.push_back((*code)[k]), k = "";
		}
	}
	return ret;
}

template <class T>
void JajankenZipper::WriteCodeAndData(map<string, T>* huffmanCode, const vector<uchar> &data, const string &dataPath) {
	vector <uchar> code;
	for (map<string, T>::iterator it = huffmanCode->begin(); it != huffmanCode->end(); ++it) {
		code.push_back(it->first.size());
		BitStringEncoding(it->first, code);
		int val = it->second;
		uchar c = (val & 0x00ff0000) >> 16;
		code.push_back(c);
		c = (val & 0x0000ff00) >> 8;
		code.push_back(c);
		c = (val & 0x000000ff);
		code.push_back(c);
	}

	vector<uchar> encoded;
	int val = code.size();
	uchar c = (val & 0x00ff0000) >> 16;
	encoded.push_back(c);
	c = (val & 0x0000ff00) >> 8;
	encoded.push_back(c);
	c = (val & 0x000000ff);
	encoded.push_back(c);

	for (uchar x : code)encoded.push_back(x);
	for (uchar x : data)encoded.push_back(x);

	WriteFile(encoded, dataPath);
}

template <class T>
void JajankenZipper::ReadCodeAndData(map<string, T> &huffmanCode, vector<uchar> &data, const string &dataPath) {
	vector<uchar> encoded;
	vector <uchar> code;
	ReadFile(encoded, dataPath);
	int codeSize = encoded[0] << 16;
	codeSize |= encoded[1] << 8;
	codeSize |= encoded[2];
	for (int i = 0; i < codeSize; ++i)code.push_back(encoded[i + 3]);
	for (int i = codeSize + 3; i < encoded.size(); ++i)data.push_back(encoded[i]);


	for (int i = 0; i < code.size(); ++i) {
		int l = code[i];
		int n = (l + 7) / 8;
		string k = "";
		for (int c = 0; c < n; ++c) {
			++i;
			for (int j = 7; j >= 0 && l; --j, --l)
				k += code[i] & (1 << j) ? '1' : '0';
		}
		int val = code[++i] << 16;
		val |= code[++i] << 8;
		val |= code[++i];
		huffmanCode[k] = val;
	}
}

template <class T>
vector<uchar> JajankenZipper::WriteCodeAndData(map<string, T>* huffmanCode, const vector<uchar> &data) {
	vector <uchar> code;
	for (map<string, T>::iterator it = huffmanCode->begin(); it != huffmanCode->end(); ++it) {
		code.push_back(it->first.size());
		BitStringEncoding(it->first, code);
		int val = it->second;
		uchar c = (val & 0x00ff0000) >> 16;
		code.push_back(c);
		c = (val & 0x0000ff00) >> 8;
		code.push_back(c);
		c = (val & 0x000000ff);
		code.push_back(c);
	}

	vector<uchar> encoded;
	int val = code.size();
	uchar c = (val & 0x00ff0000) >> 16;
	encoded.push_back(c);
	c = (val & 0x0000ff00) >> 8;
	encoded.push_back(c);
	c = (val & 0x000000ff);
	encoded.push_back(c);

	for (uchar x : code)encoded.push_back(x);
	for (uchar x : data)encoded.push_back(x);

	return encoded;
}

template <class T>
void JajankenZipper::ReadCodeAndData(map<string, T> &huffmanCode, vector<uchar> &data, const vector<uchar> &encoded) {
	vector <uchar> code;
	int codeSize = encoded[0] << 16;
	codeSize |= encoded[1] << 8;
	codeSize |= encoded[2];
	for (int i = 0; i < codeSize; ++i)code.push_back(encoded[i + 3]);
	for (int i = codeSize + 3; i < encoded.size(); ++i)data.push_back(encoded[i]);


	for (int i = 0; i < code.size(); ++i) {
		int l = code[i];
		int n = (l + 7) / 8;
		string k = "";
		for (int c = 0; c < n; ++c) {
			++i;
			for (int j = 7; j >= 0 && l; --j, --l)
				k += code[i] & (1 << j) ? '1' : '0';
		}
		int val = code[++i] << 16;
		val |= code[++i] << 8;
		val |= code[++i];
		huffmanCode[k] = val;
	}
}
static bool valid(int x, int y, int w, int h)
{
	return x >= 0 && x < h && y >= 0 && y < w;
}

vector<JajankenZipper::shape> JajankenZipper::DetectShapes(bool** matrix, const int &height, const int &width) {
	vector<shape> ret;

	bool** vis = new bool*[height];
	for (int i = 0; i < height; ++i) {
		vis[i] = new bool[width];
		for (int j = 0; j < width; ++j)
			vis[i][j] = false;
	}

	bool** borders = new bool*[height];
	for (int i = 0; i < height; ++i) {
		borders[i] = new bool[width];
		for (int j = 0; j < width; ++j)
			borders[i][j] = false;
	}

	int dx[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	int maxi = 0;
	ofstream file;
	file.open("out.txt");
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j) {

			if (matrix[i][j])continue;
			if (vis[i][j])continue;

			vector<point> shape;
			point p(i, j);
			queue<point>q;
			q.push(p);
			vis[p.x][p.y] = true;

			while (!q.empty()) {
				p = q.front();
				q.pop();

				shape.push_back(p);

				for (int k = 0; k < 8; ++k) {
					if (!valid(p.x + dx[k], p.y + dy[k], width, height)) continue;
					point np(p.x + dx[k], p.y + dy[k]);
					if (!vis[np.x][np.y] && !matrix[np.x][np.y]) {
						vis[np.x][np.y] = true;
						q.push(np);
					}
				}
			}
			ret.push_back(JajankenZipper::shape(shape, height, width));
		}
	file.close();
	for (int i = 0; i < height; ++i)delete[]vis[i];
	delete[] vis;
	for (int i = 0; i < height; ++i)delete[]borders[i];
	delete[] borders;

	return ret;
}

void JajankenZipper::RenderShapes(vector<shape> shapes, bool** matrix, const int &height, const int &width) {
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width; ++j)
			matrix[i][j] = true;

	for (shape shape : shapes) {
		shape.RunLengthDecoding(matrix);
	}
}

bool JajankenZipper::SortDistincts(pair<shape, vector<int>> a, pair<shape, vector<int>> b) {
	return a.second.size() > b.second.size();
}

pair<vector<int>, vector<int> > JajankenZipper::ShapesCompress(const vector<shape> &data, const int &width) {
	vector<int> ret1;
	vector<int> ret2;
	vector<pair<shape, vector<int>>> distincts = Distincts(data);

	ret1.push_back(distincts.size());

	int flag = 0, position;

	for (pair<shape, vector<int>> shape : distincts) {

		ret1.push_back(shape.first.shapeDescription.size());

		for (int x : shape.first.shapeDescription)ret2.push_back(x);

		ret1.push_back(shape.second.size());

		if (flag++)ret1.push_back(data[shape.second[0]].min_x * width + data[shape.second[0]].min_y - position);
		else ret1.push_back(position = data[shape.second[0]].min_x * width + data[shape.second[0]].min_y);

		position = data[shape.second[0]].min_x * width + data[shape.second[0]].min_y;

		for (int j = 1; j < shape.second.size(); ++j) {
			int i = shape.second[j];
			int k = shape.second[j - 1];
			ret1.push_back((data[i].min_x - data[k].min_x) * width + data[i].min_y - data[k].min_y);
		}
	}

	return{ ret2, ret1 };
}

vector<JajankenZipper::shape> JajankenZipper::ShapesDecompress(const pair<vector<int>, vector<int> > &data, const int &width) {
	vector<shape> ret;
	int c = -1, c2 = 0;

	int distinctsCount = data.second[0];

	int flag = 0, position;

	for (int i = 0; i < distinctsCount; ++i) {

		int shapeDescSize = data.second[++c2];

		vector<int> shapeconst;

		for (int i = 0; i < shapeDescSize; ++i)shapeconst.push_back(data.first[++c]);

		int idCount = data.second[++c2];

		int coordinates = data.second[++c2];

		if (flag++)coordinates += position;

		position = coordinates;

		ret.push_back(JajankenZipper::shape(shapeconst, coordinates / width, coordinates % width));

		for (int j = 1; j < idCount; ++j) {
			coordinates += data.second[++c2];
			ret.push_back(JajankenZipper::shape(shapeconst, coordinates / width, coordinates % width));
		}
	}
	return ret;
}

vector<pair<JajankenZipper::shape, vector<int>>> JajankenZipper::Distincts(const vector<shape> &shapes) {
	vector<pair<shape, vector<int>>> ret;

	bool *vis = new bool[shapes.size()];
	for (int i = 0; i < shapes.size(); ++i)vis[i] = false;

	for (int i = 0; i < shapes.size(); ++i) {
		if (vis[i])continue;

		vector<int> positions;
		positions.push_back(i);

		for (int j = i + 1; j < shapes.size(); ++j) {
			if (vis[j])continue;

			if (shapes[i] == shapes[j]) {
				positions.push_back(j);
				vis[j] = true;
			}
		}

		ret.push_back({ shapes[i], positions });
	}


	delete[]vis;
	return ret;
}

vector<uchar> JajankenZipper::S_WriteCodeAndData(map<string, char>* huffmanCode, const vector<uchar> &data) {
	vector <uchar> code;
	for (map<string, char>::iterator it = huffmanCode->begin(); it != huffmanCode->end(); ++it) {
		code.push_back(it->first.size());
		BitStringEncoding(it->first, code);
		code.push_back(it->second);
	}

	vector<uchar> encoded;
	encoded.push_back(code.size());

	for (uchar x : code)encoded.push_back(x);
	for (uchar x : data)encoded.push_back(x);

	return encoded;
}


vector<uchar> JajankenZipper::WriteCodeAndData(map<string, uchar>* huffmanCode, const vector<uchar> &data) {
	vector <uchar> code;
	for (map<string, uchar>::iterator it = huffmanCode->begin(); it != huffmanCode->end(); ++it) {
		code.push_back(it->first.size());
		BitStringEncoding(it->first, code);
		code.push_back(it->second);
	}

	vector<uchar> encoded;
	int val = code.size();
	uchar c = (val & 0x0000ff00) >> 8;
	encoded.push_back(c);
	c = (val & 0x000000ff);
	encoded.push_back(c);

	for (uchar x : code)encoded.push_back(x);
	for (uchar x : data)encoded.push_back(x);

	return encoded;
}


void JajankenZipper::S_ReadCodeAndData(map<string, char> &huffmanCode, vector<uchar> &data, const vector<uchar> &encoded) {
	vector <uchar> code;
	int codeSize = encoded[0];

	for (int i = 0; i < codeSize; ++i)code.push_back(encoded[i + 1]);
	for (int i = codeSize + 1; i < encoded.size(); ++i)data.push_back(encoded[i]);

	for (int i = 0; i < code.size(); ++i) {
		int l = code[i];
		int n = (l + 7) / 8;
		string k = "";
		for (int c = 0; c < n; ++c) {
			++i;
			for (int j = 7; j >= 0 && l; --j, --l)
				k += code[i] & (1 << j) ? '1' : '0';
		}
		huffmanCode[k] = code[++i];
	}
}

void JajankenZipper::ReadCodeAndData(map<string, uchar> &huffmanCode, vector<uchar> &data, const vector<uchar> &encoded) {
	vector <uchar> code;
	int codeSize = encoded[0] << 8;
	codeSize |= encoded[1];
	for (int i = 0; i < codeSize; ++i)code.push_back(encoded[i + 2]);
	for (int i = codeSize + 2; i < encoded.size(); ++i)data.push_back(encoded[i]);

	for (int i = 0; i < code.size(); ++i) {
		int l = code[i];
		int n = (l + 7) / 8;
		string k = "";
		for (int c = 0; c < n; ++c) {
			++i;
			for (int j = 7; j >= 0 && l; --j, --l)
				k += code[i] & (1 << j) ? '1' : '0';
		}
		huffmanCode[k] = code[++i];
	}
}


vector <uchar> JajankenZipper::EncodeEncoded(vector<uchar> data) {
	uchar i = 0;
	while (i != 255) {
		vector<uchar> finali;
		map<string, uchar> huffmanMap = HuffmanEncoding(data, finali);
		finali = WriteCodeAndData(&huffmanMap, finali);
		if (data.size() > finali.size())data = finali, ++i;
		else break;
	}
	data.push_back(i);
	return data;
}

void JajankenZipper::DecodeEncoded(vector<uchar> &encoded) {
	uchar i = encoded[encoded.size() - 1];
	encoded.pop_back();
	while (i--) {
		map<string, uchar> huffmanMap;
		vector<uchar> data;

		ReadCodeAndData(huffmanMap, data, encoded);
		encoded = HuffmanDecoding(data, &huffmanMap);
	}
}

template <class T>
vector<char> JajankenZipper::PrintDataToVecOfChar(const vector<T> &v, const int &height, const int &width) {
	stringstream ss, sk;
	string retS, temp;
	vector<char> ret;

	ss << height;
	ss >> retS;
	sk << width;
	sk >> temp;
	retS += ' ' + temp + ' ';

	for (T x : v) {
		stringstream ss;
		ss << x;
		ss >> temp;
		retS += temp;
		retS += ' ';
	}

	for (char x : retS)ret.push_back(x);
	return ret;
}

template <class T>
void JajankenZipper::ReadDataFromVecOfCharX(const vector<char> &data, vector<T> &v, int &height, int &width) {
	string s = "";
	for (char x : data)s += x;

	stringstream ss;
	ss << s;

	T x;
	if (ss >> height >> width);
	while (ss >> x)v.push_back(x);
}


template <class T>
vector<char> JajankenZipper::PrintDataToVecOfChar(const vector<T> &v, const int &length) {
	stringstream ss, sk;
	string retS, temp;
	vector<char> ret;

	ss << length;
	ss >> retS;
	retS += ' ';

	for (T x : v) {
		stringstream ss;
		ss << x;
		ss >> temp;
		retS += temp;
		retS += ' ';
	}

	for (char x : retS)ret.push_back(x);
	return ret;
}

template <class T>
void JajankenZipper::ReadDataFromVecOfCharX(const vector<char> &data, vector<T> &v, int &length) {
	string s = "";
	for (char x : data)s += x;

	stringstream ss;
	ss << s;

	T x;
	if (ss >> length);
	while (ss >> x)v.push_back(x);
}


void JajankenZipper::BitStringEncoding(const string &bitString, vector<uchar> &code) {
	short l = 0, r = 0;
	for (char x : bitString) {
		bool i = x == '1';
		r |= (i << (7 - l));
		if (++l == 8) { l = 0, code.push_back(r), r = 0; }
	}
	if (l > 0)
		code.push_back(r);
}

void JajankenZipper::WriteFile(const vector<uchar> &data, const string &filePath) {
	ofstream fout(filePath, ofstream::binary);
	fout.write((char*)data.data(), data.size());
	fout.close();
}

void JajankenZipper::ReadFile(vector<uchar> &data, const string &filePath) {
	ifstream fin(filePath, ifstream::binary);
	fin.seekg(0, fin.end);
	int fileSize = fin.tellg();
	fin.seekg(0, fin.beg);
	data.resize(fileSize);
	fin.read((char*)data.data(), fileSize);
	fin.close();
}