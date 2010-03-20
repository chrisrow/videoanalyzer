/*! 
*************************************************************************************
* \file 
*    dlist.h
* \brief
*    data structer double list
* \date
*    2009_06_15
* \author
*    - Cong LUO                       <>
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*************************************************************************************
*/




#if !defined  (_DLIST_H__F56001F6_6EA7_444b_92CC_56D0A72B7FCF)
#define _DLIST_H__F56001F6_6EA7_444b_92CC_56D0A72B7FCF


#include <assert.h>
#include <crtdbg.h>

//===== Use MemLeak Detector. ===== Note: Put it after last #include
#include "leakWatcher.h"
#ifdef _DEBUG
#   define new DEBUG_NEW
#   define THIS_FILE __FILE__
#endif

#ifdef _DEBUG
#ifndef ASSERT
#define ASSERT  assert
#endif
#else   // not _DEBUG
#ifndef ASSERT
#define ASSERT
#endif
#endif  // _DEBUG




//CPointerDNode is used to store data in the list.
template< typename T >
class CDataDNode
{
public:
  T data;
  CDataDNode<T>* prior;
  CDataDNode<T>* next;
  CDataDNode() : data(T()), prior(NULL), next(NULL) {}
  CDataDNode(T const& initdata) : data(initdata), prior(NULL), next(NULL) {}
  inline void release() {} //do nothing
};

//CPointerDNode is used to store pointer in the list and will free these pointers finally.
template< typename T >
class CPointerDNode // todo: inherit from public CDataDNode<T>
{
public:
  T data;
  CPointerDNode<T>* prior;
  CPointerDNode<T>* next;
  CPointerDNode() : data(T()), prior(NULL), next(NULL) {}
  CPointerDNode(T const& initdata) : data(initdata), prior(NULL), next(NULL) {}
  inline void release() { if(data) delete data; data = NULL; } // the only difference between these two types of Nodes
};

// Double list
// Example: 
//    CDList< int, CDataDNode >    :  "Destroy function" and "Remove function" are the same.
//    CDList< int*, CPointerDNode >:  "Destroy function" will release (int*) memory, "Remove function" will not release (int*) memory.  
//    CDList< int*, CDataDNode >   :  is not recommended, as DestroyAll() can not release int*; 
template< typename T, template< typename U > class CDNode >
class CDList
{

protected:
  int m_nCount;
  CDNode<T>* m_pNodeHead;
  CDNode<T>* m_pNodeTail;

public:
  CDList();
  CDList( T const& initdata );
  ~CDList(); // use DestroyAll()

public:
  int              IsEmpty() const;
  int              GetCount() const;
  int              InsertBefore( int const pos, T const data );
  int              InsertAfter( int const pos, T const data );
  int              AddHead( T const data );
  int              AddTail( T const data );
  T                RemoveAtPosandReturnIt( int const pos );
  void             DestroyAtPos( int const pos );
  T                RemoveAtAddrandReturnIt( CDNode<T>* pNode );
  void             DestroyAtAddr( CDNode<T>* pNode ); // DestroyAtAddr(FindAddrForData) is faster than DestroyAtPos(FindPosForData)
  void             DestroyHead();
  T                RemoveHeadandReturnIt(); // Note: will return NULL, if CPointerDNode is used
  void             DestroyTail();
  T                RemoveTailandReturnIt(); // Note: will return NULL, if CPointerDNode is used
  void             DestroyAll(); // equal to remove all
  T&               GetTail();
  T                GetTail() const;
  T&               GetHead();
  T                GetHead() const;
  T&               GetAt( int const pos );
  T                GetAt( int const pos ) const;
  void             SetAt( int const pos, T data );
  int              FindPosForData( T const data ) const;
  CDNode<T>*       FindAddrAt( int const pos ) const;
  CDNode<T>*       FindAddrForData( T const data ) const;    
  T&               GetPrev( int& pos );	//new function
  T&               GetNext( int& pos );	//new function
};




template< typename T, template< typename U > class CDNode >
inline CDList<T, CDNode>::CDList() : m_nCount(0), m_pNodeHead(NULL), m_pNodeTail(NULL)
{
}

