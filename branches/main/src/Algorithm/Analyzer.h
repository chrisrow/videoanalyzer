#pragma once

#include "VideoAnalyzer/FrameReceiver.h"
#include "VideoAnalyzer/Alerter.h"
#include "VideoAnalyzer/Subject.h"

enum ALERTTYPE//alert type
{
    AT_NONE = 0,    // ��
    AT_WARNING,     // Ԥ��
    AT_ALERT        // ����
};

//ͼ����������Խ��յ���ÿһ֡���з�����������Ҫ��ʱ�򽫱���ͼ���͸�ÿ��Listener
class CAnalyzer : public IFrameReceiver
{
public:
    virtual ~CAnalyzer() {}

    virtual const IplImage* analysis(const IplImage *pFrame) = 0;
    virtual ALERTTYPE haveAlert() = 0;

    inline void addReceiver(IFrameReceiver* pDisplayer);
    inline void removeReceiver(IFrameReceiver* pDisplayer);
    inline void addAlerter(IAlerter* pAlerter);

    inline void doReceiver(const IplImage *pFrame);
    inline void doAlerter(const IplImage *pFrame);

private:
    CSubject<IAlerter*> m_pAlerters;
    CSubject<IFrameReceiver*> m_pDisplayers;
};

void CAnalyzer::addAlerter(IAlerter* pAlerter)
{
    m_pAlerters.addListener(pAlerter);
}

void CAnalyzer::addReceiver(IFrameReceiver* pDisplayer)
{
    m_pDisplayers.addListener(pDisplayer);
}

void CAnalyzer::removeReceiver(IFrameReceiver* pDisplayer)
{
    m_pDisplayers.removeListener(pDisplayer);
}

void CAnalyzer::doReceiver(const IplImage *pFrame)
{
    m_pDisplayers.retrieveListener();
    std::vector<IFrameReceiver*>::iterator it = m_pDisplayers.m_elem.begin();
    for (; it != m_pDisplayers.m_elem.end(); it++)
    {
        if (*it)
        {
            (*it)->updateFrame(pFrame);
        }
    }
}

void CAnalyzer::doAlerter(const IplImage *pFrame)
{
    m_pAlerters.retrieveListener();
    std::vector<IAlerter*>::iterator it = m_pAlerters.m_elem.begin();
    for (; it != m_pAlerters.m_elem.end(); it++)
    {
        if (*it)
        {
            (*it)->alert(pFrame);
        }
    }
}
