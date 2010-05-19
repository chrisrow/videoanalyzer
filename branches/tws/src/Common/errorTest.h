/*! 
*************************************************************************************
* \file 
*    errorTest.h
* \brief
*    <<! For error test 
* \date
*    2009_04_10
* \author
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*************************************************************************************
*/

#if !defined  (_ERRORTEST_H__EFAB824F_16FD_401b_BAF3_E180548237FD)
#define _ERRORTEST_H__EFAB824F_16FD_401b_BAF3_E180548237FD

#include <assert.h>

#include <iostream>
using namespace std;
//#if! defined _DEBUG 
//# define trace(s) 
//#else 
//# define trace(s) \
//  {\ 
//    printf("in file : %s, line : %u\n", __FILE__, __LINE__);\ 
//    printf((s));\ 
//    fflush(stdout);\ 
//  }
//#endif

#if defined (_DEBUG)
# if !defined (ASSERT)
# define ASSERT  assert
# endif
#else   // not _DEBUG
# if !defined (ASSERT)
# define ASSERT  
# endif
#endif  // _DEBUG


//===== error buffer =====
size_t static const MSGBUFFERSIZE = _MAX_PATH; //<! Message buffer size
char static MsgBuffer[MSGBUFFERSIZE]; //<! Message buffer//[todo]: better message method is needed.


//===== error class =====
typedef int ErrVal;

struct Err  
{
//public:
  ErrVal static const  m_nTRUE = 1; 
  ErrVal static const  m_nFALSE = 0; 
  ErrVal static const  m_nOK = 0;         
  ErrVal static const  m_nERR = -1;         
  ErrVal static const  m_nEndOfStream = -2;
  ErrVal static const  m_nEndOfFile = -3;
  ErrVal static const  m_nEndOfBuffer = -4;
  ErrVal static const  m_nInvalidParameter = -5;
  ErrVal static const  m_nDataNotAvailable = -6;

  //ErrVal const Err::m_nTRUE =              (1);     //<! Don't change the value
  //ErrVal const Err::m_nFALSE =             (0);     //<! Don't change the value
  //ErrVal const Err::m_nOK =                (0);     //<! Don't change the value
  //ErrVal const Err::m_nERR =               (-1);    //<! Don't change the value
  //ErrVal const Err::m_nEndOfStream =       (-2);
  //ErrVal const Err::m_nEndOfFile =         (-3);    //<! Don't change the value
  //ErrVal const Err::m_nEndOfBuffer =       (-4);
  //ErrVal const Err::m_nInvalidParameter =  (-5);
  //ErrVal const Err::m_nDataNotAvailable =  (-6);

};



//===== error detection macro =====

#define ERR_CLASS Err
#define ERR_VAL   ErrVal

//[todo]: the following macros should be improved in the future to supply better debug information!

//===== shortlist of the macros =====
  //RERR          = return ERR
  //ROK           = return OK
  //REOF          = return EOF
  //RNULL         = return NULL

  //RERR_NOK      = if NOT OK, return ERR
  //RERRM_NOK     = if NOT OK, return ERR with message
  //RERRMF_NOK    = if NOT OK, return ERR with message function
  //ROK_NOK       = if NOT OK, return OK   //very abnormal
  //REOF_NOK      = if NOT OK, return EOF   //very abnormal
  //EXM_NOK       = if NOT OK, exit with message  
  //EXMF_NOK      = if NOT OK, exit with message function   

  //EXM_OK        = if OK, exit with message

  //RERR_NTRUE    = if NOT TRUE, return ERR
  //RERRM_NTRUE   = if NOT TRUE, return ERR with message
  //RERRMF_NTRUE  = if NOT TRUE, return ERR with message function
  //ROK_NTRUE     = if NOT TRUE, return OK   //very abnormal
  //EXM_NTRUE     = if NOT TRUE, exit with message  
  //EXMF_NTRUE    = if NOT TRUE, exit with function message

  //RERR_TRUE     = if TRUE, return ERR
  //RERRM_TRUE    = if TRUE, return ERR with message
  //EXM_TRUE      = if TRUE, exit with message 

  //RERR_NULL     = if NULL, return ERR
  //RERRM_NULL    = if NULL, return ERR with message
  //RNULL_NULL    = if NULL, return NULL
  //EXM_NULL      = if NULL, exit with message
  //EXMF_NULL     = if NULL, exit with message function

#define RERR( )                     \
{                                   \
  return ERR_CLASS::m_nERR;         \
}

#define ROK( )                      \
{                                   \
  return ERR_CLASS::m_nOK;          \
}

#define REOF( )                     \
{                                   \
  return ERR_CLASS::m_nEndOfFile;   \
}

#define RNULL( )                     \
{                                   \
  return NULL;         \
}

#define RERR_NOK( retVal )          \
{                                   \
  if( ERR_CLASS::m_nOK != (retVal) )  \
  {                                 \
    RERR();                         \
  }                                 \
}

