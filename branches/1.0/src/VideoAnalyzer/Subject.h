
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
    std::vector<T> m_elem;  //
    std::list<T>   m_tmpAddElem;
    std::list<T>   m_tmpRemElem;

    CCriticalSection m_CSect;
};

template<typename T>
void CSubject<T>::addListener(T elem)
{
    if (elem != NULL)
    {
        CGuard guard(m_CSect);
        m_tmpAddElem.push_back(elem);
    }
}

template<typename T>
void CSubject<T>::removeListener(T elem)
{
    if (elem != NULL)
    {
        CGuard guard(m_CSect);
        m_tmpRemElem.push_back(elem);
    }
}

template<typename T>
void CSubject<T>::retrieveListener()
{
    CGuard guard(m_CSect);

    std::list<T>::iterator itToAdd = m_tmpAddElem.begin();
    for (; itToAdd != m_tmpAddElem.end(); itToAdd++)
    {
        std::vector<T>::iterator it = m_elem.begin();
        for (; it != m_elem.end(); it++)
        {
            if (*itToAdd == *it)
            {
                break;
            }
            else if (NULL == *it)
            {
                *it = *itToAdd;
                break;
            }
        }
        if (it == m_elem.end())
        {
            m_elem.push_back(*itToAdd);
        }
    }
    m_tmpAddElem.clear();

    std::list<T>::iterator itToRem = m_tmpRemElem.begin();
    for (; itToRem != m_tmpRemElem.end(); itToRem++)
    {
        std::vector<T>::iterator it = m_elem.begin();
        for (; it != m_elem.end(); it++)
        {
            if (*itToRem == *it)
            {
                *it = NULL;
                break;
            }
        }
    }
    m_tmpRemElem.clear();
}

// template<typename T>
// void CSubject<T>::addListenerT elem)
// {
//     std::vector<T>::iterator it = m_elem.begin();
//     for (; it != m_elem.end(); it++)
//     {
//         if (elem == *it)
//         {
//             return;
//         }
// 
//         if (NULL == *it)
//         {
//             *it = elem;
//             return;
//         }
//     }
//     m_elem.push_back(elem);
// }
// 
// template<typename T>
// void CSubject<T>::removeListener(T elem)
// {
//     std::vector<T>::iterator it = m_elem.begin();
//     for (; it != m_elem.end(); it++)
//     {
//         if (elem == *it)
//         {
//             *it = NULL;
//         }
//     }
// }

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

#define FOR_EACH(type, func, ...) \
{\
    retrieveListener();\
    std::vector<##type>::iterator it = m_elem.begin();\
    for (; it != m_elem.end(); it++)\
    {\
        if (*it)\
        {\
            (*it)->##func(__VA_ARGS__);\
        }\
    }\
}