template< typename T, template< typename U > class CDNode >
inline 
CDList<T, CDNode>::CDList( T const& initdata )
: m_nCount(0), m_pNodeHead(NULL), m_pNodeTail(NULL)
{
  AddHead(initdata);
}

template< typename T, template< typename U > class CDNode >
inline 
CDList<T, CDNode>::~CDList()
{
  DestroyAll();
}

template< typename T, template< typename U > class CDNode >
inline T& 
CDList<T, CDNode>::GetNext( int& pos )
{
  ASSERT( 0 != m_nCount );
  ASSERT( 1 <= pos && pos <= m_nCount );

  int i;
  CDNode<T>* pTmpNode = m_pNodeHead;

  pTmpNode = FindAddrAt( pos );

  ++pos;

  return pTmpNode->data;
}

template< typename T, template< typename U > class CDNode >
inline T& 
CDList<T, CDNode>::GetPrev( int& pos )  //note the return of pos is the new address
{
  ASSERT( 0 != m_nCount );
  ASSERT( 1 <= pos && pos <= m_nCount );

  int i;
  CDNode<T>* pTmpNode = m_pNodeHead;

  pTmpNode = FindAddrAt( pos );

  --pos;

  return pTmpNode->data;
}

template< typename T, template< typename U > class CDNode >
inline int 
CDList<T, CDNode>::InsertBefore( int const pos, T const data )
{
  int nRetPos;
  CDNode<T>* pTmpNode;
  CDNode<T>* pNewNode;

  pNewNode = new CDNode<T>;
  if( NULL == pNewNode )
  {
    nRetPos = 0;
    goto Exit0;
  }

  pNewNode->data = data;

  // if the list is empty, replace the head node with the new node.
  if( NULL == m_pNodeHead )
  {
    pNewNode->prior = NULL;
    pNewNode->next = NULL;
    m_pNodeHead = pNewNode;
    m_pNodeTail = pNewNode;
    nRetPos = 1;
    goto Exit1;
  }

  // is pos range valid?
  ASSERT( 1 <= pos && pos <= m_nCount );

  // insert before head node?
  if( 1 == pos )
  {
    pNewNode->prior = NULL;
    pNewNode->next = m_pNodeHead;
    m_pNodeHead->prior = pNewNode;
    m_pNodeHead = pNewNode;
    nRetPos = 1;
    goto Exit1;
  }

  // if the list is not empty and is not inserted before head node,
  // seek to the pos of the list and insert the new node before it.
  pTmpNode = FindAddrAt( pos );

  pNewNode->next  = pTmpNode;
  pNewNode->prior = pTmpNode->prior;

  pTmpNode->prior->next = pNewNode;
  pTmpNode->prior = pNewNode;

  // if tail node, must update m_pNodeTail
  if( NULL == pNewNode->next )
  {
    m_pNodeTail = pNewNode;
  }

  nRetPos = pos;

Exit1:
  ++m_nCount;
Exit0:
  return nRetPos;
}

template< typename T, template< typename U > class CDNode >
inline int 
CDList<T, CDNode>::InsertAfter( int const pos, T const data )
{
  int nRetPos;
  CDNode<T>* pNewNode;
  CDNode<T>* pTmpNode;

  pNewNode = new CDNode<T>;
  if( NULL == pNewNode )
  {
    nRetPos = 0;
    goto Exit0;
  }

  pNewNode->data = data;

  // if the list is empty, replace the head node with the new node.
  if( NULL == m_pNodeHead )
  {
    pNewNode->prior = NULL;
    pNewNode->next = NULL;
    m_pNodeHead = pNewNode;
    m_pNodeTail = pNewNode;
    nRetPos = 1;
    goto Exit1;
  }

  // is pos range valid?
  ASSERT( 1 <= pos && pos <= m_nCount );

  // if the list is not empty,
  // seek to the pos of the list and insert the new node after it.
  pTmpNode = FindAddrAt( pos );

  pNewNode->next = pTmpNode->next;
  pNewNode->prior = pTmpNode;

  // if NewNode's position is m_pNodeTail, update m_pNodeTail
  if( pTmpNode->next == m_pNodeTail )
  {
    m_pNodeTail->prior = pNewNode;
  }

  pTmpNode->next = pNewNode;

  // if tail node, must update m_pNodeTail
  if( NULL == pNewNode->next )
  {
    m_pNodeTail = pNewNode;
  }

  nRetPos = pos + 1;

Exit1:
  ++m_nCount;
Exit0:
  return nRetPos;
}

