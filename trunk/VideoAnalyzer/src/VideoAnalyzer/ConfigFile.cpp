#include "stdafx.h"


#include "ConfigFile.h"

////////////////////////////////////////////////////////////////////////////////



const char* CFG_ROOT = "configure";
const char* ATTR_CHANNEL = "channel";

#define LOG_DEBUG(type, fmt, ...) printf("[%d]>>", __LINE__),printf(fmt, __VA_ARGS__),printf("\n")

enum
{
    DEBUG_INF,
    DEBUG_WARN,
    DEBUG_ERR
};

#define GET_VALUE(elem, name, var) \
    if (!this->GetElemValue(elem, name, &var))\
    {\
        LOG_DEBUG(DEBUG_ERR, "Get value of '%s' error", name);\
    }

#define SET_VALUE(elem, name, var) \
    if (!this->SetElemValue(elem, name, &var))\
    {\
        LOG_DEBUG(DEBUG_ERR, "Set value of '%s' error", name);\
    }



#define  ERR_XML_BASE 5000


CCfgParse::CCfgParse()
: m_xRootElem(NULL)
{

}

CCfgParse::~CCfgParse()
{

}

//载入配置文件
int CCfgParse::Load(const char* szFileName)
{
    bool loadOkay = m_xDoc.LoadFile(szFileName);

    LOG_DEBUG(DEBUG_INF, "HAHA");

    if ( !loadOkay )
    {
        LOG_DEBUG(DEBUG_ERR, "Could not load file: %s", szFileName);
        LOG_DEBUG(DEBUG_ERR, "%s", m_xDoc.ErrorDesc());
        return 0;
    }

    m_xRootElem = m_xDoc.RootElement();
    if( strcmp(m_xRootElem->Value(),CFG_ROOT) != 0 )
    {
        LOG_DEBUG(DEBUG_ERR, "Not Found RootElement:'%s'", CFG_ROOT);
        return 0;
    }

    return 1;
}

int CCfgParse::Save(const char* szFileName)
{
    return m_xDoc.SaveFile(szFileName);
}

int CCfgParse::GetElemValue(TiXmlElement* xParentElement, const char* szName, int *pValue)
{
    const char* NODE_INT = "int";
    const char* ATTR_VALUE = "value";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个int值
    if (xElem->Attribute(ATTR_VALUE, pValue))
    {
        return 1;
    }

    //获取int数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_INT);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", ATTR_VALUE, szName);
        return 0;
    }
    xValueElem->Attribute(ATTR_VALUE, pValue);
    xValueElem = xValueElem->NextSiblingElement(NODE_INT);

    while(xValueElem)
    {
        pValue++;
        xValueElem->Attribute("value", pValue);
        xValueElem = xValueElem->NextSiblingElement(NODE_INT);
    }
    
    return 1;
}

int CCfgParse::GetElemValue(TiXmlElement* xParentElement, const char* szName, float *pValue)
{
    const char* NODE_FLOAT = "float";
    const char* ATTR_VALUE = "value";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个float值
    if (xElem->Attribute(ATTR_VALUE, pValue))
    {
        return 1;
    }

    //获取float数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_FLOAT);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", ATTR_VALUE, szName);
        return 0;
    }
    xValueElem->Attribute(ATTR_VALUE, pValue);
    xValueElem = xValueElem->NextSiblingElement(NODE_FLOAT);

    while(xValueElem)
    {
        pValue++;
        xValueElem->Attribute(ATTR_VALUE, pValue);
        xValueElem = xValueElem->NextSiblingElement(NODE_FLOAT);
    }

    return 1;
}

