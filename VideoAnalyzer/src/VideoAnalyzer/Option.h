#pragma once

#ifndef _COption_H_
#define _COption_H_

#include "getopt.h"

//!@brief 获取不带路径的文件名。GNU和POSIX带有相同功能的函数:char *basename(char *path)
//!@param path 完整的文件名
//!@return 不带路径的文件名
inline const char* Basename(const char* path)
{
    if(NULL == path)
    {
        return NULL;
    }
    for(int i= strlen(path)-1; i >= 0; i--)
    {
        if(*(path+i) == '\\' || *(path+i) == '/')
        {
            return path+i+1;
        }
    }
    return path;
}

//!@brief 获取执行参数
//!@brief Usage: utmain [-a] [-h] [-f FilterString] [-r RepeatTimes]
//!@brief 
//!@brief    命令参数：
//!@brief    - -a 输出所有test的执行结果。默认只输出失败的test的结果
//!@brief    - -h 显示帮助
//!@brief    - -f FilterString 设置过滤器，执行指定的test
//!@brief    - -r RepeatTimes  指定每个test的执行次数
//!@note 从环境变量和命令行中获取参数。如果两者都设置了同一个参数，则以命令行中的为准。
class COption
{
public:
    static COption& Instance();
    
    //!@brief 初始化参数选项，解析命令行参数
    //!@param argc 命令行参数个数
    //!@param argv 命令行参数
    //!@return  函数执行是否成功
    //!@note 先对所有支持的选项进行初始化，然后解析命令行。
    bool init(int argc, char** argv);
    
    //!@brief 显示帮助信息
    void usage();
    
    const char* getFileName();    //打开的视频文件名称
    int getCameraID();            //摄像机ID
    const char* getURL();         //流媒体网址
    const char* getRecordName();  //录像文件名
    const char* getSimple();      //简单界面模式，窗口大小和窗口位置
    bool isDebug();               //调测模式
    bool isLogOn();               //记录日志
    bool isPreview();             //显示预览

protected:
    COption();
    
private:
    //!@brief 从环境变量初始化参数
    //!@return  函数执行是否成功
    bool initOption();
    bool setOption(char key, const char* param1 = NULL);
    const char* getOption(char key);
    
    char* makeOptString(char* str);
    int getIndex(char key);
    char fromIndex(int index);
    
private:
    static COption m_instance;
    bool m_bInit;
    const char* m_myName;
    int m_optNum;
};


#endif

