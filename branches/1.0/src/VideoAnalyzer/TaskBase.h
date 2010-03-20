
#pragma once

//�첽����Ļ��ࡣ���ڲ�ά����һ���������̡߳�
//����ֻ��Ҫ�����������д��doRun�������ɣ������ߵ���run�����󣬸ö���ͻ������첽����
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