int CCfgParse::GetElemValue(TiXmlElement* xParentElement, const char* szName, LineSet *pValue)
{
    const char* NODE_LINESET = "LineSet";
    const char* ATTR_FLAG = "bFlag";
    const char* ATTR_BX = "BeginPointX";
    const char* ATTR_BY = "BeginPointY";
    const char* ATTR_EX = "EndPointX";
    const char* ATTR_EY = "EndPointY";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个LineSet
    if (xElem->Attribute(ATTR_FLAG, &(pValue->bFlag)) 
        && xElem->Attribute(ATTR_BX, &(pValue->BeginPointX)) 
        && xElem->Attribute(ATTR_BY, &(pValue->BeginPointY)) 
        && xElem->Attribute(ATTR_EX, &(pValue->EndPointX)) 
        && xElem->Attribute(ATTR_EY, &(pValue->EndPointY))   )
    {
        return 1;
    }

    //获取LineSet数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_LINESET);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", NODE_LINESET, szName);
        return 0;
    }

    xValueElem->Attribute(ATTR_FLAG, &(pValue->bFlag));
    xValueElem->Attribute(ATTR_BX, &(pValue->BeginPointX)) ;
    xValueElem->Attribute(ATTR_BY, &(pValue->BeginPointY)) ;
    xValueElem->Attribute(ATTR_EX, &(pValue->EndPointX)) ;
    xValueElem->Attribute(ATTR_EY, &(pValue->EndPointY)) ;

    xValueElem = xValueElem->NextSiblingElement(NODE_LINESET);

    while(xValueElem)
    {
        pValue++;
        xValueElem->Attribute(ATTR_FLAG, &(pValue->bFlag));
        xValueElem->Attribute(ATTR_BX, &(pValue->BeginPointX)) ;
        xValueElem->Attribute(ATTR_BY, &(pValue->BeginPointY)) ;
        xValueElem->Attribute(ATTR_EX, &(pValue->EndPointX)) ;
        xValueElem->Attribute(ATTR_EY, &(pValue->EndPointY)) ;
        xValueElem = xValueElem->NextSiblingElement(NODE_LINESET);
    }

    return 1;
}

int CCfgParse::GetElemValue(TiXmlElement* xParentElement, const char* szName, JudgeSet *pValue)
{
    const char* NODE_JUDGESET = "JudgeSet";
    const char* ATTR_1 = "FirstValue";
    const char* ATTR_2 = "SecondValue";
    const char* ATTR_3 = "ThirdValue";
    const char* ATTR_4 = "FouthValue";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个LineSet
    if (xElem->Attribute(ATTR_1, &(pValue->FirstValue)) 
        && xElem->Attribute(ATTR_2, &(pValue->SecondValue)) 
        && xElem->Attribute(ATTR_3, &(pValue->ThirdValue)) 
        && xElem->Attribute(ATTR_4, &(pValue->FouthValue))   )
    {
        return 1;
    }

    //获取LineSet数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_JUDGESET);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", NODE_JUDGESET, szName);
        return 0;
    }

    xValueElem->Attribute(ATTR_1, &(pValue->FirstValue)) ;
    xValueElem->Attribute(ATTR_2, &(pValue->SecondValue)) ;
    xValueElem->Attribute(ATTR_3, &(pValue->ThirdValue)) ;
    xValueElem->Attribute(ATTR_4, &(pValue->FouthValue)) ;

    xValueElem = xValueElem->NextSiblingElement(NODE_JUDGESET);

    while(xValueElem)
    {
        pValue++;
        xValueElem->Attribute(ATTR_1, &(pValue->FirstValue)) ;
        xValueElem->Attribute(ATTR_2, &(pValue->SecondValue)) ;
        xValueElem->Attribute(ATTR_3, &(pValue->ThirdValue)) ;
        xValueElem->Attribute(ATTR_4, &(pValue->FouthValue)) ;
        xValueElem = xValueElem->NextSiblingElement(NODE_JUDGESET);
    }

    return 1;
}

int CCfgParse::GetElemValue(TiXmlElement* xParentElement, const char* szName, JudgeSetFlt *pValue)
{
    const char* NODE_JUDGESETFLT = "JudgeSetFlt";
    const char* ATTR_1 = "FirstValue";
    const char* ATTR_2 = "SecondValue";
    const char* ATTR_3 = "ThirdValue";
    const char* ATTR_4 = "FouthValue";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个LineSet
    if (xElem->Attribute(ATTR_1, &(pValue->FirstValue)) 
        && xElem->Attribute(ATTR_2, &(pValue->SecondValue)) 
        && xElem->Attribute(ATTR_3, &(pValue->ThirdValue)) 
        && xElem->Attribute(ATTR_4, &(pValue->FouthValue))   )
    {
        return 1;
    }

    //获取LineSet数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_JUDGESETFLT);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", NODE_JUDGESETFLT, szName);
        return 0;
    }

    xValueElem->Attribute(ATTR_1, &(pValue->FirstValue)) ;
    xValueElem->Attribute(ATTR_2, &(pValue->SecondValue)) ;
    xValueElem->Attribute(ATTR_3, &(pValue->ThirdValue)) ;
    xValueElem->Attribute(ATTR_4, &(pValue->FouthValue)) ;

    xValueElem = xValueElem->NextSiblingElement(NODE_JUDGESETFLT);

    while(xValueElem)
    {
        pValue++;
        xValueElem->Attribute(ATTR_1, &(pValue->FirstValue)) ;
        xValueElem->Attribute(ATTR_2, &(pValue->SecondValue)) ;
        xValueElem->Attribute(ATTR_3, &(pValue->ThirdValue)) ;
        xValueElem->Attribute(ATTR_4, &(pValue->FouthValue)) ;
        xValueElem = xValueElem->NextSiblingElement(NODE_JUDGESETFLT);
    }

    return 1;
}

