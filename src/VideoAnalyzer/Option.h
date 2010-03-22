#pragma once

#ifndef _COption_H_
#define _COption_H_

#include "getopt.h"

//!@brief ��ȡ����·�����ļ�����GNU��POSIX������ͬ���ܵĺ���:char *basename(char *path)
//!@param path �������ļ���
//!@return ����·�����ļ���
inline const char* Basename(const char* path)
{
    if(NULL == path)
    {
        return NULL;
    }
    for(int i= strlen(path)-1; i >= 0; i--)
    {
        if(*(path+i) == '\\' || *(path+i) == '/')
        {
            return path+i+1;
        }
    }
    return path;
}

//!@brief ��ȡִ�в���
//!@brief Usage: utmain [-a] [-h] [-f FilterString] [-r RepeatTimes]
//!@brief 
//!@brief    ���������
//!@brief    - -a �������test��ִ�н����Ĭ��ֻ���ʧ�ܵ�test�Ľ��
//!@brief    - -h ��ʾ����
//!@brief    - -f FilterString ���ù�������ִ��ָ����test
//!@brief    - -r RepeatTimes  ָ��ÿ��test��ִ�д���
//!@note �ӻ����������������л�ȡ������������߶�������ͬһ�������������������е�Ϊ׼��
class COption
{
public:
    static COption& Instance();
    
    //!@brief ��ʼ������ѡ����������в���
    //!@param argc �����в�������
    //!@param argv �����в���
    //!@return  ����ִ���Ƿ�ɹ�
    //!@note �ȶ�����֧�ֵ�ѡ����г�ʼ����Ȼ����������С�
    bool init(int argc, char** argv);
    
    //!@brief ��ʾ������Ϣ
    void usage();
    
    const char* getFileName();    //�򿪵���Ƶ�ļ�����
    int getCameraID();            //�����ID
    const char* getURL();         //��ý����ַ
    const char* getRecordName();  //¼���ļ���
    const char* getSimple();      //�򵥽���ģʽ�����ڴ�С�ʹ���λ��
    bool isDebug();               //����ģʽ
    bool isLogOn();               //��¼��־
    bool isPreview();             //��ʾԤ��

protected:
    COption();
    
private:
    //!@brief �ӻ���������ʼ������
    //!@return  ����ִ���Ƿ�ɹ�
    bool initOption();
    bool setOption(char key, const char* param1 = NULL);
    const char* getOption(char key);
    
    char* makeOptString(char* str);
    int getIndex(char key);
    char fromIndex(int index);
    
private:
    static COption m_instance;
    bool m_bInit;
    const char* m_myName;
    int m_optNum;
};


#endif
