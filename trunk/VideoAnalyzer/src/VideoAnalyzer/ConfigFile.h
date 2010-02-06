
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

    //���������ļ�������ɹ�����Զ��ر��ļ�
    int Load(const char* szFileName);
    int Save(const char* szFileName);

    //����ȫ�ֲ���
    //int LoadGolbal();

    //����ȫ�ֲ���
    //int SaveGolbal();

    const char* GetGolbalParam(const char* szName);

    //����ĳ��ͨ�������ò���
    int LoadChannel(int iCh, ParamStruct& ps, ParamDistinguish& pd);

    //����ĳ��ͨ�������ò���
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
    TiXmlDocument m_xDoc;      //xml�ļ�������
    TiXmlElement *m_xRootElem; //���ڵ�
};
