#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
using namespace cv;
using namespace std;

double QUANT_MATRIX[][8] = {{16, 11, 10, 16, 24, 40, 51, 61},
						{12, 12, 14, 19, 26, 58, 60, 55},
						{14, 13, 16, 24, 40, 57, 69, 56},
						{14, 17, 22, 29, 51, 87, 80, 62},
						{18, 22, 37, 56, 68, 109, 103, 77},
						{24, 35, 55, 64, 81, 104, 113, 92},
						{49, 64, 78, 87, 103, 121, 120, 101},
						{72, 92, 95, 98, 112, 100, 103, 99}};
double PI = 3.14159265359;
double cs[10][10];
vector<pair<int, int> > path;
bool check(int i, int j){
	return (i >= 0 && i < 8 && j >= 0 && j < 8);
}
void init(){
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 8; j++){
			cs[i][j] = cos((2.0*i+1.0)*j*PI/16.0);
		}
	}
	bool used[9][9];
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 8; j++){
			used[i][j] = false;
		}
	}
	int curri = 0;
	int currj = 0;
	for (int i = 0; i < 64; i++){
		path.push_back(make_pair(curri, currj));
		used[curri][currj] = true;
		if (check(curri+1, currj-1) && check(curri+1, currj-1) && !used[curri+1][currj-1]){
			curri++;
			currj--;
		}
		else if (check(curri-1, currj+1) && check(curri-1, currj+1) && !used[curri-1][currj+1]){
			curri--;
			currj++;
		}
		else if (curri == 7 || curri == 0){
			currj++;
		}
		else if (currj == 7 || currj == 0){
			curri++;
		}
	}
}
void write(vector<char> &v, short x){
	v.push_back((char)(x/256));
	v.push_back((char)(x%256));
}
void write(vector<char> &v, vector<vector<char>> &x){
	for (int i = 0; i < x.size(); i++){
		for (int j = 0; j < x[i].size(); j++){
			v.push_back(x[i][j]);
		}
	}
}
void read(vector<char> &in, int &offset, short &val){
	val = (short)(((uchar)in[offset]))*256 + (short)(((uchar)in[offset+1]));
	offset += 2;
}
void read(vector<char> &in, int &offset, vector<vector<char> > &mat){
	int curr = 0;
	for (int i = 0; i < mat.size(); i++){
		for (int j = 0; j < mat[i].size(); j++){
			mat[i][j] = in[offset + curr];
			curr++;
		}
	}
	offset += curr;
}
void write_to_file(vector<char> &v, string filename){
	ofstream output(filename, ofstream::binary);
	copy(v.begin(), v.end(), ostream_iterator<char>(output));
}
void read_from_file(vector<char> &v, string filename){
	ifstream input(filename, ios::binary);
	v.assign((istreambuf_iterator<char>(input)),(istreambuf_iterator<char>()));
}
void resolution_reduction(Mat &image, 	vector<vector<char> > &y,
										vector<vector<char> > &u,
										vector<vector<char> > &v){
	short w = image.cols, h = image.rows;
	cvtColor(image, image, CV_BGR2YCrCb);	
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){
			Vec3b temp = image.at<Vec3b>(i, j);
			y[i][j] = (char)temp[0];
			if (i%2 == 0 && j%2 == 0){
				u[i/2][j/2] = (char)temp[1];
				v[i/2][j/2] = (char)temp[2];
			}
		}
	}
}
void inv_resolution_reduction(Mat &res, vector<vector<char> > &y, vector<vector<char> > &u, vector<vector<char> > &v){
	int h = y.size();
	int w = y[0].size();
	for (int i = 0; i < h; i++){
		for (int j = 0; j < w; j++){
			res.at<Vec3b>(i, j)[0] = (uchar)y[i][j];
			res.at<Vec3b>(i, j)[1] = (uchar)u[i/2][j/2];
			res.at<Vec3b>(i, j)[2] = (uchar)v[i/2][j/2];
		}
	}
	cvtColor(res, res, CV_YCrCb2BGR);
}
double c(int i){
	if (i == 0) return 1.0/sqrt(2.0);
	return 1.0;
}
void dct(vector<vector<char> > &in, vector<vector<char> > &out){
	int h = in.size();
	int w = in[0].size();
	double dr[8][8];
	for (int i = 0; i+7 < h; i += 8){
		for (int j = 0; j+7 < w; j += 8){
			for (int ii = i; ii < i+8; ii++){
				for (int jj = j; jj < j+8; jj++){
					dr[ii-i][jj-j] = 0;
					for (int jk = j; jk < j+8; jk++){
						dr[ii-i][jj-j] += 0.5*c(jj-j)*((double)((uchar)in[ii][jk]))*cs[jk-j][jj-j];
					}
				}
			}
			for (int ii = i; ii < i+8; ii++){
				for (int jj = j; jj < j+8; jj++){
					double val = 0;
					for (int ik = i; ik < i+8; ik++){
						val += 0.5*c(ii-i)*cs[ik-i][ii-i]*dr[ik-i][jj-j];
					}
					int fval = (int)(val/QUANT_MATRIX[ii-i][jj-j]);
					if (fval > 127) fval = 127;
					if (fval < -128) fval = -128;
					out[ii][jj] = (char)(fval);	
				}
			}			
		}
	}	
	clock_t end = clock();
	in = out;
}
void inv_dct(vector<vector<char> > &in, vector<vector<char> > &out){
	int h = in.size();
	int w = out[0].size();
	double dr[8][8];
	for (int i = 0; i+7 < h; i += 8){
		for (int j = 0; j+7 < w; j += 8){
			for (int ii = i; ii < i+8; ii++){
				for (int jj = j; jj < j+8; jj++){
					dr[ii-i][jj-j] = 0;
					for (int jk = j; jk < j+8; jk++){
						dr[ii-i][jj-j] += 0.5*c(jk-j)*((double)(in[ii][jk])*QUANT_MATRIX[ii-i][jk-j])*cs[jj-j][jk-j];
					}
				}
			}
			for (int ii = i; ii < i+8; ii++){
				for (int jj = j; jj < j+8; jj++){
					double val = 0;
					for (int ik = i; ik < i+8; ik++){
						val += 0.5*c(ik-i)*cs[ii-i][ik-i]*dr[ik-i][jj-j];
					}
					int fval = ((int)(val));
					if (fval > 255) fval = 255;
					if (fval < 0) fval = 0;
					out[ii][jj] = (char)fval;
				}
			}			
		}
	}	
	in = out;
}
void transform(vector<vector<char> > &y, vector<vector<char> > &u, vector<vector<char> > &v){
	int h = y.size();
	int w = y[0].size();
	vector<vector<char> > dy(h, vector<char>(w,1)), du(h/2, vector<char>(w/2,1)), dv(h/2, vector<char>(w/2,1));
	dct(y, dy);
	dct(u, du);
	dct(v, dv);
}
void inv_transform(vector<vector<char> > &y, vector<vector<char> > &u, vector<vector<char> > &v){
	int h = y.size();
	int w = y[0].size();
	vector<vector<char> > dy(h, vector<char>(w,1)), du(h/2, vector<char>(w/2,1)), dv(h/2, vector<char>(w/2,1));
	inv_dct(y, dy);
	inv_dct(u, du);
	inv_dct(v, dv);
}
void zigzag(vector<vector<char> > &in, vector<char> &res){
	int h = in.size();
	int w = in[0].size();
	int curr = 0;
	for (int i = 0; i+8 <= h; i += 8){
		for (int j = 0; j+8 <= w; j += 8){
			for (int k = 0; k < path.size(); k++){
				int ito = path[k].first;
				int jto = path[k].second;
				res.push_back(in[i+ito][j+jto]);
				curr++;
			}
		}
	}
}
void inv_zigzag(vector<char> &in, int &offset, vector<vector<char> > &out){
	int h = out.size();
	int w = out[0].size();
	int curr = offset;
	for (int i = 0; i+8 <= h; i += 8){
		for (int j = 0; j+8 <= w; j += 8){
			for (int k = 0; k < path.size(); k++){
				int ito = path[k].first;
				int jto = path[k].second;
				out[i+ito][j+jto] = in[curr];
				curr++;
			}
		}
	}	
	offset = curr;
}
void rle(vector<char> &seq, vector<char> &res){
	int zeros = 0;
	for (int i = 0; i < seq.size(); i++){
		if (seq[i] == 0) zeros ++;
		else {
			write(res, (short)zeros);
			res.push_back(seq[i]);
			zeros = 0;
		}
	}
}
void inv_rle(vector<char> &seq, vector<char> &res, int &offset, int count){
	for (int i = offset; i < seq.size(); i += 3){
		int zeros = ((uchar)seq[i])*256+(uchar)seq[i+1];
		char val = seq[i+2];
		for (int j = 0; j < zeros; j++){
			res.push_back((char)0);
		}
		res.push_back(val);
	}
	while (res.size() < count){
		res.push_back((char)0);
	}
}
void compress(Mat &frame, string filename){
	int w = frame.cols;
	int h = frame.rows;
	vector<vector<char> > y(h, vector<char>(w,1)), u(h/2, vector<char>(w/2,1)), v(h/2, vector<char>(w/2,1));
	resolution_reduction(frame, y, u, v);
	transform(y, u, v);
	vector<char> seq;
	zigzag(y, seq);
	zigzag(u, seq);
	zigzag(v, seq);
	vector<char> compressed;
	write(compressed, (short)w);
	write(compressed, (short)h);
	rle(seq, compressed);
	write_to_file(compressed, filename);
}
void decompress(string filename){
	vector<char> compressed;
	read_from_file(compressed, filename);
	short w, h;
	int offset = 0;
	read(compressed, offset, w);
	read(compressed, offset, h);
	int count = (w/8)*(h/8)*64 + 2*(w/16)*(h/16)*64;
	vector<char> seq;
	inv_rle(compressed, seq, offset, count);	
	offset = 0;
	vector<vector<char> > y(h, vector<char>(w,1)), u(h/2, vector<char>(w/2,1)), v(h/2, vector<char>(w/2,1));
	inv_zigzag(seq, offset, y);
	inv_zigzag(seq, offset, u);
	inv_zigzag(seq, offset, v);
	//inv_transform(y, u, v);
	Mat frame(h, w, CV_8UC3, Scalar(255));
	inv_resolution_reduction(frame, y, u, v);		
	imwrite("dct.bmp", frame);
}
int main(int argc, char* argv[]) {
	init();
	Mat image = imread("original.bmp");
	//imwrite("orig.jpg", image);
	compress(image, "compressed.pku");
	decompress("compressed.pku");
	
	/*VideoCapture cap(0);
	if (!cap.isOpened()) {
		throw "Error when reading steam_avi";
	}
	namedWindow("original", 1);
	Mat frame;
	int frame_n = 0;
	for (; ; ){
		cap >> frame;
		if (frame.empty())
			break;
		frame_n++;
		cout << frame_n << endl;
		imshow("original", frame);
		imwrite("original/frame" + to_string(frame_n) + ".bmp", frame);
		compress(frame, "frames/frame" + to_string(frame_n) + ".pku");
		//imshow("w", frame);
		waitKey(1);
	}*/
	return 0;
}