int CCfgParse::SetElemValue(TiXmlElement* xParentElement, const char* szName, const int *pValue)
{
    const char* NODE_INT = "int";
    const char* ATTR_VALUE = "value";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个int值
    int iOrgValue = 0;
    if (xElem->Attribute(ATTR_VALUE, &iOrgValue))
    {
        xElem->SetAttribute(ATTR_VALUE, *pValue);
        return 1;
    }

    //获取int数组
    TiXmlElement *xValueElem = this->SearchElement(xParentElement, NODE_INT);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", ATTR_VALUE, szName);
        return 0;
    }
    xValueElem->SetAttribute(ATTR_VALUE, *pValue);
    xValueElem = xValueElem->NextSiblingElement(NODE_INT);

    while(xValueElem)
    {
        pValue++;
        xValueElem->SetAttribute("value", *pValue);
        xValueElem = xValueElem->NextSiblingElement(NODE_INT);
    }

    return 1;
}

int CCfgParse::SetElemValue(TiXmlElement* xParentElement, const char* szName, const float *pValue)
{
    const char* NODE_FLOAT = "float";
    const char* ATTR_VALUE = "value";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个int值
    float fOrgValue = 0;
    if (xElem->Attribute(ATTR_VALUE, &fOrgValue))
    {
        xElem->SetAttribute(ATTR_VALUE, *pValue);
        return 1;
    }

    //获取int数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_FLOAT);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", ATTR_VALUE, szName);
        return 0;
    }
    xValueElem->SetAttribute(ATTR_VALUE, *pValue);
    xValueElem = xValueElem->NextSiblingElement(NODE_FLOAT);

    while(xValueElem)
    {
        pValue++;
        xValueElem->SetAttribute("value", *pValue);
        xValueElem = xValueElem->NextSiblingElement(NODE_FLOAT);
    }

    return 1;
}

int CCfgParse::SetElemValue(TiXmlElement* xParentElement, const char* szName, const LineSet *pValue)
{
    const char* NODE_LINESET = "LineSet";
    const char* ATTR_FLAG = "bFlag";
    const char* ATTR_BX = "BeginPointX";
    const char* ATTR_BY = "BeginPointY";
    const char* ATTR_EX = "EndPointX";
    const char* ATTR_EY = "EndPointY";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个LineSet
    LineSet tmp;
    if (xElem->Attribute(ATTR_FLAG, &(tmp.bFlag)) 
        && xElem->Attribute(ATTR_BX, &(tmp.BeginPointX)) 
        && xElem->Attribute(ATTR_BY, &(tmp.BeginPointY)) 
        && xElem->Attribute(ATTR_EX, &(tmp.EndPointX)) 
        && xElem->Attribute(ATTR_EY, &(tmp.EndPointY))   )
    {
        xElem->SetAttribute(ATTR_FLAG, pValue->bFlag);
        xElem->SetAttribute(ATTR_BX, pValue->BeginPointX) ;
        xElem->SetAttribute(ATTR_BY, pValue->BeginPointY) ;
        xElem->SetAttribute(ATTR_EX, pValue->EndPointX) ;
        xElem->SetAttribute(ATTR_EY, pValue->EndPointY) ;

        return 1;
    }

    //获取LineSet数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_LINESET);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", NODE_LINESET, szName);
        return 0;
    }

    xValueElem->SetAttribute(ATTR_FLAG, pValue->bFlag);
    xValueElem->SetAttribute(ATTR_BX, pValue->BeginPointX) ;
    xValueElem->SetAttribute(ATTR_BY, pValue->BeginPointY) ;
    xValueElem->SetAttribute(ATTR_EX, pValue->EndPointX) ;
    xValueElem->SetAttribute(ATTR_EY, pValue->EndPointY) ;

    xValueElem = xValueElem->NextSiblingElement(NODE_LINESET);

    while(xValueElem)
    {
        pValue++;
        xValueElem->SetAttribute(ATTR_FLAG, pValue->bFlag);
        xValueElem->SetAttribute(ATTR_BX, pValue->BeginPointX) ;
        xValueElem->SetAttribute(ATTR_BY, pValue->BeginPointY) ;
        xValueElem->SetAttribute(ATTR_EX, pValue->EndPointX) ;
        xValueElem->SetAttribute(ATTR_EY, pValue->EndPointY) ;
        xValueElem = xValueElem->NextSiblingElement(NODE_LINESET);
    }

    return 1;
}

