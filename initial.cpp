#include "header.1.0.h"
//Created by huzi
//定义了基础数据类型

patch::patch() :data(*(new Mat(PATCH_SIZE, PATCH_SIZE, CV_8UC3))) {};
patch::~patch(){}
void patch::del()
{
    delete &data;
}
patch & patch:: operator=(const patch & op)
{
    data = op.data;
    anchor = op.anchor;
    return *this;
}
bool operator<(const patch & op1,const patch & op2) //先按x从小到大，再按y从小到大
{
    if (op1.anchor.y == op2.anchor.y)
        return op1.anchor.x < op2.anchor.x;
    else
        return op1.anchor.y < op2.anchor.y;
}
bool operator==(const patch & op1, const patch &op2)
{
    if (op1.anchor.x == op2.anchor.x && op1.anchor.y == op2.anchor.y)
    {
        return 1;
    }
    return 0;
}

ostream & operator<<(ostream & out, myRect & op)
{
    out << op.top << ' ' << op.right << ' ' << op.bottom << ' ' << op.left << endl;
    return out;
}
myRect getRect(Mat &mask)
{
    myRect res;
    //get top
    int top, bot, left, right;
    for (top = 0; top < mask.rows; top++)
    {
        Mat row = mask.row(top);
        bool marked = 0;
        for (MatIterator_<uchar> i = row.begin<uchar>(); i != row.end<uchar>(); i++)
        {
            if (*i>0)
            {
                marked = 1;
                break;
            }
        }
        if (marked)
        {
            break;
        }
    }
    res.top = top;
    for (bot = mask.rows - 1; bot >= 0; bot--)
    {
        Mat row = mask.row(bot);
        bool marked = 0;
        for (MatIterator_<uchar> i = row.begin<uchar>(); i != row.end<uchar>(); i++)
        {
            if (*i>0)
            {
                marked = 1;
                break;
            }
        }
        if (marked)
        {
            break;
        }
    }
    res.bottom = bot;
    for (right = mask.cols - 1; right >= 0; right--)
    {
        Mat col = mask.col(right);
        bool marked = 0;
        for (MatIterator_<uchar> i = col.begin<uchar>(); i != col.end<uchar>(); i++)
        {
            if (*i>0)
            {
                marked = 1;
                break;
            }
        }
        if (marked)
        {
            break;
        }
    }
    res.right = right;
    for (left = 0; left < mask.cols; left++)
    {
        Mat col = mask.col(left);
        bool marked = 0;
        for (MatIterator_<uchar> i = col.begin<uchar>(); i != col.end<uchar>(); i++)
        {
            if (*i>0)
            {
                marked = 1;
                break;
            }
        }
        if (marked)
        {
            break;
        }
    }
    res.left = left;
    return res;
}
myRect::myRect(int t, int b, int l, int r) :top(t), bottom(b), left(l), right(r){};