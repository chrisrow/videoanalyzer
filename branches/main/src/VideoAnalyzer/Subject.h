
#pragma once

#include "afxmt.h"
#include <vector>
#include <list>

class CGuard
{
public:
    CGuard(CCriticalSection& CSect): m_lock(&CSect)
    { 
        m_lock.Lock(); 
    }

    ~CGuard()                 
    { 
        m_lock.Unlock(); 
    }

private:
    CSingleLock m_lock;
};

// Obersver模式中的Subject基类
// 注意： 类型T必须是一个指针
template<typename T>
class CSubject
{
public:
    virtual ~CSubject() {}

    void addListener(T elem);
    void removeListener(T elem);
    void retrieveListener();

    void clearListener();
    void destroyListener();

protected:
    enum TO_DO{ TO_ADD, TO_REM };
    struct TTodo
    {
        TO_DO flag;
        T elem;

        TTodo(TO_DO t, T e): flag(t), elem(e) {}
    };

    std::vector<T> m_elem;  //
    std::vector<TTodo> m_todoList;

    CCriticalSection m_CSect;
};

template<typename T>
void CSubject<T>::addListener(T elem)
{
    if (elem != NULL)
    {
        CGuard guard(m_CSect);
        m_todoList.push_back(TTodo(TO_ADD, elem));
    }
}

template<typename T>
void CSubject<T>::removeListener(T elem)
{
    if (elem != NULL)
    {
        CGuard guard(m_CSect);
        m_todoList.push_back(TTodo(TO_REM, elem));
    }
}

template<typename T>
void CSubject<T>::retrieveListener()
{
    CGuard guard(m_CSect);

    std::vector<TTodo>::iterator itTodo = this->m_todoList.begin();
    for (; itTodo != m_todoList.end(); itTodo++)
    {
        std::vector<T>::iterator it = m_elem.begin();
        for (; it != m_elem.end(); it++)
        {
            if (TO_ADD == itTodo->flag)
            {
                if (itTodo->elem == *it)
                {
                    break;
                }
                else if (NULL == *it)
                {
                    *it = itTodo->elem;
                    break;
                }
            } 
            else // TO_REM == itTodo->flag
            {
                if (itTodo->elem == *it)
                {
                    *it = NULL;
                    break;
                }
           }
        }
        if (it == m_elem.end() && TO_ADD == itTodo->flag)
        {
            m_elem.push_back(itTodo->elem);
        }

    }

    m_todoList.clear();
}

template<typename T>
void CSubject<T>::clearListener()
{
//     CGuard guard(m_CSect);

    m_elem.clear();
}

template<typename T>
void CSubject<T>::destroyListener()
{
//     CGuard guard(m_CSect);

    std::vector<T>::iterator it = m_elem.begin();
    for (; it != m_elem.end(); it++)
    {
        delete *it;
    }
    m_elem.clear();
}

#define ADD_LISTENER(type, subject, listener) \
{\
    CSubject<##type>* ptr = dynamic_cast<CSubject<##type>*>(subject);\
    if (ptr)\
    {\
        ptr->addListener(listener);\
    }\
}


#define FOR_EACH(type, func, ...) \
{\
    CSubject<##type>::retrieveListener();\
    std::vector<##type>::iterator it = CSubject<##type>::m_elem.begin();\
    for (; it != CSubject<##type>::m_elem.end(); it++)\
    {\
        if (*it)\
        {\
            (*it)->##func(__VA_ARGS__);\
        }\
    }\
}
