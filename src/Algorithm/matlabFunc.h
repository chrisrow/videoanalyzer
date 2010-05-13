/*! 
*************************************************************************************
* \file 
*    matlabFunc.h
* \brief
*    M<<???
* \date
*    2009_05_26
* \author
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*    - Yang SONG                      <>
*************************************************************************************
*/

#if !defined  (_MATLABFUNC_H__057950B5_0DD5_4c68_B94D_3197DFCB4AB2)
#define _MATLABFUNC_H__057950B5_0DD5_4c68_B94D_3197DFCB4AB2

// #include "..\..\CommonLibStatic\src\frames.h"
// #include "..\..\CommonLibStatic\src\BasicToolsLibStatic.h"
// #include "..\..\CommonLibStatic\src\dlist.h" 
#include "Common/frames.h"
#include "Common/BasicToolsLibStatic.h"
#include "Common/dlist.h" 

// #if( 1 == MATLABFUNC )



#define RLSECTIONSTRIDE 2 //Run Length
#define RLHEADSTRIDE 2 //-y, sectNum

class CObjLabeled
{
public:
  //===== MiscInfo =====
  uint32_t m_nXYDotPlus[3];       //<! [xPlus, yPlus, dotPlus]
  uint16_t m_nOuterRect[4];       //<! [upleft_x, upleft_y, width, height]
  uint32_t m_nBGRPlus[3];         //<! [BPlus, GPlus, RPlus]

  /*m_RLcode: the minus value indicate the Y position, 
  the following x indicate the start x position,
  the next following x indicate the run value, where 0 indicate a point.

  // MinusY  == 0 means line 0,                   -1 means line 1.              Range:[-int16_t+1, 0];
  // sectNum == 0 means one section,              1 means two section           Range:[0, int16_t];
  // XStart  == 0 means start from pixel 0,       1 means start from pixel 1    Range:[0, int16_t];
  // XLength == 0 means length is one pixel only, 1 means length is two pixels. Range:[0, int16_t];
  // Code Mode: [ MinusY, sectNum, XStart, XLength, XStart, XLength, ..., MinusY, sectNum ...... ]
  */
  int16_t* m_RLcode;        //!< White Pixel data coded by Run length coding, 
  uint32_t m_nRLcodeSize;

public:
  //===== constructor, destructor =====
  CObjLabeled( int16_t const  RLcodeMinusY_in  = 0, \
                  int16_t const  RLcodeSectNum_in = 0, \
                  int16_t const  RLcodeXStart_in  = 0, \
                  int16_t const  RLcodeXLength_in = 0 );
  CObjLabeled( CObjLabeled const& obj_in );          // copy constructor
  virtual ~CObjLabeled();

  //===== operator overload =====
  CObjLabeled const& operator = ( CObjLabeled const& obj_in );

  //===== methods =====
  void     mergeWith( CObjLabeled const& obj_in );
  void     tailBy( CObjLabeled const& obj_in );
  

  void     updateMiscInfoForOneSection( void );
  void     resetLastTwoAddrLinesByUsingRLcode( CObjLabeled** pLine0_inout, \
                                     CObjLabeled** pLine1_inout, \
                                     int16_t const    nLine0MinusJ_in, \
                                     CObjLabeled* const newAddr_in ) const;
  void     decodeRLcodeAndDrawTo( uint8_t* pBuffer_inout, int16_t const nWidth_in ) const;

private:
  int16_t  m_nFirstLineInfo[2];   //<! [y0, sectNum]; store the first line number and section number
  int16_t  m_nLastLineInfo[2];    //<! [yn, sectNum]; store the last line number and section number

  uint32_t mergeRLcodeWith( int16_t const* pRLcode_in, uint32_t const nRLcodeSize_in );
  uint32_t tailRLcodeBy( int16_t const* pRLcode_in, uint32_t const nRLcodeSize_in );
  ErrVal   resetOneAddressLine( CObjLabeled** pLine_inout, int16_t const LineMinusY_in,\
                                uint32_t const RLcodeLineStartpos_in, CObjLabeled* const newAddress_in ) const;
  void     mergeMiscInfoWith( const CObjLabeled &obj_in ); 
  void     mergeColorInfoWith( const CObjLabeled &obj_in ); 
};




//! the matlab function for one video stream
class CMatlabFunc
{
public:
  ErrVal init( uint8_t nRgbThreshold_in ); // init function
  ErrVal clear();
  void   setRgbThreshold( uint8_t nRgbThreshold_in ); 
  ErrVal smoothRgb( CFrameContainer* pFrame_decoded_in, \
                    CFrameContainer* pFrame_RgbSmoothed_inout ) const;
  ErrVal binarizeY_fromRgbBkgnd( CFrameContainer* pFrame_RgbtoYBinarized_inout, \
                                 CFrameContainer const* pFrame_curr_in, \
                                 CFrameContainer const* pFrame_bkgnd_in ) const;
  ErrVal erodeY(  CFrameContainer* const pFrame_inout, uint32_t nErodeTimesPerPixel_in ) const;
  ErrVal dilateY( CFrameContainer* const pFrame_inout, uint32_t nDilateTimesPerPixel_in ) const;
  ErrVal labelObj( CDList< CObjLabeled*, CPointerDNode >* ObjDList_inout, 
    const CFrameContainer* const pFrame_RgbtoYBinarized_in, const CFrameContainer* const pFrame_Rgb_in ) const;

  void   drawObjRgbOutRect( CFrameContainer* const pFrame_inout, const CDList< CObjLabeled*, CPointerDNode >* const ObjDList_in ) const;

  void   refineObjListRgb( CDList< CObjLabeled*, CPointerDNode >* ObjDList_inout, 
    const CFrameContainer* const pFrame_Rgb_in ) const;

  static void RGB24ToYUV444( unsigned char * Src , unsigned char * Dst ,int wide ,int height );

  //根据指定矩形框内的点的亮度来判断是否是黑夜
  static bool isNight(unsigned char *pSrc, std::vector<CRect>& rectArray, int iWidth, int iHeight, int iNightRangeVal);

private:
  uint8_t m_nRgbThreshold; //<! threshold to be used in binarizeY_fromRgbBkgnd
  
};






// #endif  //( MATLABFUNC == 1 )




#endif  // safeguard