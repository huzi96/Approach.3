#include "header.1.0.h"

//void getSimiar(set<patch> & patches, set<patch> & good, map<patch, patch> & indexMap)
//功能：对于patches中的每个元素,在good中找到与之最相似的,并将结果存入indexMap

void getSimilar(set<patch> & patches, set<patch> & good, map<patch, patch> & indexMap)
{
    vector<Scalar> ptchLab; //存放相应patch的Lab三通道值之和
    vector<Scalar> goodLab;
    
    set<patch>::iterator goodIt;
    set<patch>::iterator ptchIt;
    
    Mat tmpLab;
    
    for(goodIt = good.begin(); goodIt != good.end(); goodIt++)
    {
        cvtColor(goodIt->data, tmpLab, CV_BGR2Lab);
        
        Scalar tmpMean(0,0,0);
        
        cv::MatIterator_<Vec3b> it;
        for (it = tmpLab.begin<Vec3b>(); it != tmpLab.end<Vec3b>(); it++)//遍历整个tmpLab
        {
            for (int k = 0; k < 3; k++) //分别计算tmpLab三个通道的数值之和
                tmpMean[k] += (*it)[k];
        }
        goodLab.push_back(tmpMean);
    }
    
    
    for(ptchIt = patches.begin(); ptchIt != patches.end(); ptchIt++)
    {
        cvtColor(ptchIt->data, tmpLab, CV_BGR2Lab);
        
        Scalar tmpMean(0,0,0);
        
        cv::MatIterator_<Vec3b> it;
        for (it = tmpLab.begin<Vec3b>(); it != tmpLab.end<Vec3b>(); it++)//遍历整个tmpLab
        {
            for (int k = 0; k < 3; k++) //分别计算tmpLab三个通道的数值之和
                tmpMean[k] += (*it)[k];
        }
        ptchLab.push_back(tmpMean);
    }
    
    
    //tmpMean如果要除以PATCH_SIZE*PATCH_SIZE的话，需要在两个循环内部操作
    
    int cnt = 0;
    
    int i,j;
    for(i = 0, ptchIt = patches.begin(); ptchIt != patches.end(); ptchIt++, i++)//对于每一个需要打补丁的地方
    {
        Scalar meanP(ptchLab[i]);
        
        double min = 10000000; //INF
        
        patch * resValue=NULL;
        
        for (j = 0, goodIt = good.begin(); goodIt != good.end(); goodIt++, j++)//在good里找到和它最相似的的，放到一个map indexMap里
        {
            Scalar meanG(goodLab[j]);
            
            double dis = 0;
            for (int k = 0; k < 3; k++) //求两个均值向量欧拉距离
            {  dis += (meanP[k] - meanG[k])*(meanP[k] - meanG[k]); } //考虑精度和是否溢出
            dis = std::sqrt(dis);
            
            if (dis < min)
            {
                min = dis;
                resValue = (patch*)&(*goodIt);
            }
        }
        
        indexMap[*ptchIt] = *resValue;
        
        //cout << cnt++ << " mindist is :" << min << endl;
    }
    
}