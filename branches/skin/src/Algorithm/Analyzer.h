#pragma once

#include "VideoAnalyzer/FrameReceiver.h"
#include "VideoAnalyzer/Alerter.h"
#include "VideoAnalyzer/Subject.h"

enum ALERTTYPE//alert type
{
    AT_NONE = 0,    // 无
    AT_WARNING,     // 预警
    AT_ALERT        // 报警
};

//图像分析器。对接收到的每一帧进行分析，并在需要的时候将报警图像发送给每个Listener
class CAnalyzer : public IFrameReceiver, public CSubject<IAlerter*>, public CSubject<IFrameReceiver*>
{
public:
    virtual ~CAnalyzer() {}

    virtual const IplImage* analysis(const IplImage *pFrame) = 0;
    virtual ALERTTYPE haveAlert() = 0;
};