template< typename T, template< typename U > class CDNode >
inline T& 
CDList<T, CDNode>::GetAt( int const pos )
{
  ASSERT( 1 <= pos && pos <= m_nCount );

  CDNode<T>* pTmpNode = FindAddrAt( pos );

  return pTmpNode->data;
}

template< typename T, template< typename U > class CDNode >
inline T 
CDList<T, CDNode>::GetAt( int const pos ) const
{
  ASSERT( 1 <= pos && pos <= m_nCount );

  CDNode<T>* pTmpNode = FindAddrAt( pos );

  return pTmpNode->data;
}

template< typename T, template< typename U > class CDNode >
inline int 
CDList<T, CDNode>::AddHead( T const data )
{
  return InsertBefore(1, data);
}

template< typename T, template< typename U > class CDNode >
inline int 
CDList<T, CDNode>::AddTail( T const data )
{
  return InsertAfter(GetCount(), data);
}

template< typename T, template< typename U > class CDNode >
inline int 
CDList<T, CDNode>::IsEmpty() const
{
  return 0 == m_nCount;
}

template< typename T, template< typename U > class CDNode >
inline int 
CDList<T, CDNode>::GetCount() const
{
  return m_nCount;
}

template< typename T, template< typename U > class CDNode >
inline T& 
CDList<T, CDNode>::GetTail()
{
  ASSERT( 0 != m_nCount );
  return m_pNodeTail->data;
}

template< typename T, template< typename U > class CDNode >
inline T 
CDList<T, CDNode>::GetTail() const
{
  ASSERT( 0 != m_nCount );
  return m_pNodeTail->data;
}

template< typename T, template< typename U > class CDNode >
inline T& 
CDList<T, CDNode>::GetHead()
{
  ASSERT( 0 != m_nCount );
  return m_pNodeHead->data;
}

template< typename T, template< typename U > class CDNode >
inline T 
CDList<T, CDNode>::GetHead() const
{
  ASSERT( 0 != m_nCount );
  return m_pNodeHead->data;
}

template< typename T, template< typename U > class CDNode >
inline T 
CDList<T, CDNode>::RemoveAtPosandReturnIt( int const pos )
{
  //if( m_nCount < pos || pos < 1 )
  //  return -1;
  ASSERT( 1 <= pos && pos <= m_nCount );

  int i;
  CDNode<T>* pTmpNode = NULL; //m_pNodeHead;

  // head node?
  if( 1 == pos )
  {
    pTmpNode    = m_pNodeHead;
    m_pNodeHead = m_pNodeHead->next;
    if( m_pNodeHead ) m_pNodeHead->prior = NULL;
    goto Exit1;
  }
  else if( m_nCount == pos )
  {
    pTmpNode    = m_pNodeTail;
    m_pNodeTail = m_pNodeTail->prior;
    if( m_pNodeTail ) m_pNodeTail->next = NULL;
    goto Exit1;
  }
  else
  {
    pTmpNode = FindAddrAt( pos );

    pTmpNode->prior->next = pTmpNode->next;

    // 06-22-2009, 15:34, YE Tianxiao <ytxjonathan@gmail.com>
    if( pTmpNode->next )
      pTmpNode->next->prior = pTmpNode->prior;
  }

Exit1:
  T returnValue = pTmpNode->data;
  delete pTmpNode;
  //pTmpNode->~CDNode<T>();
  --m_nCount;
  if( 0 == m_nCount ) 
  {
    m_pNodeHead = m_pNodeTail = NULL;
  }

  return returnValue;
}

