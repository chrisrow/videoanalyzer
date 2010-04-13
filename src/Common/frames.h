/*! 
*************************************************************************************
* \file 
*    frames.h
* \brief
*    a common yuvFrame struct, to be used as the payload for all projects in this solution
* \date
*    2008_05_14
* \author
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*************************************************************************************
*/
#if !defined  (_YUVFRAME_H__99CC419F_4E3C_44cf_A2B8_1598CE76C89A)
#define _YUVFRAME_H__99CC419F_4E3C_44cf_A2B8_1598CE76C89A


#include <cstdio>

#include "BasicToolsLibStatic.h"
#include "macro.h"
#include "enum.h"
// #include "commonErrMsg.h"

#include <gdiplus.h>        // [ytx] for gdi+ draw
#pragma comment(lib,"gdiplus.lib")

#ifdef _DEBUG
    #pragma comment(lib,"cv200d.lib")
    #pragma comment(lib,"cvaux200d.lib")
    #pragma comment(lib,"cxcore200d.lib")
    #pragma comment(lib,"cxts200d.lib")
    #pragma comment(lib,"highgui200d.lib")
    #pragma comment(lib,"ml200d.lib")
#else
    #pragma comment(lib,"cv200.lib")
    #pragma comment(lib,"cvaux200.lib")
    #pragma comment(lib,"cxcore200.lib")
    #pragma comment(lib,"cxts200.lib")
    #pragma comment(lib,"highgui200.lib")
    #pragma comment(lib,"ml200.lib")
#endif

#include "cv.h"
#include "highgui.h"

// Currently, I use the set of Jpeg lib. 
#define YWIDTH_MAX   32767L       //it should be smaller than signed int16_t
#define YHEIGHT_MAX  32767L       //it should be smaller than signed int16_t

// Camera frame size
//#define STREAMCAMERSIZE_WIDTH   640       //it should be smaller than signed int16_t
//#define STREAMCAMERSIZE_HEIGHT  480       //it should be smaller than signed int16_t

//#define STREAMCAMERSIZE_WIDTH   720       //it should be smaller than signed int16_t
//#define STREAMCAMERSIZE_HEIGHT  576       //it should be smaller than signed int16_t

#define STREAMCAMERSIZE_WIDTH   352       //it should be smaller than signed int16_t
#define STREAMCAMERSIZE_HEIGHT  288       //it should be smaller than signed int16_t

// The size of FrameBuffer
#define FRAMEBUFFER_MAX      50   //it should be smaller than MAX of int32, not MAX of uint32
#define FRAMEBUFFER_CURRSIZE 10


//===== frame size =====
class CImgSize
{
public:
  uint16_t      m_nYWidth;       //!< Frame width
  uint16_t      m_nYHeight;      //!< Frame height
  
  //===== mini constructor =====
  CImgSize()
  {
    m_nYWidth = m_nYHeight = 0;
  }

  //===== operator overload =====
  friend bool operator == ( CImgSize const& info0, CImgSize const& info1 )
  {
    return !(info0.m_nYWidth != info1.m_nYWidth ||
      info0.m_nYHeight != info1.m_nYHeight);
  }

  friend bool operator != ( CImgSize const& info0, CImgSize const& info1 )
  {
    return (info0.m_nYWidth != info1.m_nYWidth ||
      info0.m_nYHeight != info1.m_nYHeight);
  }

  CImgSize const& operator = ( CImgSize const& info0 )
  {
    m_nYWidth       = info0.m_nYWidth;
    m_nYHeight      = info0.m_nYHeight;
  }
};


//! CFrameContainer
class CFrameContainer
{
public:
  //===== Yuv buffer =====
  uint8_t*      m_YuvBuffer;      //!< Yuv buffer, stored as [YImage, UImage, VImage]
  uint8_t*      m_YuvPlane[3];    //!< [Y, U, V] pointer to m_YuvBuffer [YImage, UImage, VImage]

  /******************************************************************************************* 
  * Note: The RGB pixel is stored as B, G, R, continuously in m_BmpBuffer, which is different
  * from the YUV storage method in m_YuvBuffer.
  ********************************************************************************************/
  //===== Rgb buffer =====
  uint8_t*                m_BmpBuffer;           //!< Rgb buffer, [B,G,R, B,G,R, ..., B,G,R]
  BITMAPINFO              m_BmpBufferGdiCfg;
  Gdiplus::BitmapData     m_BmpBufferGdiPlusCfg; //!< Rgb buffer config data, to be used with Gdiplus::Bitmap