int CCfgParse::SetElemValue(TiXmlElement* xParentElement, const char* szName, const JudgeSet *pValue)
{
    const char* NODE_JUDGESET = "JudgeSet";
    const char* ATTR_1 = "FirstValue";
    const char* ATTR_2 = "SecondValue";
    const char* ATTR_3 = "ThirdValue";
    const char* ATTR_4 = "FouthValue";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个LineSet
    JudgeSet tmp;
    if (xElem->Attribute(ATTR_1, &(tmp.FirstValue)) 
        && xElem->Attribute(ATTR_2, &(tmp.SecondValue)) 
        && xElem->Attribute(ATTR_3, &(tmp.ThirdValue)) 
        && xElem->Attribute(ATTR_4, &(tmp.FouthValue))   )
    {
        xElem->SetAttribute(ATTR_1, pValue->FirstValue) ;
        xElem->SetAttribute(ATTR_2, pValue->SecondValue) ;
        xElem->SetAttribute(ATTR_3, pValue->ThirdValue) ;
        xElem->SetAttribute(ATTR_4, pValue->FouthValue) ;

        return 1;
    }

    //获取LineSet数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_JUDGESET);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", NODE_JUDGESET, szName);
        return 0;
    }

    xValueElem->SetAttribute(ATTR_1, pValue->FirstValue) ;
    xValueElem->SetAttribute(ATTR_2, pValue->SecondValue) ;
    xValueElem->SetAttribute(ATTR_3, pValue->ThirdValue) ;
    xValueElem->SetAttribute(ATTR_4, pValue->FouthValue) ;

    xValueElem = xValueElem->NextSiblingElement(NODE_JUDGESET);

    while(xValueElem)
    {
        pValue++;
        xValueElem->SetAttribute(ATTR_1, pValue->FirstValue) ;
        xValueElem->SetAttribute(ATTR_2, pValue->SecondValue) ;
        xValueElem->SetAttribute(ATTR_3, pValue->ThirdValue) ;
        xValueElem->SetAttribute(ATTR_4, pValue->FouthValue) ;
        xValueElem = xValueElem->NextSiblingElement(NODE_JUDGESET);
    }

    return 1;
}

int CCfgParse::SetElemValue(TiXmlElement* xParentElement, const char* szName, const JudgeSetFlt *pValue)
{
    const char* NODE_JUDGESET = "JudgeSet";
    const char* ATTR_1 = "FirstValue";
    const char* ATTR_2 = "SecondValue";
    const char* ATTR_3 = "ThirdValue";
    const char* ATTR_4 = "FouthValue";

    TiXmlElement* xElem = SearchElement(xParentElement, szName);
    if(NULL == xElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info", szName);
        return NULL;
    }

    //获取单个LineSet
    JudgeSet tmp;
    if (xElem->Attribute(ATTR_1, &(tmp.FirstValue)) 
        && xElem->Attribute(ATTR_2, &(tmp.SecondValue)) 
        && xElem->Attribute(ATTR_3, &(tmp.ThirdValue)) 
        && xElem->Attribute(ATTR_4, &(tmp.FouthValue))   )
    {
        xElem->SetAttribute(ATTR_1, pValue->FirstValue) ;
        xElem->SetAttribute(ATTR_2, pValue->SecondValue) ;
        xElem->SetAttribute(ATTR_3, pValue->ThirdValue) ;
        xElem->SetAttribute(ATTR_4, pValue->FouthValue) ;

        return 1;
    }

    //获取LineSet数组
    TiXmlElement *xValueElem = this->SearchElement(xElem, NODE_JUDGESET);
    if (!xValueElem)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' in '%s'", NODE_JUDGESET, szName);
        return 0;
    }

    xValueElem->SetAttribute(ATTR_1, pValue->FirstValue) ;
    xValueElem->SetAttribute(ATTR_2, pValue->SecondValue) ;
    xValueElem->SetAttribute(ATTR_3, pValue->ThirdValue) ;
    xValueElem->SetAttribute(ATTR_4, pValue->FouthValue) ;

    xValueElem = xValueElem->NextSiblingElement(NODE_JUDGESET);

    while(xValueElem)
    {
        pValue++;
        xValueElem->SetAttribute(ATTR_1, pValue->FirstValue) ;
        xValueElem->SetAttribute(ATTR_2, pValue->SecondValue) ;
        xValueElem->SetAttribute(ATTR_3, pValue->ThirdValue) ;
        xValueElem->SetAttribute(ATTR_4, pValue->FouthValue) ;
        xValueElem = xValueElem->NextSiblingElement(NODE_JUDGESET);
    }

    return 1;
}