template< typename T, template< typename U > class CDNode >
inline void 
CDList<T, CDNode>::DestroyAtPos( int const pos )
{
  //if( m_nCount < pos || pos < 1 )
  //  return -1;
  ASSERT( 1 <= pos && pos <= m_nCount );

  int i;
  CDNode<T>* pTmpNode = NULL; //m_pNodeHead;

  // head node?
  if( 1 == pos )
  {
    pTmpNode    = m_pNodeHead;
    m_pNodeHead = m_pNodeHead->next;
    if( m_pNodeHead ) m_pNodeHead->prior = NULL;
    goto Exit1;
  }
  else if( m_nCount == pos )
  {
    pTmpNode    = m_pNodeTail;
    m_pNodeTail = m_pNodeTail->prior;
    if( m_pNodeTail ) m_pNodeTail->next = NULL;
    goto Exit1;
  }
  else
  {
    pTmpNode = FindAddrAt( pos );

    pTmpNode->prior->next = pTmpNode->next;

    // 06-22-2009, 15:34, YE Tianxiao <ytxjonathan@gmail.com>
    if( pTmpNode->next )
      pTmpNode->next->prior = pTmpNode->prior;
  }

Exit1:
  pTmpNode->release();
  delete pTmpNode;
  //pTmpNode->~CDNode<T>();
  --m_nCount;
  if( 0 == m_nCount ) 
  {
    m_pNodeHead = m_pNodeTail = NULL;
  }

  //return 0;
}

template< typename T, template< typename U > class CDNode >
inline T 
CDList<T, CDNode>::RemoveAtAddrandReturnIt( CDNode<T>* pNode )
{
  //if( m_nCount < pos || pos < 1 )
  //  return -1;
  ASSERT( pNode );

  CDNode<T>* pTmpNode = NULL; //m_pNodeHead;

  if( pNode == m_pNodeHead )
  {// head node
    m_pNodeHead = m_pNodeHead->next;
    if( m_pNodeHead ) m_pNodeHead->prior = NULL;
    goto Exit1;
  }
  else if( pNode == m_pNodeTail )
  {// tail node
    m_pNodeTail = m_pNodeTail->prior;
    if( m_pNodeTail ) m_pNodeTail->next = NULL;
    goto Exit1;
  }
  else
  {
    pNode->prior->next = pNode->next;

    // 06-22-2009, 15:34, YE Tianxiao <ytxjonathan@gmail.com>
    if( pNode->next )
      pNode->next->prior = pNode->prior;
  }

Exit1:
  T returnValue = pNode->data;
  delete pNode;
  --m_nCount;
  if( 0 == m_nCount ) 
  {
    m_pNodeHead = m_pNodeTail = NULL;
  }

  return returnValue;
  //return 0;
}

template< typename T, template< typename U > class CDNode >
inline void 
CDList<T, CDNode>::DestroyAtAddr( CDNode<T>* pNode )
{
  //if( m_nCount < pos || pos < 1 )
  //  return -1;
  ASSERT( pNode );

  CDNode<T>* pTmpNode = NULL; //m_pNodeHead;
  
  if( pNode == m_pNodeHead )
  {// head node
    m_pNodeHead = m_pNodeHead->next;
    if( m_pNodeHead ) m_pNodeHead->prior = NULL;
    goto Exit1;
  }
  else if( pNode == m_pNodeTail )
  {// tail node
    m_pNodeTail = m_pNodeTail->prior;
    if( m_pNodeTail ) m_pNodeTail->next = NULL;
    goto Exit1;
  }
  else
  {
    pNode->prior->next = pNode->next;

    // 06-22-2009, 15:34, YE Tianxiao <ytxjonathan@gmail.com>
    if( pNode->next )
      pNode->next->prior = pNode->prior;
  }


Exit1:
  pNode->release();
  delete pNode;
  --m_nCount;
  if( 0 == m_nCount ) 
  {
    m_pNodeHead = m_pNodeTail = NULL;
  }

  //return 0;
}

