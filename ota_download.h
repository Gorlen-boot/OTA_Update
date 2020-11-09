#ifndef OTA_DOWNLOAD_H
#define OTA_DOWNLOAD_H

#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>

class ota_download
{
private:
    //复制将数据写入文件的回调函数，关于回调函数，可以参考C程序设计伴侣8.5.4小节介绍
    static uint64_t write_data(void * ptr,uint64_t size,uint64_t nmemb,FILE * stream)
    {
        uint64_t written  = fwrite(ptr,size,nmemb,stream);
        return written;
    }
public:
    ota_download(){}
    ~ota_download(){}

    void getfilename(char * url,char * name)
       {
           //找到URL中的最后一个‘/’字符
           char* pos = strrchr(url,'/');
           if(nullptr != pos)
           {
               //将URL中文件名（从pos+1开始知道字符串结束）复制到name字符串中
               strcpy(name,pos+1);
           }
       }
       //使用函数库所提供函数实现文件下载函数
       CURLcode download(char * url,char * out)  //char * url待下载文件的URL,char * out 下载后的文件名
       {
           CURL * curl = nullptr;
           FILE * fp = nullptr;
           CURLcode res;
           curl_global_init(1);
           //调用函数库中的curl_easy_init()函数完成初始化
           curl = curl_easy_init();
           if(curl)  //判断初始化是否成功
           {
               //以二进制可写方式打开文件，保存下载得到的数据
               fp = fopen(out,"wb");
               //根据libcurl的使用方法，设定下载的URL，写入函数以及写入的文件
               curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
               curl_easy_setopt(curl,CURLOPT_URL,url);
               curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
               curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);
               //curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
               //curl_easy_setopt(curl, CURLOPT_USERPWD, "SUREN:SUREN");
               //调用curl_easy_perform（）函数执行下载操作
               res = curl_easy_perform(curl);
               //下载完成，进行最后的清理工作
               curl_easy_cleanup(curl);
               //关闭文件
               fclose(fp);
               return res;
           }
           else
           {
               std::cout<<"CURL ERROR!!!"<<std::endl;
               //如果初始化失败，返回相应的错误代码
               return CURLE_FAILED_INIT;
           }
       }
};

#endif // OTA_DOWNLOAD_H
