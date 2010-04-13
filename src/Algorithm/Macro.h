
#pragma once

#include "cv.h"
#include "highgui.h"
#include <set>
#include <string>

//调试用的相关宏

#define INIT_IMAGE() \
    if (g_debug)\
    {\
        CTestPreview::init();\
    }

//显示IplImage图像
#define SHOW_IMAGE(name, image) \
    if (g_debug)\
    {\
        CTestPreview::show(name, image);\
    }\
    else\
    {\
        CTestPreview::clear();\
    }

//显示二值图像
#define SHOW_BIN_IMAGE(name, width, height, data) \
    if (g_debug)\
    {\
        CTestPreview::showBin(name, width, height, data);\
    }\
    else\
    {\
        CTestPreview::clear();\
    }

//删除所有的图像窗口
#define DEL_ALL_IMAGE() \
    if (g_debug)\
    {\
        CTestPreview::clear();\
    }

///////////////////////////////////////////////////

class CTestPreview
{
public:
	static inline void init()
	{
		std::set<std::string>::iterator it = m_wnds.begin();
		for (; it != m_wnds.end(); it++)
		{
			cvNamedWindow(it->c_str());
		}
	}

    static inline void show(const char* szWndName, const IplImage* pImage)
    {
		m_wnds.insert(szWndName);
        cvShowImage(szWndName, pImage);
    }

    static inline void showBin(const char* szWndName, int iWidth, int iHeight, char* pBuf)
    {
        if (NULL == m_pImage || m_pImage->width != iWidth || m_pImage->height != iHeight)
        {
            static int depth = 8;
            static int channel = 1;
            cvReleaseImage(&m_pImage);
            m_pImage = cvCreateImage( cvSize(iWidth, iHeight), depth, channel);
        }
        memcpy(m_pImage->imageData, pBuf, iWidth*iHeight);
        cvThreshold(m_pImage, m_pImage, 0.5, 255, CV_THRESH_BINARY);

		m_wnds.insert(szWndName);
        cvShowImage(szWndName, m_pImage);
    }

    static inline void clear()
    {
        cvDestroyAllWindows();
    }

private:
    static IplImage*  m_pImage;
	static std::set<std::string> m_wnds;
};