template< typename T, template< typename U > class CDNode >
inline void 
CDList<T, CDNode>::DestroyHead()
{
  ASSERT( 0 != m_nCount );
  DestroyAtAddr( m_pNodeHead );
}

template< typename T, template< typename U > class CDNode >
inline T 
CDList<T, CDNode>::RemoveHeadandReturnIt()
{
  ASSERT( 0 != m_nCount );
  return RemoveAtAddrandReturnIt( m_pNodeHead );
  //T returnValue = m_pNodeHead->data;
  //DestroyHead();
  //return returnValue;
}

template< typename T, template< typename U > class CDNode >
inline void 
CDList<T, CDNode>::DestroyTail()
{
  ASSERT( 0 != m_nCount );
  DestroyAtAddr( m_pNodeTail );
}

template< typename T, template< typename U > class CDNode >
inline T 
CDList<T, CDNode>::RemoveTailandReturnIt()
{
  ASSERT( 0 != m_nCount );
  return RemoveAtAddrandReturnIt( m_pNodeTail );
  //T returnValue = m_pNodeTail->data;
  //DestroyTail();
  //return returnValue;
}

template< typename T, template< typename U > class CDNode >
inline void 
CDList<T, CDNode>::DestroyAll()
{
  int i;
  int nCount;
  CDNode<T> *pTmpNode;

  nCount = m_nCount;
  for( i = 0; i < nCount; ++i )
  {
    pTmpNode = m_pNodeHead->next;
    m_pNodeHead->release();
    delete m_pNodeHead;
    m_pNodeHead = pTmpNode;
  }

  // 07-08-2009, 21:33, YE Tianxiao <ytxjonathan@gmail.com>
  m_pNodeTail = NULL;

  m_nCount = 0;
}


template< typename T, template< typename U > class CDNode >
inline void 
CDList<T, CDNode>::SetAt( int const pos, T data )
{
  ASSERT( 1 <= pos && pos <= m_nCount );

  int i;
  CDNode<T>* pTmpNode = FindAddrAt( pos );

  pTmpNode->data = data;
}

//start finding from tail
template< typename T, template< typename U > class CDNode >
inline int 
CDList<T, CDNode>::FindPosForData( T const data ) const
{
  int i;
  int nCount;
  CDNode<T>* pTmpNode = m_pNodeTail;

  nCount = m_nCount;
  for( i = 0; i < nCount; ++i )
  {
    if( data == pTmpNode->data )
      return i + 1;
    pTmpNode = pTmpNode->prior;
  }

  return 0;
}

template< typename T, template< typename U > class CDNode >
inline typename CDNode<T>* 
CDList<T, CDNode>::FindAddrAt( int const pos ) const
{
  ASSERT( 1 <= pos && pos <= m_nCount );

  if( 1 == pos )
  {
    return m_pNodeHead;
  }
  else if( m_nCount == pos )
  {
    return m_pNodeTail;
  }

  CDNode<T>* pTmpNode = NULL;

  if( pos <= m_nCount ) 
  {// search from head
    pTmpNode = m_pNodeHead;
    int posTemp = pos;
    while( posTemp-- > 1 )
    {
      ASSERT( pTmpNode );
      pTmpNode = pTmpNode->next;
    }
  }
  else  
  {// search from tail
    pTmpNode = m_pNodeTail;
    int posTemp = m_nCount - pos;
    while( posTemp-- > 0 )
    {
      ASSERT( pTmpNode );
      pTmpNode = pTmpNode->prior;
    }
  }

  return pTmpNode;
}

//return NULL for not find, start finding from tail
template< typename T, template< typename U > class CDNode >
inline typename CDNode<T>* 
CDList<T, CDNode>::FindAddrForData( T const data ) const
{
  int i;
  int nCount;
  CDNode<T>* pTmpNode = m_pNodeTail;

  nCount = m_nCount;
  for( i = 0; i < nCount; ++i )
  {
    if( data == pTmpNode->data )
      return pTmpNode;
    pTmpNode = pTmpNode->prior;
  }

  return NULL;
}

#endif  //(_DLIST_H__F56001F6_6EA7_444b_92CC_56D0A72B7FCF)