//载入某个通道的参数
int CCfgParse::LoadChannel(int iCh, ParamStruct& ps, ParamDistinguish& pd)
{
    if (iCh < 0)
    {
        LOG_DEBUG(DEBUG_ERR, "Error channel:  %d", iCh);
        return 0;
    }

    //找到对应的视频通道
    TiXmlElement *xChannelElement = this->SearchChannel(iCh);
    if(NULL == xChannelElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No channel info:  %d", iCh);
        return 0;
    }

    //ParamStruct ///////////////////////////////////////////////
    const char* NODE_PARAMSTRUCT = "ParamStruct";
    TiXmlElement *xPSElement = this->SearchElement(xChannelElement, NODE_PARAMSTRUCT);
    if(NULL == xPSElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No 'ParamStruct' info in channel %d", iCh);
        return 0;
    }

	//初始化类型
    GET_VALUE(xPSElement, "ChangeStyle", ps.iStyleChange);

    //int
    GET_VALUE(xPSElement, "bSensitiveFlag", ps.bSensitiveFlag);
    GET_VALUE(xPSElement, "bTransLensImage", ps.bTransLensImage);
    GET_VALUE(xPSElement, "bBinarizeSubThreshold", ps.iBinarizeSubThreshold);
    GET_VALUE(xPSElement, "bNightSubThreshold", ps.iNightSubThreshold);
    GET_VALUE(xPSElement, "iImfilterSingleThreshold", ps.iImfilterSingleThreshold);
    GET_VALUE(xPSElement, "iImdilateThreshold", ps.iImdilateThreshold);
    GET_VALUE(xPSElement, "iWhiteSpotNumMax", ps.iWhiteSpotNumMax);
    GET_VALUE(xPSElement, "iWhiteSpotNumMin", ps.iWhiteSpotNumMin);
    GET_VALUE(xPSElement, "iXTrackContinueThreshold", ps.iXTrackContinueThreshold);
    GET_VALUE(xPSElement, "iXTrackOffsetValue", ps.iXTrackOffsetValue);
    GET_VALUE(xPSElement, "iTrackMaxFrameNum", ps.iTrackMaxFrameNum);
    GET_VALUE(xPSElement, "iLittleRegionValue", ps.iLittleRegionTop);
	GET_VALUE(xPSElement, "iNightRangeVal", ps.iNightRangeVal);

	GET_VALUE(xPSElement, "iCurveLeftVal", ps.iCurveLeftVal);
	GET_VALUE(xPSElement, "iCurveRightVal", ps.iCurveRightVal);

	GET_VALUE(xPSElement, "iPersonFlag", ps.iPersonFlag);
	GET_VALUE(xPSElement, "iPersonWhitePotNum", ps.iPersonWhitePotNum);

    //LineSet
    GET_VALUE(xPSElement, "tRectLittleRegion", ps.tRectLittleRegion);
    GET_VALUE(xPSElement, "tRectTreeLittleRegion", ps.tRectTreeLittleRegion);
	GET_VALUE(xPSElement, "tLineCurverRange", ps.tLineCurverRange);
	GET_VALUE(xPSElement, "tLineStraightFirst", ps.tLineStraightFirst);
	GET_VALUE(xPSElement, "tLineStraightSecond", ps.tLineStraightSecond);
	GET_VALUE(xPSElement, "tLineBlackLeft", ps.tLineBlackLeft);
	GET_VALUE(xPSElement, "tLineBlackRight", ps.tLineBlackRight);

    //注意：对于数组，要加“*”       
    GET_VALUE(xPSElement, "tRectBlackBlock", *ps.tRectBlackBlock);    
    GET_VALUE(xPSElement, "tNightRange", *ps.tNightRange);
//	GET_VALUE(xPSElement, "fPersonRange", *ps.iPersonRange);
    

    //ParamDistinguish ///////////////////////////////////////////////
    const char* NODE_PARAMDISTINGUISH = "ParamDistinguish";
    TiXmlElement *xPDElement = this->SearchElement(xChannelElement, NODE_PARAMDISTINGUISH);
    if(NULL == xPDElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No 'ParamStruct' info in channel %d", iCh);
        return 0;
    }

    //JudgeSet
    GET_VALUE(xPDElement, "TrackNumMax", pd.TrackNumMax);
    GET_VALUE(xPDElement, "TrackNumMin", pd.TrackNumMin);
    GET_VALUE(xPDElement, "LostNumVal", pd.LostNumVal);
    GET_VALUE(xPDElement, "MigrationDiff", pd.MigrationDiff);
    GET_VALUE(xPDElement, "OriginCurrDis", pd.OriginCurrDis);
    GET_VALUE(xPDElement, "YHeightVal", pd.YHeightVal);
    GET_VALUE(xPDElement, "XContValue", pd.XContValue);
    GET_VALUE(xPDElement, "XMatchValue", pd.XMatchValue);
    GET_VALUE(xPDElement, "YMatchValue", pd.YMatchValue);
    GET_VALUE(xPDElement, "FindObjValue", pd.FindObjValue);
    GET_VALUE(xPDElement, "RiseNumVal", pd.RiseNumVal);
    GET_VALUE(xPDElement, "DownNumVal", pd.DownNumVal);
    GET_VALUE(xPDElement, "FindObjValMax", pd.FindObjValMax);
    GET_VALUE(xPDElement, "FindObjValMin", pd.FindObjValMin);
    GET_VALUE(xPDElement, "WhiteSpotVal", pd.WhiteSpotVal);
    GET_VALUE(xPDElement, "TopPointVal", pd.TopPointVal);

    //JudgeSetFlt
    GET_VALUE(xPDElement, "fFindObjInverse", pd.fFindObjInverse);
    GET_VALUE(xPDElement, "fXMatchInverse", pd.fXMatchInverse);
    GET_VALUE(xPDElement, "fYMatchInverse", pd.fYMatchInverse);
    GET_VALUE(xPDElement, "fXContInverse", pd.fXContInverse);

    return 1;
}


