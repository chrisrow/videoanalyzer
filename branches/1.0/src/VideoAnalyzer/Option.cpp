
#include "stdafx.h"
#include "Option.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // atoi

//getopt 相关的全局变量
extern char *optarg;
extern int optind, opterr, optopt; 

// va <-f filename|-c cameraid|-n rtsp://xxx/yy> [-r record filename] [-s WxH;(PosX,PosY)] [-d] [-l] [-p]
// 
// -f filename:        播放本地视频文件
// -c cameraid:        打开本地摄像机
// -n rtsp://xxx/yy:   打开远程视频
// -r record filename: 录像输出的文件路径
// -s WxH;(PosX,PosY): 使用最简界面模式（只包含视频图像）W和H分别为窗口的宽和高,PosX和PosY为窗口左上角的坐标
// -x xml file:        载入对应的xml配置文件             
// -d:                 使用调测模式
// -l:                 输出日志。以程序文件名命名，以log为后缀
// -p:                 打开视频预览

//!@brief 命令行选项结构体
struct TOption
{
    char key;               //!< 参数标识
    bool usable;            //!< 是否设置了该选项
    const char* env;        //!< 环境变量
    const char* param_demo; //!< 示例参数
    const char* param1;     //!< 实际参数
    const char* comment;    //!< 对该选项的解释性文字
}OptTab[] = {
    {'f', false, "VA_FILE",    "file name", NULL, "Open video file."},
    {'c', false, "VA_CAMERA",  "camera id", NULL, "Open camera."},
    {'n', false, "VA_STREAM",  "URL",       NULL, "Open net stream."},
    {'r', false, "VA_RECORD",  "file name", NULL, "Record video."},
    {'s', false, "VA_SIMPLE",  "352x288;(100,100)",   NULL, "Simplest interface mode."},
    {'x', false, "VA_XMLFILE", "xml file name", NULL, "Load specific xml file."},
    {'d', false, "VA_DEBUG",   NULL,        NULL, "Debug mode."},
    {'l', false, "VA_LOGON",   NULL,        NULL, "Write log file."},
    {'p', false, "VA_PREVIEW", NULL,        NULL, "Display video image."},
    {'z', false, "VA_ZZZ",     NULL,        NULL, "Auto delete file and exit."},
};

//! 未知参数
const char Unknown_key = '?';
//! 显示帮助信息
const char Usage_key = 'h';

COption COption::m_instance;

COption&COption::Instance()
{
    return m_instance;
}

COption::COption():
    m_bInit(false), m_myName(NULL), m_optNum(0)
{
}

bool COption::init(int argc, char** argv)
{
    if(m_bInit)
    {
        return true;
    }
    m_optNum = sizeof(OptTab)/sizeof(OptTab[0]);
    if(!initOption())
    {
        return false;
    }
    
    //获取程序名称
    m_myName = Basename(argv[0]);
    
    //获取命令行选项
    int oc = -1;
    char optString[sizeof(OptTab)/sizeof(OptTab[0]) * 2] = {0};
    makeOptString(optString);

    //解析命令行
    while((oc = getopt(argc, argv, optString)) != -1)
    {
        if(Unknown_key == oc || Usage_key == oc)
        {
            usage();
            return false;
        }

        setOption(oc, optarg);
    }

    return true;
}

const char* COption::getFileName()
{
    return getOption('f');
}

int COption::getCameraID()
{
    const char* p = getOption('c');
    if (NULL == p)
    {
        return -1;
    } 
    else
    {
        return atoi(p);
    }
}

const char* COption::getURL()
{
    return getOption('n');
}

const char* COption::getRecordName()
{
    return getOption('r');
}

const char* COption::getSimple()
{
    return getOption('s');
}

const char* COption::getXMLFile()
{
    return getOption('x');
}

bool COption::isDebug()
{
    if(OptTab[getIndex('d')].usable && 
        OptTab[getIndex('d')].param1 != NULL && atoi(OptTab[getIndex('d')].param1) != 0)
    {
        return true;
    }
    return false;
}

bool COption::isLogOn()
{
    if(OptTab[getIndex('l')].usable && 
        OptTab[getIndex('l')].param1 != NULL && atoi(OptTab[getIndex('l')].param1) != 0)
    {
        return true;
    }
    return false;
}

bool COption::isPreview()
{
    if(OptTab[getIndex('p')].usable && 
        OptTab[getIndex('p')].param1 != NULL && atoi(OptTab[getIndex('p')].param1) != 0)
    {
        return true;
    }
    return false;
}

bool COption::isZZZ()
{
    if(OptTab[getIndex('z')].usable && 
        OptTab[getIndex('z')].param1 != NULL && atoi(OptTab[getIndex('z')].param1) != 0)
    {
        return true;
    }
    return false;
}

bool COption::initOption()
{
    for(int i = 0; i < m_optNum; i++)
    {
        if(NULL == OptTab[i].env)
        {
            continue;
        }

        const char* param = getenv(OptTab[i].env);
        if(param != NULL)
        {
            setOption(OptTab[i].key, param);
        }
    }
    return true;
}

bool COption::setOption(char key, const char* param1)
{
    OptTab[getIndex(key)].usable = true;
    OptTab[getIndex(key)].param1 = param1;
    if(NULL == OptTab[getIndex(key)].param1)
    {
        OptTab[getIndex(key)].param1 = "1";
    }
    return true;
}

const char* COption::getOption(char key)
{
    return OptTab[getIndex(key)].param1;
}

int COption::getIndex(char key)
{
    for(int i = 0; i < m_optNum; i++)
    {
        if(OptTab[i].key == key)
        {
            return i;
        }
    }

    return -1;
}

char COption::fromIndex(int index)
{
    return OptTab[index].key;
}

char* COption::makeOptString(char* str)
{
    for(int i = 0, j = 0; i < m_optNum; i++)
    {
        str[j++] = OptTab[i].key;
        if(OptTab[i].param_demo != NULL)
        {
            str[j++] = ':';
        }
    }
    //printf("str = %d, %s\n", m_optNum, str);
    return str;
}

void COption::usage()
{
    char cmd[1024] = {0};
    char tmp[128] = {0};
    int i = 0, j = 0;

    // 打印 Usage
    sprintf(cmd, "\nUsage: %s ", m_myName);
    for(i = 0, j = 0 ; i < m_optNum; i++)
    {
        if(OptTab[i].param_demo)
        {
            sprintf(tmp, "[ -%c %s ] ", OptTab[i].key, OptTab[i].param_demo);
        }
        else
        {
            sprintf(tmp, "[ -%c ] ", OptTab[i].key);
        }
        strcat(cmd, tmp);
    }
    printf("%s\n\n", cmd);

    // 打印 Description
    printf("Description\n");
    printf("       If a parameter is set both in command line and in environment variable, "
           "the value in command line work.\n");
    
    // 打印 Flags
    printf("Flags\n");
    for(i = 0, j = 0 ; i < m_optNum; i++)
    {
        if(NULL == OptTab[i].comment)
        {
            continue;
        }

        if(OptTab[i].param_demo != NULL)
        {
            printf("       -%c %s\n", OptTab[i].key, OptTab[i].param_demo);
        }
        else
        {
            printf("       -%c\n", OptTab[i].key);
        }
        printf("          %s\n", OptTab[i].comment);

        if(OptTab[i].env != NULL)
        {
            printf("          Environment variable '%s' also can set this option.\n", OptTab[i].env);
        }
    }
    printf("\n");
}


