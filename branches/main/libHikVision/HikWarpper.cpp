#include "stdafx.h"
#include "HikWarpper.h"

static CHikMgr* g_HikMgr = NULL;

ICameraMgr* getCameraMgr()
{
    if (!g_HikMgr)
    {
        g_HikMgr = new CHikMgr;
    }

    return g_HikMgr;
}

void destoryCameraMgr()
{
    delete g_HikMgr;
    g_HikMgr = NULL;
}

CHikWarpper::CHikWarpper()
: m_iIndex(-1)
, m_pFrame(NULL)
, m_iWidth(CIF_WIDTH)
, m_iHeight(CIF_HEIGHT)
{
}

CHikWarpper::~CHikWarpper()
{
}

bool CHikWarpper::open(int iIndex)
{
    NET_DVR_Init();
    return true;
}

void CHikWarpper::close()
{
    NET_DVR_Cleanup();
}

bool CHikWarpper::setWidth(int iWidth)
{
    return true;
}

bool CHikWarpper::setHeight(int iHeight)
{
    return true;
}

int  CHikWarpper::getWidth()
{
    return 352;
}
int  CHikWarpper::getHeight()
{
    return 288;
}

/////////////////////////////////////////////////

CHikMgr::CHikMgr()
{
    memset(m_szName, 0, CAM_NAME_LEN);
}

CHikMgr::~CHikMgr()
{
}

ICamera* CHikMgr::createCamera()
{
    return new CHikWarpper;
}

void CHikMgr::destroyCamera(ICamera** pCamera)
{
    delete *pCamera;
    *pCamera = NULL;
}

int CHikMgr::getCount()
{
    return 1;
}

const char* CHikMgr::getName(int iIndex)
{
    return "test";

}
