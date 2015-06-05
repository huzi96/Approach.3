#include "header.1.0.h"
//只需要看shrink函数和sim函数，其它都是库函数

void shrink(Mat & mask, myRect rect,set<patch> &patches,Mat &srcCopy ,set<patch> &good) //按照mask将srcCopy相应部分划分为patch加入patches，其余加入good
{
#ifdef DEBUG
    fstream out("out.txt",ios::out);
#endif
    int width, height, patchColNumber, patchRowNumber;
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    width -= width % PATCH_SIZE;//舍弃后面除不尽的部分（分割后的剩余部分）
    height -= height % PATCH_SIZE;
    patchColNumber = width / PATCH_SIZE;//横向patch的数量
    patchRowNumber = height / PATCH_SIZE;//纵向
    
    Point topleft(rect.left, rect.top);//由于rect传进来的是全图，故此处为(0,0)
    
    for (int i = 0; i < patchColNumber; i++)
    {
        for (int j = 0; j < patchRowNumber; j++)//对于每一个格子
        {
            Point anchor(topleft.x + i*PATCH_SIZE, topleft.y + j*PATCH_SIZE);//找到左上角角点
            
            Mat roiMask(mask, Rect(anchor.x, anchor.y, PATCH_SIZE, PATCH_SIZE)); //mask上取patch
            
            Mat roiOri(srcCopy, Rect(anchor.x, anchor.y, PATCH_SIZE, PATCH_SIZE)); //原图上取patch
            
            patch thispatch;
            thispatch.data = roiOri.clone(); //深复制
            thispatch.anchor = anchor;
            
            bool valid = false;//所选小块内是否有任何区域被mask覆盖
            
            MatIterator_<uchar> it = roiMask.begin<uchar>();//遍历整个roiMask，看看有没有不为0的像素
            for  (;it!=roiMask.end<uchar>(); it++)
            {
                if (*it != 0)
                {
                    valid = 1;
                    break;
                }
            }
            
            if (valid)//如果含有mask的部分，加入patches
                patches.insert(thispatch);
            else //否则加入good
                good.insert(thispatch);
            //cout << patches.size() << " " << good.size() << endl << endl;
            
            //加入重叠方格部分
            if (i != patchColNumber - 1 && j != patchRowNumber - 1) //方格中心点
            {
                Point halfAnchor(topleft.x + PATCH_SIZE / 2 + i*PATCH_SIZE, topleft.y + PATCH_SIZE / 2 + j*PATCH_SIZE);
                Mat roiMaskH(mask, Rect(halfAnchor.x, halfAnchor.y, PATCH_SIZE, PATCH_SIZE)); //mask上取patch
                
                Mat roiOriH(srcCopy, Rect(halfAnchor.x, halfAnchor.y, PATCH_SIZE, PATCH_SIZE)); //原图上取patch
                
                patch thispatchH;
                thispatchH.data = roiOriH.clone(); //深复制
                thispatchH.anchor = halfAnchor;
                
                bool validH = false;//所选小块内是否有任何区域被mask覆盖
                
                MatIterator_<uchar> it = roiMaskH.begin<uchar>();//遍历整个roiMask，看看有没有不为0的像素
                for (; it != roiMaskH.end<uchar>(); it++)
                {
                    if (*it != 0)
                    {
                        validH = 1;
                        break;
                    }
                }
                
                if (validH)//如果含有mask的部分，加入patches
                    patches.insert(thispatchH);
                else //否则加入good
                    good.insert(thispatchH);
                //cout <<"Small "<< patches.size() << " " << good.size() << endl;
                //cout << thispatchH.anchor << " " << thispatch.anchor << endl;
            }
        }
    }
}

double sim(const patch & op1, const  patch &op2)
{
    Mat h1, h2;
    Scalar mean1(0, 0, 0), mean2(0, 0, 0);//L*a*b三通道的均值
    cvtColor(op1.data, h1, CV_BGR2Lab);
    cvtColor(op2.data, h2, CV_BGR2Lab);//h1,h2里面是op1,op2转化到lab色彩空间的图
    //通道转换大量重复
    
    cv::MatIterator_<Vec3b> i, j;
    for (i = h1.begin<Vec3b>(), j = h2.begin<Vec3b>(); i != h1.end<Vec3b>(); i++, j++)//遍历和
    {
        for (int k = 0; k < 3; k++)
        {
            mean1[k] += (*i)[k];
            mean2[k] += (*j)[k];
        }
    }
    //mean1 /= PATCH_SIZE;//其实没有除以像素点数，而是除了一个宽度。。。
    //mean2 /= PATCH_SIZE;
    
    double m = 0;
    for (int i = 0; i < 3; i++)//求两个均值向量欧拉距离
    {
        m += (mean1[i] - mean2[i])*(mean1[i] - mean2[i]);
    }
    cout<<"trad "<<sqrt(m)<<endl;
    return sqrt(m);
}
Scalar getMSSIM(const Mat& i1, const Mat& i2);
double getSimSturcture(const patch & op1,const patch &op2)
{
    Scalar res=getMSSIM(op1.data, op2.data);
    //cout<<"res "<<res<<endl;
    double ans=res[0];//+res[1]+res[2];
    cout<<1/ans<<endl;
    return 1/ans;
}