int CCfgParse::SaveChannel(int iCh, ParamStruct& ps, ParamDistinguish& pd)
{
    if (iCh < 0)
    {
        LOG_DEBUG(DEBUG_ERR, "Error channel:  %d", iCh);
        return 0;
    }

    //找到对应的视频通道
    TiXmlElement *xChannelElement = this->SearchChannel(iCh);
    if(NULL == xChannelElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No channel info:  %d", iCh);
        return 0;
    }

    //ParamStruct ///////////////////////////////////////////////
    const char* NODE_PARAMSTRUCT = "ParamStruct";
    TiXmlElement *xPSElement = this->SearchElement(xChannelElement, NODE_PARAMSTRUCT);
    if(NULL == xPSElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No 'ParamStruct' info in channel %d", iCh);
        return 0;
    }

	//初始化类型
	SET_VALUE(xPSElement, "ChangeStyle", ps.iStyleChange);

	SET_VALUE(xPSElement, "bSensitiveFlag", ps.bSensitiveFlag);
	SET_VALUE(xPSElement, "bTransLensImage", ps.bTransLensImage);
	SET_VALUE(xPSElement, "bBinarizeSubThreshold", ps.iBinarizeSubThreshold);
	SET_VALUE(xPSElement, "bNightSubThreshold", ps.iNightSubThreshold);
	SET_VALUE(xPSElement, "iImfilterSingleThreshold", ps.iImfilterSingleThreshold);
	SET_VALUE(xPSElement, "iImdilateThreshold", ps.iImdilateThreshold);
	SET_VALUE(xPSElement, "iWhiteSpotNumMax", ps.iWhiteSpotNumMax);
	SET_VALUE(xPSElement, "iWhiteSpotNumMin", ps.iWhiteSpotNumMin);
	SET_VALUE(xPSElement, "iXTrackContinueThreshold", ps.iXTrackContinueThreshold);
	SET_VALUE(xPSElement, "iXTrackOffsetValue", ps.iXTrackOffsetValue);
	SET_VALUE(xPSElement, "iTrackMaxFrameNum", ps.iTrackMaxFrameNum);
	SET_VALUE(xPSElement, "iLittleRegionValue", ps.iLittleRegionTop);
	SET_VALUE(xPSElement, "iNightRangeVal", ps.iNightRangeVal);

	SET_VALUE(xPSElement, "iCurveLeftVal", ps.iCurveLeftVal);
	SET_VALUE(xPSElement, "iCurveRightVal", ps.iCurveRightVal);

	SET_VALUE(xPSElement, "iPersonFlag", ps.iPersonFlag);
	SET_VALUE(xPSElement, "iPersonWhitePotNum", ps.iPersonWhitePotNum);

	//LineSet
	SET_VALUE(xPSElement, "tRectLittleRegion", ps.tRectLittleRegion);
	SET_VALUE(xPSElement, "tRectTreeLittleRegion", ps.tRectTreeLittleRegion);
	SET_VALUE(xPSElement, "tLineCurverRange", ps.tLineCurverRange);
	SET_VALUE(xPSElement, "tLineStraightFirst", ps.tLineStraightFirst);
	SET_VALUE(xPSElement, "tLineStraightSecond", ps.tLineStraightSecond);
	SET_VALUE(xPSElement, "tLineBlackLeft", ps.tLineBlackLeft);
	SET_VALUE(xPSElement, "tLineBlackRight", ps.tLineBlackRight);

	//注意：对于数组，要加“*”       
	SET_VALUE(xPSElement, "tRectBlackBlock", *ps.tRectBlackBlock);    
	SET_VALUE(xPSElement, "tNightRange", *ps.tNightRange);
	//	SET_VALUE(xPSElement, "fPersonRange", *ps.iPersonRange);

/*    //int
    SET_VALUE(xPSElement, "bSensitiveFlag", ps.bSensitiveFlag);
    SET_VALUE(xPSElement, "bTransLensImage", ps.bTransLensImage);
    SET_VALUE(xPSElement, "bLittleRegionFlag", ps.bLittleRegionFlag);
    SET_VALUE(xPSElement, "bTreeLittleFlag", ps.bTreeLittleFlag);

    //float array
    SET_VALUE(xPSElement, "fLineFirstLocation", *ps.fLineFirstLocation);
    SET_VALUE(xPSElement, "fLineSecondLocation", *ps.fLineSecondLocation);

    //int
    SET_VALUE(xPSElement, "bBinarizeSubThreshold", ps.bBinarizeSubThreshold);
    SET_VALUE(xPSElement, "bNightSubThreshold", ps.bNightSubThreshold);
    SET_VALUE(xPSElement, "iImfilterSingleThreshold", ps.iImfilterSingleThreshold);
    SET_VALUE(xPSElement, "iImdilateThreshold", ps.iImdilateThreshold);
    SET_VALUE(xPSElement, "iWhiteSpotNumMax", ps.iWhiteSpotNumMax);
    SET_VALUE(xPSElement, "iWhiteSpotNumMin", ps.iWhiteSpotNumMin);
    SET_VALUE(xPSElement, "iXTrackContinueThreshold", ps.iXTrackContinueThreshold);
    SET_VALUE(xPSElement, "iXTrackOffsetValue", ps.iXTrackOffsetValue);
    SET_VALUE(xPSElement, "iTrackMaxFrameNum", ps.iTrackMaxFrameNum);
    SET_VALUE(xPSElement, "iLittleRegionValue", ps.iLittleRegionValue);

    //LineSet
    SET_VALUE(xPSElement, "tRectLittleRegion", ps.tRectLittleRegion);
    SET_VALUE(xPSElement, "tRectTreeLittleRegion", ps.tRectTreeLittleRegion);

    //注意：对于数组，要加“*”
    SET_VALUE(xPSElement, "fPersonRange", *ps.fPersonRange);
    SET_VALUE(xPSElement, "tLineCurverRange", *ps.tLineCurverRange);
    SET_VALUE(xPSElement, "tLineStraightFirst", *ps.tLineStraightFirst);
    SET_VALUE(xPSElement, "tLineStraightSecond", *ps.tLineStraightSecond);
    SET_VALUE(xPSElement, "tRectBlackBlock", *ps.tRectBlackBlock);
    SET_VALUE(xPSElement, "tLineBlackLeft", *ps.tLineBlackLeft);
    SET_VALUE(xPSElement, "tLineBlackRight", *ps.tLineBlackRight);
    SET_VALUE(xPSElement, "tNightRange", *ps.tNightRange);

    //int
    SET_VALUE(xPSElement, "iNightRangeVal", ps.iNightRangeVal);*/

    //ParamDistinguish ///////////////////////////////////////////////
    const char* NODE_PARAMDISTINGUISH = "ParamDistinguish";
    TiXmlElement *xPDElement = this->SearchElement(xChannelElement, NODE_PARAMDISTINGUISH);
    if(NULL == xPDElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No 'ParamStruct' info in channel %d", iCh);
        return 0;
    }

    //JudgeSet
    SET_VALUE(xPDElement, "TrackNumMax", pd.TrackNumMax);
    SET_VALUE(xPDElement, "TrackNumMin", pd.TrackNumMin);
    SET_VALUE(xPDElement, "LostNumVal", pd.LostNumVal);
    SET_VALUE(xPDElement, "MigrationDiff", pd.MigrationDiff);
    SET_VALUE(xPDElement, "OriginCurrDis", pd.OriginCurrDis);
    SET_VALUE(xPDElement, "YHeightVal", pd.YHeightVal);
    SET_VALUE(xPDElement, "XContValue", pd.XContValue);
    SET_VALUE(xPDElement, "XMatchValue", pd.XMatchValue);
    SET_VALUE(xPDElement, "YMatchValue", pd.YMatchValue);
    SET_VALUE(xPDElement, "FindObjValue", pd.FindObjValue);
    SET_VALUE(xPDElement, "RiseNumVal", pd.RiseNumVal);
    SET_VALUE(xPDElement, "DownNumVal", pd.DownNumVal);
    SET_VALUE(xPDElement, "FindObjValMax", pd.FindObjValMax);
    SET_VALUE(xPDElement, "FindObjValMin", pd.FindObjValMin);
    SET_VALUE(xPDElement, "WhiteSpotVal", pd.WhiteSpotVal);
    SET_VALUE(xPDElement, "TopPointVal", pd.TopPointVal);

    //JudgeSetFlt
    SET_VALUE(xPDElement, "fFindObjInverse", pd.fFindObjInverse);
    SET_VALUE(xPDElement, "fXMatchInverse", pd.fXMatchInverse);
    SET_VALUE(xPDElement, "fYMatchInverse", pd.fYMatchInverse);
    SET_VALUE(xPDElement, "fXContInverse", pd.fXContInverse);

    return 1;
}

