#include "header.1.0.h"

Mat *mask;
//src是原图，注意不改变src，使用时将src拷贝出来再修改
//mask是单通道矩阵，叫做蒙版/掩膜，0表示没有选择，255表示选择区域


void on_mouse(int event, int x, int y, int flags, void *param)
//鼠标事件调用函数，用于用鼠标在param指向的原图上绘制黑色色块，同时在全局的mask上绘制255的区域
{
    static bool s_bMoushowMaskButtonDown = false;//是否已按下左键，用来检测拖动
    static Point s_cvPrePoint = Point(0, 0);
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN://这是OpenCV highgui部分
            s_bMoushowMaskButtonDown = true;
            s_cvPrePoint = Point(x, y);//记录左键按下时的坐标，即直线起点
            break;
            
        case  CV_EVENT_LBUTTONUP:
            s_bMoushowMaskButtonDown = false;
            break;
            
        case CV_EVENT_MOUSEMOVE:
            if (s_bMoushowMaskButtonDown)
            {
                Point cvCurrPoint = Point(x, y);//记下鼠标移动时坐标
                //在开始按下鼠标的地方到开始移动的地方画一条直线
                //事实上由于只有移动一点点就会画直线，画出的直线是一个微分,所以相当于画曲线
                int thickness = 30;
                line(*(Mat *)param, s_cvPrePoint, cvCurrPoint, Scalar(255, 255, 255), thickness);//在原图上画白线
                line(*mask, s_cvPrePoint, cvCurrPoint, Scalar(255), thickness);//在mask上画白线
                s_cvPrePoint = cvCurrPoint;//更新下一次绘制时的起始坐标为现在的终止坐标
                imshow("draw", *(Mat *)(param));//实时更新屏幕显示
            }
            break;
    }
}
int process(Mat &src,Mat &mask)
{
    for (int i=0; i<src.rows; i++)
    {
        Mat line(src.row(i));
        Mat lineMask(mask.row(i));
        int left=0,right=0,flag=0;
        for (int j=0; j<mask.cols; j++)
        {
            if (lineMask.at<uchar>(0,j)==255&&flag==0)
            {
                flag=1;
                left=j;
            }
            if (lineMask.at<uchar>(0,j)==0&&flag==1)
            {
                right=j;
                break;
            }
        }
        int width=right-left;
        if (width==0)
        {
            continue;
        }
        if (width>left)
        {
            cout<<"Warning Ovelflow"<<endl;
            //return 1;
        }
        /*
         for (int j=0; j<min(left,width) ;j++)
         {
         line.at<Vec3b>(0, j+left)=line.at<Vec3b>(0,left-j-1);
         }
         if (width>left)
         {
         for (int j=0; j<=min(right,width-left); j++)
         {
         line.at<Vec3b>(0, right-j)=line.at<Vec3b>(0, right+j+1);
         }
         }
        */
        for (int j=1; j<=min(right, width); j++)
        {
            line.at<Vec3b>(0, right-j)=line.at<Vec3b>(0, right+j);
        }
    }
    return 1;
}
set<patch> patches,good;
map<patch, patch> indexMap;
void exertPatch(Point anchor, Mat &ROI, Mat &dst, int pos);
void exertPatchR(Point anchor, Mat &ROI, Mat &dst, int pos);
int main()
{
    namedWindow("draw");//原图绘制窗口
    Mat src, copy, dst;//dst为库函数输出
    src = imread("testmiga.png");//读入目标图像
    
    copy = src.clone();//拷贝，以后仅对copy更改，不再更改src
    mask = new Mat(src.rows, src.cols, CV_8UC1, Scalar(0));//建立一个原图大小的单通道图像矩阵
    
    imshow("draw", src);
    setMouseCallback("draw", on_mouse, &src);//鼠标事件监测(绘制mask)
    waitKey(0);
    
    namedWindow("mask");
    imshow("mask", *mask);
    waitKey(0);
    myRect edge = getRect(*mask); //将mask包围的最小矩形
    //cout << edge;
    
    
    namedWindow("naive");//库函数跑出来的结果
    //inpaint(copy, *mask, dst, 30, INPAINT_TELEA);//调用库函数，后面两个参数是取样半径和处理方法（方法有两个，分别参照两篇paper）
    process(src,*mask);
    dst=src.clone();
    imshow("naive", dst);
    waitKey(0);
    
    namedWindow("output");
    myRect full(0,dst.rows,0,dst.cols);//myRect(int top, int bottom, int left, int right);此处表示全图
    
    shrink(*mask, full, patches, dst, good);//根据mask在full表示的范围内，把dst中mask覆盖的区域切成小块扔到patches里面,其他区域扔到good里面
    
    
    getSimilar(patches,good,indexMap);
    //对于patches中的每个元素,在good中找到与之最相似的,并将结果存入indexMap中
    
    /*
    set<patch>::iterator itg = good.begin(),itd = patches.begin();
    int cnt = 0;
    for (itd = patches.begin(); itd != patches.end(); itd++)//对于每一个需要打补丁的地方
    {
        double min = 1000000; //INF
        for (itg = good.begin(); itg != good.end(); itg++)//在good里找到和它最相似的的，放到一个map indexMap里
        {
            double crt = sim(*itd, *itg);
            crt=getSimSturcture(*itd, *itg);
            if (crt < min)
            {
                min = crt;
                indexMap[*itd] = *itg;//itd指向dst上需要打补丁的地方 itg指向选出来的补丁
            }
        }
        cout << cnt++ << endl;
    }
     */
    
    
    //cout << patches.size() << endl;
    for (map<patch,patch>::iterator i = indexMap.begin(); i != indexMap.end(); i++)
    {
        //dst为库函数输出的结果
        Mat roi(dst, Rect(i->first.anchor.x, i->first.anchor.y, PATCH_SIZE, PATCH_SIZE));//按照补丁的anchor(左上角点)把这一块设置从roi
        
        //namedWindow("test");
        //namedWindow("test2");
        //imshow("test", i->second.data);
        //cout << i->second.data << endl;
        //waitKey(0);
        
        exertPatch(i->first.anchor, i->second.data ,dst, 0);
        //void exertPatch(Point anchor, Mat &ROI, Mat &dst, int pos)
        //对于patches中需要更改的anchor处的patch，用ROI来对其进行覆盖
        
        //cout << i->second.data << endl;
        exertPatchR(i->first.anchor, i->second.data, dst, 1);
        
        i->second.data.copyTo(roi);//打补丁到roi
        
        //imshow("test2", i->second.data);
        //waitKey(0);
    }
    
    Mat dstROI(dst, Rect(edge.left, edge.top, edge.right - edge.left, edge.bottom - edge.top));
    
    Mat ddst = dstROI.clone();
    //blur(dstROI, ddst, Size(5, 5));
    //ddst.copyTo(dstROI);
    imshow("output", dst);
    waitKey(0);
    delete mask;
    
    return 0;
}

