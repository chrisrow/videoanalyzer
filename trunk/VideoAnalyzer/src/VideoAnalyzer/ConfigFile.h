
#pragma once

#include "Algorithm/ParabolaStruct.h"

#ifdef TIXML_USE_STL
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
#else
#include <stdio.h>
#endif

#if defined( WIN32 ) && defined( TUNE )
#include <crtdbg.h>
_CrtMemState startMemState;
_CrtMemState endMemState;
#endif

#include "Tinyxml/tinyxml.h"


class CCfgParse
{
public:
    CCfgParse();
    ~CCfgParse();

    //载入配置文件。载入成功后会自动关闭文件
    int Load(const char* szFileName);
    int Save(const char* szFileName);

    //载入全局参数
    //int LoadGolbal();

    //保存全局参数
    //int SaveGolbal();

    const char* GetGolbalParam(const char* szName);

    //载入某个通道的配置参数
    int LoadChannel(int iCh, ParamStruct& ps, ParamDistinguish& pd);

    //保存某个通道的配置参数
    int SaveChannel(int iCh, ParamStruct& ps, ParamDistinguish& pd);

private:
    TiXmlElement* SearchChannel(int iCh);
    TiXmlElement* SearchElement(TiXmlElement* xParentElem, const char* szName);

    int GetElemValue(TiXmlElement* xParentElement, const char* szName, int *pValue);
    int GetElemValue(TiXmlElement* xParentElement, const char* szName, float *pValue);
    int GetElemValue(TiXmlElement* xParentElement, const char* szName, LineSet *pValue);
    int GetElemValue(TiXmlElement* xParentElement, const char* szName, JudgeSet *pValue);
    int GetElemValue(TiXmlElement* xParentElement, const char* szName, JudgeSetFlt *pValue);

    int SetElemValue(TiXmlElement* xParentElement, const char* szName, const int *pValue);
    int SetElemValue(TiXmlElement* xParentElement, const char* szName, const float *pValue);
    int SetElemValue(TiXmlElement* xParentElement, const char* szName, const LineSet *pValue);
    int SetElemValue(TiXmlElement* xParentElement, const char* szName, const JudgeSet *pValue);
    int SetElemValue(TiXmlElement* xParentElement, const char* szName, const JudgeSetFlt *pValue);

private:
    TiXmlDocument m_xDoc;      //xml文件操作类
    TiXmlElement *m_xRootElem; //根节点
};
