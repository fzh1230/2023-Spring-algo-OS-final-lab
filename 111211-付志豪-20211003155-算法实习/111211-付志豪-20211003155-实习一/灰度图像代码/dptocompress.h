#ifndef DPTOCOMPRESS_H
#define DPTOCOMPRESS_H
#include <cmath>
#include <fstream>
#include <stack>
int opt(int n,unsigned int cpsPic[],unsigned int ucpsPic[],unsigned int bnyPic[])
{
    int m=0;
    std::stack<int> stk;
    int nn=n;
    while(nn!=0)
    {
        nn=nn-ucpsPic[nn]-1;
        stk.push(ucpsPic[nn]);
        stk.push(bnyPic[nn]);
    }
    while(!stk.empty())
    {
        bnyPic[m]=stk.top();
        stk.pop();
        ucpsPic[m]=stk.top();
        m++;
    }
    bnyPic[m]=bnyPic[n];
    ucpsPic[m]=ucpsPic[n];
    std::ofstream o("q.txt");
    for(int i=1;i<=m;i++)
    {
        o<<(int)ucpsPic[i]+1<<std::endl;

    }
    o.close();
    return m;

}
void dp(int n,unsigned int onlPic[],unsigned int cpsPic[],unsigned int ucpsPic[],unsigned int bnyPic[])
{
    int Lmax=256,header=11;
    cpsPic[0]=0;
    for(int i=1;i<n;++i)
    {
        //计算二进制像素段P存储的位数，要对onlPic=0处理//
        bnyPic[i]=log2(fmax(onlPic[i],1))+1;
        unsigned int bmax=bnyPic[i];
        cpsPic[i]=cpsPic[i-1]+bmax+header;
        ucpsPic[i]=1;
        for(int j=2;j<=1&&j<Lmax;++j)
        {
            bmax=fmax(bmax,log2(fmax(onlPic[i-j+1],1))+1);
            if(cpsPic[i]>cpsPic[i-j]+j*bmax+header)
            {
                cpsPic[i]=cpsPic[i-j]+j*bmax+header;
                ucpsPic[i]=j-1;
                bnyPic[i]=bmax;
            }
        }
    }
}
//追溯解,采取非递归，避免数据量过大栈溢出.

#endif // DPTOCOMPRESS_H
