
#include "stdafx.h"
#include "ConfigFile.h"
#include "Common.h"
#include <string.h>

TCommonParam g_commParam;

TCommonParam::TCommonParam()
{
    reset();
}

void TCommonParam::reset()
{
    memset(szLocalAddr, 0, 4);
    memset(szUDPServerIP, 0, 4);
    iUDPServerPort = 0;
    memset(szImagePath, 0, MAX_PATH);
    iHeartBeat = 180;
}


const char* NODE_LOCALADDR = "LocalAddr";
const char* NODE_UPDSERVER = "UDPServer";
const char* NODE_HEARTBEAT = "HeartBeat";
const char* NODE_IMAGEPATH = "ImagePath";

const char* loadCommonParam(const char* szFileName)
{
    CCfgParse parse;
    if (!parse.Load(szFileName))
    {
        return _T("���������ļ�ʧ��");
    }

    //����IP
    {
        int tmp[4] = {0};
        const char* ip = NULL;
        if ( NULL != (ip = parse.GetGolbalParam(NODE_LOCALADDR)) )
        {
            sscanf(ip, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
            g_commParam.szLocalAddr[0] = (unsigned char)tmp[0];
            g_commParam.szLocalAddr[1] = (unsigned char)tmp[1];
            g_commParam.szLocalAddr[2] = (unsigned char)tmp[2];
            g_commParam.szLocalAddr[3] = (unsigned char)tmp[3];    
        }
        else
        {
            return _T("��ȡ����IPʧ��");
        }
    }

    //��������IP�Ͷ˿ں�
    {
        int tmp[4] = {0};
        const char* ip = NULL;
        if ( NULL != (ip = parse.GetGolbalParam(NODE_UPDSERVER)) )
        {
            sscanf(ip, "%d.%d.%d.%d:%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &g_commParam.iUDPServerPort);
            g_commParam.szUDPServerIP[0] = (unsigned char)tmp[0];
            g_commParam.szUDPServerIP[1] = (unsigned char)tmp[1];
            g_commParam.szUDPServerIP[2] = (unsigned char)tmp[2];
            g_commParam.szUDPServerIP[3] = (unsigned char)tmp[3];    
        }
        else
        {
            return _T("��ȡ��������IP�Ͷ˿ں�ʧ��");
        }
    }

    //����������Ϣ�ļ��ʱ�䣬��
    {
        const char* pInterval = NULL;
        if ( NULL != (pInterval = parse.GetGolbalParam(NODE_HEARTBEAT)) )
        {
            g_commParam.iHeartBeat = atoi(pInterval);
        }
        if(g_commParam.iHeartBeat <= 0)
        {
            g_commParam.iHeartBeat = 180;
        }
    }

    //����ͼƬ����·��
    {
        const char* path = parse.GetGolbalParam(NODE_IMAGEPATH);
        if(path)
        {
            _snprintf(g_commParam.szImagePath, MAX_PATH, "%s", path);
        }
        else
        {
            _snprintf(g_commParam.szImagePath, MAX_PATH, "d:/");
        }

    }

    return NULL;
}
