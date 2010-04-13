
#pragma once

//异步任务的基类。其内部维护了一个独立的线程。
//子类只需要将具体的任务写到doRun方法即可，调用者调用run方法后，该对象就会启动异步任务。
class CTaskBase
{
    friend inline UINT TaskThreadProc(LPVOID lParam);

public:
    CTaskBase(): m_pThread(NULL) {}
    virtual ~CTaskBase() {}

    inline void run();

protected:
    virtual void doRun() = 0;

protected:
    CWinThread* m_pThread;
};

inline UINT TaskThreadProc(LPVOID lParam)
{
    CTaskBase* base = (CTaskBase*)lParam;
    base->doRun();
    return 0;
}

void CTaskBase::run()
{
    m_pThread = AfxBeginThread(TaskThreadProc, this);
}

