
#pragma once

#include <vector>

// Obersver模式中的Subject基类
// 注意： 类型T必须是一个指针
template<typename T>
class CSubject
{
public:
    virtual ~CSubject() {}

    void addListener(T elem);
    void removeListener(T elem);
    void clearListener();
    void destroyListener();

protected:
    std::vector<T> m_elem;  //
};

template<typename T>
void CSubject<T>::addListener(T elem)
{
    std::vector<T>::iterator it = m_elem.begin();
    for (; it != m_elem.end(); it++)
    {
        if (elem == *it)
        {
            return;
        }

        if (NULL == *it)
        {
            *it = elem;
            return;
        }
    }
    m_elem.push_back(elem);
}

template<typename T>
void CSubject<T>::removeListener(T elem)
{
    std::vector<T>::iterator it = m_elem.begin();

    for (; it != m_elem.end(); it++)
    {
        if (elem == *it)
        {
            *it = NULL;
        }
    }
}

template<typename T>
void CSubject<T>::clearListener()
{
    m_elem.clear();
}

template<typename T>
void CSubject<T>::destroyListener()
{
    std::vector<T>::iterator it = m_elem.begin();

    for (; it != m_elem.end(); it++)
    {
        delete *it;
    }
    m_elem.clear();
}

#define FOR_EACH(type, func, ...) \
    std::vector<##type>::iterator it = m_elem.begin();\
    for (; it != m_elem.end(); it++)\
    {\
        if (*it)\
        {\
            (*it)->##func(__VA_ARGS__);\
        }\
    }
