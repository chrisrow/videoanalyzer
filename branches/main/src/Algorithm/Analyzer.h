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
class CAnalyzer : public IFrameReceiver, public CSubject<IAlerter*>, public CSubject<IFrameReceiver*>
{
public:
    virtual ~CAnalyzer() {}

    virtual const IplImage* analysis(const IplImage *pFrame) = 0;
    virtual ALERTTYPE haveAlert() = 0;
};