//在父节点下根据名称搜索
TiXmlElement* CCfgParse::SearchElement(TiXmlElement* xParentElem, const char* szName)
{
    TiXmlElement *xElement = xParentElem->FirstChildElement();
    if(NULL == xElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No child element");
        return NULL;
    }

    if (strcmp(xElement->Value(), szName) == 0)
    {
        return xElement;
    }
    else
    {
        while(NULL != (xElement =xElement->NextSiblingElement()) )
        {
            if(strcmp(xElement->Value(), szName) == 0) 
            {
                return xElement;
            }
        }
    }

    LOG_DEBUG(DEBUG_ERR, "Can't find %s", szName);
    return NULL;
}

//根据通道号查找节点
TiXmlElement* CCfgParse::SearchChannel(int iCh)
{
    if (!m_xRootElem)
    {
        LOG_DEBUG(DEBUG_ERR, "Error root element");
        return NULL;
    }

    TiXmlElement *xElement = m_xRootElem->FirstChildElement();
    if(NULL == xElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No channel info: %d", iCh);
        return NULL;
    }

    int tmpCh = -1;
    if(NULL != xElement->Attribute(ATTR_CHANNEL, &tmpCh) && tmpCh == iCh)
    {
        return xElement;
    }

    float xx;
    xElement->Attribute(ATTR_CHANNEL, &xx);

    while(NULL != (xElement =xElement->NextSiblingElement()) )
    {
        if(NULL != xElement->Attribute(ATTR_CHANNEL, &tmpCh) && tmpCh == iCh) 
        {
            //找到指定通道
            return xElement;
        }
    }

    LOG_DEBUG(DEBUG_ERR, "Can't find channel:  %d", iCh);
    return NULL;
}

const char* CCfgParse::GetGolbalParam(const char* szName)
{
    if (!m_xRootElem)
    {
        LOG_DEBUG(DEBUG_ERR, "Error root element");
        return NULL;
    }

    //ParamStruct ///////////////////////////////////////////////
    const char* GOLBAL_PARAMSTRUCT = "golbal";
    TiXmlElement *xGlobalElement = this->SearchElement(m_xRootElem, GOLBAL_PARAMSTRUCT);
    if(NULL == xGlobalElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info ", GOLBAL_PARAMSTRUCT);
        return 0;
    }

    TiXmlElement *xDestElement = this->SearchElement(xGlobalElement, szName);
    if(NULL == xDestElement)
    {
        LOG_DEBUG(DEBUG_ERR, "No '%s' info ", szName);
        return 0;
    }

    return xDestElement->GetText();
}