void rot90(cv::Mat &matImage, int rotflag){
    //1=CW, 2=CCW, 3=180
    if (rotflag == 1){
        transpose(matImage, matImage);
        flip(matImage, matImage, 1); //transpose+flip(1)=CW
    }
    else if (rotflag == 2) {
        transpose(matImage, matImage);
        flip(matImage, matImage, 0); //transpose+flip(0)=CCW
    }
    else if (rotflag == 3){
        flip(matImage, matImage, -1);    //flip(-1)=180
    }
    else if (rotflag != 0){ //if not 0,1,2,3:
        cout << "Unknown rotation flag(" << rotflag << ")" << endl;
    }
}

//将dp结果输出到res
void dp(Mat & seamMatrix,Mat &oldPatch,Mat &newPatch,Mat &res)
{
    int dist[PATCH_SIZE][PATCH_SIZE] = { 0 };
    int route[PATCH_SIZE][PATCH_SIZE] = { 0 };
    int line[PATCH_SIZE][PATCH_SIZE] = { 0 };
    int edge = HALFPATCHSIZE;
    
    for (int i = 1; i < HALFPATCHSIZE; i++)
    {
        dist[0][i] = dist[0][i - 1] + seamMatrix.at<int>(0, i);
        route[0][i] = 1;//左
        dist[i][0] = dist[i - 1][0] + seamMatrix.at<int>(i, 0);
        route[i][0] = 2;//上
    }
    for (int i = 1; i < HALFPATCHSIZE; i++)
    {
        for (int j = 1; j < HALFPATCHSIZE; j++)
        {
            if (dist[i][j - 1] > dist[i - 1][j])
            {
                dist[i][j] = dist[i - 1][j] + seamMatrix.at<int>(i, j);
                route[i][j] = 2;
            }
            else
            {
                dist[i][j] = dist[i][j - 1] + seamMatrix.at<int>(i, j);
                route[i][j] = 1;
            }
        }
    }
    int x = HALFPATCHSIZE - 1;
    int y = x;
    line[x][y] = 1;
    while (true)
    {
        switch (route[x][y])
        {
            case 1:
                y--;
                break;
            case 2:
                x--;
            default:
                break;
        }
        line[x][y] = 1;
        if (x == 0 && y == 0)
        {
            break;
        }
    }
    Mat patch = oldPatch.clone();
    //cout << patch << endl;
    for (int i = 0; i < edge; i++)
    {
        for (int j = 0; j < edge; j++)
        {
            if (line[i][j])
            {
                break;
            }
            //cout << i << " " << j << endl;
            patch.at<Vec3b>(i, j) = newPatch.at<Vec3b>(i, j);
        }
    }
    //cout << patch << endl;
    res = patch.clone();
    //cout << dst << endl;
}