  IplImage  *m_pIplImage;

public:
  //===== constructor, destructor =====
  CFrameContainer( uint16_t const nYWidth_in   = 0, 
                   uint16_t const nYHeight_in  = 0, 
                   YUVTYPE  const YuvType_in   = YUVTYPE_444 ); // constructor
  void reinit(     uint16_t const nYWidth_in   = 0, 
                   uint16_t const nYHeight_in  = 0, 
                   YUVTYPE  const YuvType_in   = YUVTYPE_444 ); // constructor
  CFrameContainer( CFrameContainer const& frame_in );          // copy constructor
  ~CFrameContainer ();                               // destructor

  //===== operator overload =====
  CFrameContainer const& operator = ( CFrameContainer const& frame_in );

  //===== methods =====
  inline CImgSize const& getImgSize ()  const { return m_ImgSize; } // return const reference in order to  disable outside modification
  inline uint16_t const& getWidth ()    const { return m_ImgSize.m_nYWidth; }
  inline uint16_t const& getHeight ()   const { return m_ImgSize.m_nYHeight; }
  inline YUVTYPE  const& getYuvType ()  const { return m_YuvType; }
  inline uint32_t const& getYSize ()    const { return m_nYuvByteSize[0]; } 
  inline uint32_t const& getRSize ()    const { return m_nRgbByteSize[0]; } 
  inline uint32_t const& getYuvSize ()  const { return m_nYuvByteSize[3]; }  
  inline uint32_t const& getRgbSize ()  const { return m_nRgbByteSize[3]; }  

  inline const IplImage* getImage() const { return m_pIplImage; }
  
  void updateYUV444FromRGB24(); // convert YUV444(m_YuvBuffer) to RGB24(m_BmpBuffer), c coding 
  void updateRGB24FromYUV444(); // convert RGB24(m_BmpBuffer) to YUV444(m_YuvBuffer), c coding
  void setChromaTo128();        // set U and V to be 128, c coding
  void cvtY1toY255();           // set y == 1 to be 255, c coding
  void randomYUV444();          // generate a random YUV 444 image for test purpose

private:

  CImgSize      m_ImgSize;
  YUVTYPE       m_YuvType;       //!< YUV type (YUVTYPE_Y, YUVTYPE_420, YUVTYPE_422 or YUVTYPE_444)

  //===== Yuv and Rgb buffer byte size =====
  uint32_t      m_nYuvByteSize[4]; //!< [Y, V, U, Yuv] size in byte 
  uint32_t      m_nRgbByteSize[4]; //!< [R, G, B, Rgb] size in byte 

};

//! CFramesBuffer
class CFramesBuffer
{
public: 
  //===== data port =====
  struct TStatus
  {
    uint32_t        nAllocatedFrames;       //!< Number of frames allocated in the picture buffer
  } status;

  //===== constructor, destructor =====
  CFramesBuffer( uint32_t const nBufferSize_in = 1,         \
                 uint16_t const nYWidth_in     = 0,         \
                 uint16_t const nYHeight_in    = 0,         \
                 YUVTYPE  const  YuvType_in    = YUVTYPE_444 ); //Range of BufferSize_in: [0, FRAMEBUFFER_MAX] 
                                                                //Range of nYWidth_in: [0, YWIDTH_MAX] 
                                                                //Range of nYHeight_in: [0, YHEIGHT_MAX] 
  ~CFramesBuffer();

  //===== methods =====
  //inline uint16_t const getWidth () const   { return m_nYWidth; }
  //inline uint16_t const getHeight () const  { return m_nYHeight; }
  inline uint32_t const getBufferSize () const  { return m_nFramesBufferSize; }

  const CFrameContainer* const getFrame( uint32_t const nFrameNum = 0 ) const;
  CFrameContainer* operator[] ( uint32_t const nFrameNum_in ) const; // todo: this function is a substitute for getframe() in the future
  void pushFrame( const CFrameContainer* const pFrame_in );

private:
  CFrameContainer** m_FramesBuffer;      //!< Pictures buffer
  uint32_t          m_nFramesBufferSize; //!< Maximum number of frames allowed in the picture buffer
  uint16_t          m_nYWidth;           //!< Frame width. Note width should below the int16_t
  uint16_t          m_nYHeight;          //!< Frame height. Note height should below the int16_t
};

//void InitRGB2YUVTable();

#endif