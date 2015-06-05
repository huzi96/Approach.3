#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <set>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <opencv2/gpu/gpu.hpp>
#define PATCH_SIZE 10
#define HALFPATCHSIZE 5
//#define DEBUG
using namespace std;
using namespace cv;

class patch{
public:
    Mat &data;
    Point anchor;//左上角点
    patch();
    ~patch();
    void del();//由于data是动态分配的，需要delete，但是由于Mat的复制是浅复制，所以不能在析构的时候delete，所以单独分出来一个函数
    patch & operator=(const patch & op);
};
bool operator<(const patch &op1, const patch &op2);
bool operator ==(const patch &op1, const patch & op2);

class myRect
{
public:
    int top, right, bottom, left;
    myRect(int top, int bottom, int left, int right);
    myRect(){};
};
ostream & operator<<(ostream &, myRect &);
myRect getRect(Mat &);

void shrink(Mat &mask, myRect rect, set<patch>& patches, Mat &ori,set<patch>& good);
double sim(const patch &, const patch &);
void getSimilar(set<patch> & patches, set<patch> & good, map<patch, patch> & indexMap);
double getSimSturcture(const patch & op1,const patch &op2);