//对于patches中需要更改的anchor处的patch，用ROI来对其进行覆盖
void exertPatch(Point anchor, Mat &ROI,Mat &dst,int pos)//我们认为左上角和右上角已经有数据了
{
    //pos没有用！！！
    const int edge = HALFPATCHSIZE;
    Mat newLeftTopROI(ROI, Rect(0,0, HALFPATCHSIZE, HALFPATCHSIZE));
    Mat oldLeftTopROI(dst, Rect(anchor.x, anchor.y, HALFPATCHSIZE, HALFPATCHSIZE));
    
    Mat seamMatrix(HALFPATCHSIZE, HALFPATCHSIZE, CV_32FC1, Scalar(0));
    
    for (int k = 0; k < PATCH_SIZE/2; k++)
    {
        for (int l = 0; l < PATCH_SIZE/2; l++)
        {
            //需要修改
            int diff = 0;
            if(k>0&&k<HALFPATCHSIZE-1&&l>0&&l<HALFPATCHSIZE-1)
            {
                for (int m = -1; m < 2; m++)
                {
                    for (int j=-1; j<2; j++)
                    {
                        for (int i=0; i<3; i++)
                        {
                            diff += abs(newLeftTopROI.at<Vec3b>(k+j, l+m)[i] - oldLeftTopROI.at<Vec3b>(k+j, l+m)[i]);
                        }
                    }
                }
                diff/=9;
            }
            else
            {
                for (int i = 0; i < 3; i++)
                {
                    diff += abs(newLeftTopROI.at<Vec3b>(k, l)[i] - oldLeftTopROI.at<Vec3b>(k, l)[i]);
                }
                diff /= 3;
            }
            //diff += 128;
            seamMatrix.at<int>(k, l) = diff;
        }
    }
    
    //将其顺时针旋转90度
    int rotIndex[4][2] = { { 1, 2 }, { 0, 0 }, { 2, 1 }, { 3, 3 } };
    rot90(seamMatrix, rotIndex[pos][0]);
    rot90(newLeftTopROI, rotIndex[pos][0]);
    rot90(oldLeftTopROI, rotIndex[pos][0]);
    Mat newdst = oldLeftTopROI.clone();
    
    //DP
    dp(seamMatrix, oldLeftTopROI, newLeftTopROI, newdst);
    
    //将其转回
    rot90(newLeftTopROI, rotIndex[pos][1]);
    rot90(oldLeftTopROI, rotIndex[pos][1]);
    rot90(newdst, rotIndex[pos][1]);
    newdst.copyTo(newLeftTopROI);
}

void exertPatchR(Point anchor, Mat &ROI, Mat &dst, int pos)//我们认为左上角和右上角已经有数据了
{
    int edge = HALFPATCHSIZE;
    Mat newRightTopROI(ROI, Rect(edge-1, 0, HALFPATCHSIZE, HALFPATCHSIZE));
    Mat oldRightTopROI(dst, Rect(anchor.x+edge-1, anchor.y, HALFPATCHSIZE, HALFPATCHSIZE));
    Mat seamMatrix(HALFPATCHSIZE, HALFPATCHSIZE, CV_8UC1, Scalar(0));
    int k, l;
    for (k = 0; k < HALFPATCHSIZE; k++)
    {
        for (l = 0; l < HALFPATCHSIZE; l++)
        {
            int diff = 0;
            for (int i = 0; i < 3; i++)
            {
                diff += newRightTopROI.at<Vec3b>(k, l)[i] - oldRightTopROI.at<Vec3b>(k, l)[i];
            }
            diff /= 3;
            seamMatrix.at<int>(k, l) = diff;
        }
    }
    //DP
    int rotIndex[4][2] = { { 1, 2 }, { 0, 0 }, { 2, 1 }, { 3, 3 } };
    rot90(seamMatrix, rotIndex[pos][0]);
    rot90(newRightTopROI, rotIndex[pos][0]);
    rot90(oldRightTopROI, rotIndex[pos][0]);
    Mat newdst = oldRightTopROI.clone();
    dp(seamMatrix, oldRightTopROI, newRightTopROI, newdst);
    rot90(newRightTopROI, rotIndex[pos][1]);
    rot90(oldRightTopROI, rotIndex[pos][1]);
    rot90(newdst, rotIndex[pos][1]);
    //cout << newdst << endl;;
    //cout << newLeftTopROI << endl;
    newdst.copyTo(newRightTopROI);
}