//以下均为库函数
Scalar getMSSIM(const Mat& i1, const Mat& i2)
{
    const double C1 = 6.5025, C2 = 58.5225;
    /***************************** INITS **********************************/
    int d = CV_32F;
    
    Mat I1, I2;
    i1.convertTo(I1, d);           // cannot calculate on one byte large values
    i2.convertTo(I2, d);
    
    Mat I2_2 = I2.mul(I2);        // I2^2
    Mat I1_2 = I1.mul(I1);        // I1^2
    Mat I1_I2 = I1.mul(I2);        // I1 * I2
    
    /*************************** END INITS **********************************/
    
    Mat mu1, mu2;   // PRELIMINARY COMPUTING
    GaussianBlur(I1, mu1, Size(11, 11), 1.5);
    GaussianBlur(I2, mu2, Size(11, 11), 1.5);
    
    Mat mu1_2 = mu1.mul(mu1);
    Mat mu2_2 = mu2.mul(mu2);
    Mat mu1_mu2 = mu1.mul(mu2);
    
    Mat sigma1_2, sigma2_2, sigma12;
    
    GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;
    
    GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;
    
    GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;
    
    ///////////////////////////////// FORMULA ////////////////////////////////
    Mat t1, t2, t3;
    
    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);              // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    
    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);               // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
    
    Mat ssim_map;
    divide(t3, t1, ssim_map);      // ssim_map =  t3./t1;
    
    Scalar mssim = mean(ssim_map); // mssim = average of ssim map
    return mssim;
}
Scalar getMSSIM_GPU(const Mat& i1, const Mat& i2)
{
    const float C1 = 6.5025f, C2 = 58.5225f;
    /***************************** INITS **********************************/
    gpu::GpuMat gI1, gI2, gs1, tmp1, tmp2;
    
    gI1.upload(i1);
    gI2.upload(i2);
    
    gI1.convertTo(tmp1, CV_MAKE_TYPE(CV_32F, gI1.channels()));
    gI2.convertTo(tmp2, CV_MAKE_TYPE(CV_32F, gI2.channels()));
    
    vector<gpu::GpuMat> vI1, vI2;
    gpu::split(tmp1, vI1);
    gpu::split(tmp2, vI2);
    Scalar mssim;
    
    for (int i = 0; i < gI1.channels(); ++i)
    {
        gpu::GpuMat I2_2, I1_2, I1_I2;
        
        gpu::multiply(vI2[i], vI2[i], I2_2);        // I2^2
        gpu::multiply(vI1[i], vI1[i], I1_2);        // I1^2
        gpu::multiply(vI1[i], vI2[i], I1_I2);       // I1 * I2
        
        /*************************** END INITS **********************************/
        gpu::GpuMat mu1, mu2;   // PRELIMINARY COMPUTING
        gpu::GaussianBlur(vI1[i], mu1, Size(11, 11), 1.5);
        gpu::GaussianBlur(vI2[i], mu2, Size(11, 11), 1.5);
        
        gpu::GpuMat mu1_2, mu2_2, mu1_mu2;
        gpu::multiply(mu1, mu1, mu1_2);
        gpu::multiply(mu2, mu2, mu2_2);
        gpu::multiply(mu1, mu2, mu1_mu2);
        
        gpu::GpuMat sigma1_2, sigma2_2, sigma12;
        
        gpu::GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
        gpu::subtract(sigma1_2, mu1_2, sigma1_2); // sigma1_2 -= mu1_2;
        
        gpu::GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
        gpu::subtract(sigma2_2, mu2_2, sigma2_2); // sigma2_2 -= mu2_2;
        
        gpu::GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
        gpu::subtract(sigma12, mu1_mu2, sigma12); // sigma12 -= mu1_mu2;
        
        ///////////////////////////////// FORMULA ////////////////////////////////
        gpu::GpuMat t1, t2, t3;
        
        mu1_mu2.convertTo(t1, -1, 2, C1); // t1 = 2 * mu1_mu2 + C1;
        sigma12.convertTo(t2, -1, 2, C2); // t2 = 2 * sigma12 + C2;
        gpu::multiply(t1, t2, t3);        // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
        
        gpu::addWeighted(mu1_2, 1.0, mu2_2, 1.0, C1, t1);       // t1 = mu1_2 + mu2_2 + C1;
        gpu::addWeighted(sigma1_2, 1.0, sigma2_2, 1.0, C2, t2); // t2 = sigma1_2 + sigma2_2 + C2;
        gpu::multiply(t1, t2, t1);                              // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
        
        gpu::GpuMat ssim_map;
        gpu::divide(t3, t1, ssim_map);      // ssim_map =  t3./t1;
        
        Scalar s = gpu::sum(ssim_map);
        mssim.val[i] = s.val[0] / (ssim_map.rows * ssim_map.cols);
        
    }
    return mssim;
}
struct BufferMSSIM                                     // Optimized GPU versions
{   // Data allocations are very expensive on GPU. Use a buffer to solve: allocate once reuse later.
    gpu::GpuMat gI1, gI2, gs, t1, t2;
    
