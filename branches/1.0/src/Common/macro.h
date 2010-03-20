/*! 
*************************************************************************************
* \file 
*    macro.h
* \brief
*    common macro for the whole project
* \date
*    2008_05_13
* \author
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*************************************************************************************
*/

#if !defined (_MACRO_H__DEFBABE6_AB25_4d84_B395_B936FC41715F)
#define _MACRO_H__DEFBABE6_AB25_4d84_B395_B936FC41715F

#if !defined (DISABLELOG) 
//# define DISABLELOG
#endif

#if !defined (MAX)
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

//template <typename T>
//static inline T MAX(T const& a, T const& b) {
//  return a > b ? a : b;
//};

#if !defined (MIN)
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

//template <typename T>
//static inline T MIN(T const& a, T const& b) {
//  return a < b ? a : b;
//};

#if !defined (NULLED)
#define NULLED(x) ((x) == NULL)
#endif

#if !defined (SUMTHREE)
#define SUMTHREE ((a), (b), (c)) ((a)+(b)+(c))
#endif

#if !defined (INRANGE)
#define INRANGE ((a), (amin), (amax)) ( (a)>=(amin) && (a)<=(amax) )
#endif

#if !defined (SWAPTWO)
#define SWAPTWO( a, b, temp ) \
{                                       \
  (temp) = (a);                         \
  (a) = (b);                            \
  (b) = (temp);                         \
}
#endif

#if !defined (WHITESPOT)
#define WHITESPOT 1    //pixel value for white
#endif

#if !defined (BLACKSPOT)
#define BLACKSPOT 0    //pixel value for black
#endif


// common macros
#if !defined (SAFEDELETE)
#define SAFEDELETE(p) if( (p) ){ delete (p); (p) = NULL; }
#endif

#if !defined (SAFEDELETEARRAY)
#define SAFEDELETEARRAY(p) if( (p) ){ delete [] (p); (p) = NULL; }
#endif

#if !defined (RELEASE)
#define RELEASE( p ) if( p != NULL ) { p->Release(); p = NULL; }
#endif


//#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))
//#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
//#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
//#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
//#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#endif