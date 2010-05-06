#ifndef _ICAMERAFACTORY_H_
#define _ICAMERAFACTORY_H_

#include "DllManager.h"

#ifdef _DEBUG
#pragma comment(lib,"cv200d.lib")
#pragma comment(lib,"cvaux200d.lib")
#pragma comment(lib,"cxcore200d.lib")
#pragma comment(lib,"cxts200d.lib")
#pragma comment(lib,"highgui200d.lib")
#pragma comment(lib,"ml200d.lib")
#else
#pragma comment(lib,"cv200.lib")
#pragma comment(lib,"cvaux200.lib")
#pragma comment(lib,"cxcore200.lib")
#pragma comment(lib,"cxts200.lib")
#pragma comment(lib,"highgui200.lib")
#pragma comment(lib,"ml200.lib")
#endif

#include "cv.h"
#include "../src/VideoAnalyzer/VideoGraberInterface.h"

extern "C" 
{
DLL_EXPORT ICameraMgr* getCameraMgr();
DLL_EXPORT void destoryCameraMgr();
}

#endif