#define RERRM_NOK( retVal, retMsg ) \
{                                       \
  if( ERR_CLASS::m_nOK != (retVal) )    \
  {                                     \
    printf( (retMsg) );                 \
    RERR();                             \
  }                                     \
}

#define RERRMF_NOK( retVal, retMsgFunc ) \
{                                       \
  if( ERR_CLASS::m_nOK != (retVal) )    \
  {                                     \
    (retMsgFunc);                         \
    RERR();                             \
  }                                     \
}


#define ROK_NOK( retVal )           \
{                                   \
  if( ERR_CLASS::m_nOK != (retVal) )  \
  {                                 \
    ROK();                          \
  }                                 \
}

#define REOF_NOK( retVal )           \
{                                   \
  if( ERR_CLASS::m_nOK != (retVal) )  \
  {                                 \
    REOF();                          \
  }                                 \
}

#define EXM_NOK( retVal, retMsg ) \
{                                             \
  if( ERR_CLASS::m_nOK != (retVal) )          \
  {                                           \
    error( retMsg, -1 );                      \
  }                                           \
}

#define EXMF_NOK( retVal, retMsgFunc )        \
{                                             \
  if( ERR_CLASS::m_nOK != (retVal) )          \
  {                                           \
    (retMsgFunc);                             \
    exit( -1 );                               \
  }                                           \
}

#define EXM_OK( retVal, retMsg )              \
{                                             \
  if( ERR_CLASS::m_nOK == (retVal) )          \
  {                                           \
    error( retMsg, -1 );                      \
  }                                           \
}

#define RERR_NTRUE( retVal )          \
{                                     \
  if( ERR_CLASS::m_nTRUE != (retVal) )  \
  {                                   \
    RERR();                           \
  }                                   \
}

#define RERRM_NTRUE( retVal, retMsg )   \
{                                       \
  if( ERR_CLASS::m_nTRUE != (retVal) )  \
  {                                     \
    printf( (retMsg) );                   \
    RERR();                             \
  }                                     \
}

#define RERRMF_NTRUE( retVal, retMsgFunc )   \
{                                           \
  if( ERR_CLASS::m_nTRUE != (retVal) )      \
  {                                         \
    (retMsgFunc);                           \
    RERR();                                 \
  }                                         \
}

#define ROK_NTRUE( retVal )             \
{                                       \
  if( ERR_CLASS::m_nTRUE != (retVal) )  \
  {                                     \
    ROK();                              \
  }                                     \
}

#define RERR_TRUE( retVal )          \
{                                     \
  if( ERR_CLASS::m_nTRUE == (retVal) )  \
  {                                   \
  RERR();                           \
  }                                   \
}

#define RERRM_TRUE( retVal, retMsg )   \
{                                       \
  if( ERR_CLASS::m_nTRUE == (retVal) )  \
  {                                     \
    printf( (retMsg) );                 \
    RERR();                             \
  }                                     \
}

#define EXM_NTRUE( retVal, retMsg ) \
{                                             \
  if( ERR_CLASS::m_nTRUE != (retVal) )        \
  {                                           \
    error( (retMsg), -1 );                    \
  }                                           \
}

#define EXMF_NTRUE( retVal, retMsgFunc )      \
{                                             \
  if( ERR_CLASS::m_nTRUE != (retVal) )        \
  {                                           \
    (retMsgFunc);                             \
    exit( -1 );                               \
  }                                           \
}


#define EXM_TRUE( retVal, retMsg ) \
{                                             \
  if( ERR_CLASS::m_nTRUE == (retVal) )        \
  {                                           \
  error( (retMsg), -1 );                    \
  }                                           \
}

#define RERR_NULL( retVal )         \
{                                   \
  if( NULL == (retVal) )            \
  {                                 \
    RERR();                         \
  }                                 \
}

#define RERRM_NULL( retVal, retMsg )    \
{                                       \
  if( NULL == (retVal) )                \
  {                                     \
    printf( (retMsg) );                 \
    RERR();                             \
  }                                     \
}

#define RNULL_NULL( retVal )    \
{                                       \
  if( NULL == (retVal) )                \
  {                                     \
    RNULL();                             \
  }                                     \
}

#define EXM_NULL( retVal, retMsg )              \
{                                             \
  if( NULL == (retVal) )          \
  {                                           \
  error( retMsg, -1 );                      \
  }                                           \
}

#define EXMF_NULL( retVal, retMsgFunc )        \
{                                             \
  if( NULL != (retVal) )          \
  {                                           \
    (retMsgFunc);                             \
    exit( -1 );                               \
  }                                           \
}

// void error( char *text, int code );
inline void error( char* text, int code )
{
//     cout << "Error: " << text << endl;
//     exit( code );
}

// [todo]: should not but in global scope, should use enum pattern 





//enum Err_Enum
//{
//#define TMESSAGE(code,string)	code ,
//  TMESSAGE(TMSG_NOMESSAGE,      "Bogus message code %d") /* Must be first entry! */
//  TMESSAGE(TERR_ARITH_NOTIMPL,  "Sorry, there are legal restrictions on arithmetic coding")
//};

#endif // #include guard