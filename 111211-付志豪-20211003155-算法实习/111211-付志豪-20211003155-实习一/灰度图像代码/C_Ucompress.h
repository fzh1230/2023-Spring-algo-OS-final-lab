#ifndef C_UCOMPRESS_H
#define C_UCOMPRESS_H
#include "dptocompress.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QFileDialog>
#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include<QMessageBox>
#include<stack>
#include <windows.h>
bool compress(std::string name)
{
    std::ifstream fin(&name[0],std::ios::binary);
    if(!fin)
        return false;
    else
    {
        int tmpnums;
        int begin;
        BITMAPFILEHEADER bmp_fh;
        BITMAPINFOHEADER bmp_ih;
        RGBQUAD *bmp_rgb;

        int nums=bmp_ih.biClrUsed;
        bmp_rgb=new RGBQUAD[nums];
        fin.read((char*)&bmp_fh,sizeof(BITMAPFILEHEADER));
        fin.read((char*)&bmp_ih,sizeof(BITMAPINFOHEADER));
        fin.read((char*)bmp_rgb,sizeof(RGBQUAD)*nums);
        int line=(bmp_ih.biWidth*bmp_ih.biBitCount/8+3)/4*4;// 行数
        fin.seekg(bmp_fh.bfOffBits);
        unsigned char **Pix;
        Pix=new unsigned char*[bmp_ih.biHeight];
        for(int i=0;i<bmp_ih.biHeight;++i)
        {
            Pix[i]=new unsigned char[line];
            fin.read((char*)Pix[i],line);
        }
        fin.close();
        name=name+".mdp";
        std::ofstream fnscps(&name[0],std::ios::binary);
        if(!fnscps)
            return false;
        fnscps.write((char*)&bmp_fh,sizeof(BITMAPFILEHEADER));
        fnscps.write((char*)&bmp_ih,sizeof(BITMAPINFOHEADER));
        unsigned int *d=new unsigned int[nums+1];
        d[0]=0;
        //蛇
        for(int i=0;i<bmp_ih.biHeight;++i)
        {
            if (i % 2 == 0)
            {
                        for (int j = 0; j < bmp_ih.biWidth; j++)
                        {
                            d[bmp_ih.biWidth * i + j + 1] = Pix[i][j];
                        }
             }
            else {
                        for (int j = bmp_ih.biWidth - 1; j >= 0; j--)
                        {
                            d[bmp_ih.biWidth * i + bmp_ih.biWidth - j] = Pix[i][j];
                        }
                  }
        }
        unsigned int *cpsPic=new unsigned int[nums+1];
        cpsPic[0]=0;
        unsigned int *bnyPic=new unsigned int[nums+1];
        bnyPic[0]=0;
        unsigned int *ucpsPic=new unsigned int[nums+1];
        ucpsPic[0]=0;
        dp(nums,d,cpsPic,ucpsPic,bnyPic);
        tmpnums=opt(nums,cpsPic,ucpsPic,bnyPic);
        begin=1;
      //写东西
        int idx=0;
        int unsigned cpsnums=0;
        int value=0;
        for(int i=1;i<tmpnums;++i)
        {//存段长
            if(idx+8<64)
            {
                value<<=8;
                value=value|(ucpsPic[i]-1);
                idx+=8;
            }
            else if(idx+8==64)
            {
                value<<=8;
                value=value|(ucpsPic[i]-1);
                fnscps.write((char*)&value,sizeof(value));
                cpsnums++;
                value=0;
                idx=0;
            }
            else
            {
                int tmp=64-idx;
                value<<=tmp;
                value=value|((ucpsPic[i]-1)>>(8-tmp));
                fnscps.write((char*)&value,sizeof(value));
                cpsnums++;
                value=0;
                value=value|(((ucpsPic[i]-1)<<(64-8+tmp))>>(64-8+tmp));
                idx=8-tmp;
            }//3BIT
            if(idx+3<64)
            {
                value<<=3;
                value=value|(bnyPic[i]-1);
                idx+=3;
            }
            else if(idx+3==64)
            {
                value<<=3;
                value=value|(bnyPic[i]-1);
                fnscps.write((char*)&value,sizeof(value));
                cpsnums++;
                value=0;
                idx=0;
            }
            else
            {
                int tmp=64-idx;
                value<<=tmp;
                value=value|((bnyPic[i]-1)>>(3-tmp));
                fnscps.write((char*)&value,sizeof(value));
                cpsnums++;
                value=0;
                value=value|(((bnyPic[i]-1)<<(64-3+tmp))>>(64-3+tmp));
                idx=3-tmp;
            }
            //存像素数据
            for(int unsigned j=0;j<ucpsPic[i];++j);
            {
                if(idx+bnyPic[i]<64)
                {
                    value<<=bnyPic[i];
                    value=value|d[begin];
                    begin++;
                    idx+=bnyPic[i];
                }
                else if(idx+bnyPic[i]==64)
                {
                    value<<bnyPic[i];
                    value=value|d[begin];
                    begin++;
                    fnscps.write((char*)&value,sizeof(value));
                    cpsnums++;
                    value=0;
                    idx=0;
                }
                else
                {
                    int tmp=64-idx;//bny型前tmp
                    value<<=tmp;
                    value=value|(d[begin]>>(bnyPic[i]-tmp));
                    fnscps.write((char*)&value,sizeof(value));
                    cpsnums++;
                    value=0;
                    value=value|((d[begin]<<(64-bnyPic[i]+tmp))>>(64-bnyPic[i]+tmp));
                    begin++;
                    idx=bnyPic[i]-tmp;
                    if(begin==nums+1)
                    {
                        value<<=(64-idx);
                        fnscps.write((char*)&value,sizeof(value));
                        cpsnums++;
                        value=0;
                    }
                }
            }
        }
        fnscps.close();
        return true;
    }
}
bool unCompress(std::string name)
{
    if (name.substr(name.length() - 4) != ".mdp")
        {
            return false;
        }
    std::ifstream fin(&name[0], std::ios::binary);
    if(fin)
    {
        BITMAPFILEHEADER bmp_fh;
        BITMAPINFOHEADER bmp_ih;
        RGBQUAD* bmp_rgb;
        std::vector<char>data;
        int nums = bmp_ih.biClrUsed;
        bmp_rgb = new RGBQUAD[nums];
        fin.read((char*)&bmp_fh, sizeof(BITMAPFILEHEADER));
        fin.read((char*)&bmp_ih, sizeof(BITMAPINFOHEADER));
        fin.read((char*)bmp_rgb, sizeof(RGBQUAD) * nums);
        name += ".bmp";
        std::ofstream fout(&name[0], std::ios::binary);
        if (!fout)
          return false;
         fout.write((char*)&bmp_fh, sizeof(BITMAPFILEHEADER));
         fout.write((char*)&bmp_ih, sizeof(BITMAPINFOHEADER));
         fout.write((char*)bmp_rgb, sizeof(RGBQUAD) * nums);//写回
         std::vector<unsigned char> revvalue;
         int unsigned tmp = 0;
         while(!fin.eof())
         {
             fin.read((char*)&tmp, sizeof(tmp));
             revvalue.push_back(tmp);

         }
         revvalue.pop_back();
         tmp = 0;
         fin.close();
         int idx = 0;
         int nuber = 0;
         int bits = 0;
         while(true)
         {
             if (idx + 8 < 64)
             {//注意+1才是段长
                 nuber = ((revvalue[0] << idx) >> 56) + 1;
                 idx += 8;
             }
             else if (idx + 8 == 64)
             {
                 nuber = ((revvalue[0] << 56) >> 56) + 1;
                 idx = 0;
                 revvalue.erase(revvalue.begin());
             }
             else
             {
                 int fnums = 64 - idx;
                 nuber = (revvalue[0] << idx) >> idx;
                 idx = 8 - fnums;
                 revvalue.erase(revvalue.begin());
                 nuber = ((nuber << idx) | (revvalue[0] >> (64 - idx))) + 1;

             }
             //3
             if (idx + 3 < 64)
             {
                 bits = ((revvalue[0] << idx) >> 61) + 1;
                 idx += 3;
             }
             else if (idx + 3 == 64)
             {
                 bits = ((revvalue[0] << 61) >> 61) + 1;
                 idx = 0;
                 revvalue.erase(revvalue.begin());
             }
             else
             {
                 int fnums = 64 - idx;
                 bits = (revvalue[0] << idx) >> idx;
                 idx = 64 - fnums;
                 revvalue.erase(revvalue.begin());
                 bits = ((bits << idx) | (revvalue[0] >> (64 - idx))) + 1;
             }
             for(int i=0;i<nuber;++i)
             {
                 if(bits+idx<64)
                 {
                     tmp = ((revvalue[0] << idx) >> (64 - bits));
                     data.push_back(tmp);
                 if(data.size()==nums)
                 {
                      std::vector<unsigned char>Pix;
                      for (int i = 1; i <= bmp_ih.biHeight; i++)
                      {
                          for (int j = 1; j <= bmp_ih.biWidth; j++)
                          {
                              Pix.push_back(data[0]);
                              data.erase(data.begin());
                          }
                          if (i % 2 != 0)
                          {
                              while (Pix.size() > 0)
                              {
                                  fout.write((char*)&Pix[0], sizeof(Pix[0]));
                                  Pix.erase(Pix.begin());
                              }
                          }
                          else //偶数
                          {
                              while (Pix.size() > 0)
                              {
                                  fout.write((char*)&Pix.back(), sizeof(Pix.back()));
                                  Pix.pop_back();
                              }
                          }
                      }
                      fout.close();
                      return 1;
                 }
                  idx += bits;
                 }
                 else if(bits+idx==64)
                 {
                     tmp = (revvalue[0] << idx) >> idx;
                     data.push_back(tmp);
                     if(data.size()==nums)
                     {
                          std::vector<unsigned char>Pix;
                          for (int i = 1; i <= bmp_ih.biHeight; i++)
                          {
                              for (int j = 1; j <= bmp_ih.biWidth; j++)
                              {
                                  Pix.push_back(data[0]);
                                  data.erase(data.begin());
                              }
                              if (i % 2 != 0)
                              {
                                  while (Pix.size() > 0)
                                  {
                                      fout.write((char*)&Pix[0], sizeof(Pix[0]));
                                      Pix.erase(Pix.begin());
                                  }
                              }
                              else //偶数
                              {
                                  while (Pix.size() > 0)
                                  {
                                      fout.write((char*)&Pix.back(), sizeof(Pix.back()));
                                      Pix.pop_back();
                                  }
                              }
                          }
                          fout.close();
                          return 1;
                     }
                     idx = 0;
                     revvalue.erase(revvalue.begin());
                 }
                 else if(bits+idx>64)
                 {
                     int fnums = 64 - idx;
                     tmp = (revvalue[0] << idx) >> idx;
                     idx = bits - fnums;
                     revvalue.erase(revvalue.begin());
                     tmp = (tmp << idx) | ((revvalue[0]) >> (64 - idx));
                     data.push_back(tmp);
                     if(data.size()==nums)
                     {
                          std::vector<unsigned char>Pix;
                          for (int i = 1; i <= bmp_ih.biHeight; i++)
                          {
                              for (int j = 1; j <= bmp_ih.biWidth; j++)
                              {
                                  Pix.push_back(data[0]);
                                  data.erase(data.begin());
                              }
                              if (i % 2 != 0)
                              {
                                  while (Pix.size() > 0)
                                  {
                                      fout.write((char*)&Pix[0], sizeof(Pix[0]));
                                      Pix.erase(Pix.begin());
                                  }
                              }
                              else
                              {
                                  while (Pix.size() > 0)
                                  {
                                      fout.write((char*)&Pix.back(), sizeof(Pix.back()));
                                      Pix.pop_back();
                                  }
                              }
                          }
                          fout.close();
                          return 1;
                     }
                 }
             }

         }

    }
    else
        return false;
}






#endif // C_UCOMPRESS_H