    gpu::GpuMat I1_2, I2_2, I1_I2;
    vector<gpu::GpuMat> vI1, vI2;
    
    gpu::GpuMat mu1, mu2;
    gpu::GpuMat mu1_2, mu2_2, mu1_mu2;
    
    gpu::GpuMat sigma1_2, sigma2_2, sigma12;
    gpu::GpuMat t3;
    
    gpu::GpuMat ssim_map;
    
    gpu::GpuMat buf;
};
Scalar getMSSIM_GPU_optimized(const Mat& i1, const Mat& i2, BufferMSSIM& b)
{
    const float C1 = 6.5025f, C2 = 58.5225f;
    /***************************** INITS **********************************/
    
    b.gI1.upload(i1);
    b.gI2.upload(i2);
    
    gpu::Stream stream;
    
    stream.enqueueConvert(b.gI1, b.t1, CV_32F);
    stream.enqueueConvert(b.gI2, b.t2, CV_32F);
    
    gpu::split(b.t1, b.vI1, stream);
    gpu::split(b.t2, b.vI2, stream);
    Scalar mssim;
    
    gpu::GpuMat buf;
    
    for (int i = 0; i < b.gI1.channels(); ++i)
    {
        gpu::multiply(b.vI2[i], b.vI2[i], b.I2_2, stream);        // I2^2
        gpu::multiply(b.vI1[i], b.vI1[i], b.I1_2, stream);        // I1^2
        gpu::multiply(b.vI1[i], b.vI2[i], b.I1_I2, stream);       // I1 * I2
        
        gpu::GaussianBlur(b.vI1[i], b.mu1, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
        gpu::GaussianBlur(b.vI2[i], b.mu2, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
        
        gpu::multiply(b.mu1, b.mu1, b.mu1_2, stream);
        gpu::multiply(b.mu2, b.mu2, b.mu2_2, stream);
        gpu::multiply(b.mu1, b.mu2, b.mu1_mu2, stream);
        
        gpu::GaussianBlur(b.I1_2, b.sigma1_2, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
        gpu::subtract(b.sigma1_2, b.mu1_2, b.sigma1_2, gpu::GpuMat(), -1, stream);
        //b.sigma1_2 -= b.mu1_2;  - This would result in an extra data transfer operation
        
        gpu::GaussianBlur(b.I2_2, b.sigma2_2, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
        gpu::subtract(b.sigma2_2, b.mu2_2, b.sigma2_2, gpu::GpuMat(), -1, stream);
        //b.sigma2_2 -= b.mu2_2;
        
        gpu::GaussianBlur(b.I1_I2, b.sigma12, Size(11, 11), buf, 1.5, 0, BORDER_DEFAULT, -1, stream);
        gpu::subtract(b.sigma12, b.mu1_mu2, b.sigma12, gpu::GpuMat(), -1, stream);
        //b.sigma12 -= b.mu1_mu2;
        
        //here too it would be an extra data transfer due to call of operator*(Scalar, Mat)
        gpu::multiply(b.mu1_mu2, 2, b.t1, 1, -1, stream); //b.t1 = 2 * b.mu1_mu2 + C1;
        gpu::add(b.t1, C1, b.t1, gpu::GpuMat(), -1, stream);
        gpu::multiply(b.sigma12, 2, b.t2, 1, -1, stream); //b.t2 = 2 * b.sigma12 + C2;
        gpu::add(b.t2, C2, b.t2, gpu::GpuMat(), -12, stream);
        
        gpu::multiply(b.t1, b.t2, b.t3, 1, -1, stream);     // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
        
        gpu::add(b.mu1_2, b.mu2_2, b.t1, gpu::GpuMat(), -1, stream);
        gpu::add(b.t1, C1, b.t1, gpu::GpuMat(), -1, stream);
        
        gpu::add(b.sigma1_2, b.sigma2_2, b.t2, gpu::GpuMat(), -1, stream);
        gpu::add(b.t2, C2, b.t2, gpu::GpuMat(), -1, stream);
        
        
        gpu::multiply(b.t1, b.t2, b.t1, 1, -1, stream);     // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
        gpu::divide(b.t3, b.t1, b.ssim_map, 1, -1, stream);      // ssim_map =  t3./t1;
        
        stream.waitForCompletion();
        
        Scalar s = gpu::sum(b.ssim_map, b.buf);
        mssim.val[i] = s.val[0] / (b.ssim_map.rows * b.ssim_map.cols);
        
    }
    return mssim;
}