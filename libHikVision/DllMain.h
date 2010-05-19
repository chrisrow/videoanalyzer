#ifndef _DLLMANAGER_H_
#define _DLLMANAGER_H_

#pragma once

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) ||defined(_WIN32_WCE)
  #define DLL_EXPORT _declspec(dllexport)
  #define DLL_IMPORT _declspec(dllimport)
#else
  #define DLL_EXPORT 
  #define DLL_IMPORT 
#endif


#include "../src/VideoAnalyzer/VideoGraberInterface.h"

extern "C" 
{
    DLL_EXPORT ICameraMgr* getCameraMgr();
    DLL_EXPORT void destoryCameraMgr();
}

#endif

