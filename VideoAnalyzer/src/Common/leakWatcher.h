/*! 
*************************************************************************************
* \file 
*    leakWatcher.h
* \brief
*    Memory Leak Watcher 
* \date
*    2009_04_10
* \author
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*************************************************************************************
*/

#if !defined  (_LEAKWATCHER_H__6518ED0F_E1C0_4188_9908_B44C30FBA534)
#define _LEAKWATCHER_H__6518ED0F_E1C0_4188_9908_B44C30FBA534

#include <crtdbg.h>

#ifdef _DEBUG

#ifndef DEBUG_NEW
#define DEBUG_NEW new (_NORMAL_BLOCK, THIS_FILE, __LINE__)
#endif

#define MALLOC_DBG(x) _malloc_dbg(x, 1, THIS_FILE, __LINE__);
#define malloc(x) MALLOC_DBG(x)

#endif // _DEBUG

#endif // #include guard

