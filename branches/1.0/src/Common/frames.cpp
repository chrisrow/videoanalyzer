/*!
************************************************************************
* \file
*     frame_temp.cpp
* \brief
*     <<?? 
* \date
*     2008_05_14
* \author
*     - Tianxiao YE                    <ytxjonathan@gmail.com>
************************************************************************
*/


#include "stdafx.h"
#include "frames.h"

//===== Use MemLeak Detector. ===== Note: Put it after last #include
#include "leakWatcher.h"
#ifdef _DEBUG
#   define new DEBUG_NEW
#   undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//===== RGB to YUV look up table method (abandoned method) =====
//// Conversion from RGB to YUV420
//int16_t static RGB2YUV_YB[256], RGB2YUV_YG[256], RGB2YUV_YR[256];
//int16_t static RGB2YUV_UB[256], RGB2YUV_UG[256], RGB2YUV_UR[256];
//int16_t static RGB2YUV_VB[256], RGB2YUV_VG[256], RGB2YUV_VR[256];
//
//void InitRGB2YUVTable()
//{
//  uint16_t i;
//
//  //bVal = (m_BmpBuffer[bmp_pos++])<<7;
//  //gVal = (m_BmpBuffer[bmp_pos++])<<7;
//  //rVal = (m_BmpBuffer[bmp_pos++])<<7; 
//
//
//  ////m_YuvPlane[0][yuv_pos]= MIN( 255, MAX( 0, ((rVal>>2)+(rVal>>5)+(rVal>>6) + 
//  //(gVal>>1)+(gVal>>4)+(gVal>>6) +(gVal>>7) 
//  //  + (bVal>>3) -(bVal>>7))>>7 ) ); //Y
//  ////m_YuvPlane[1][yuv_pos]= MIN( 255, MAX( 0, ((-(rVal>>2)+(rVal>>4)+(rVal>>6) 
//  //-(gVal>>2)-(gVal>>4)-(gVal>>6) 
//  //  +(bVal>>1))>>7) +128) ); //U
//  ////m_YuvPlane[2][yuv_pos]= MIN( 255, MAX( 0, (((rVal>>1) 
//  //-(gVal>>1)+(gVal>>4)+(gVal>>6) 
//  //  -(bVal>>4)-(bVal>>6))>>7) +128) ); //V
//
//  for (i = 0; i < 256; i++) RGB2YUV_YB[i] =  ((i<<7)>>3)-((i<<7)>>7);
//  for (i = 0; i < 256; i++) RGB2YUV_YG[i] =  ((i<<7)>>1)+((i<<7)>>4)+((i<<7)>>6)+((i<<7)>>7);
//  for (i = 0; i < 256; i++) RGB2YUV_YR[i] =  ((i<<7)>>2)+((i<<7)>>5)+((i<<7)>>6);
//  for (i = 0; i < 256; i++) RGB2YUV_UB[i] =  ((i<<7)>>1);
//  for (i = 0; i < 256; i++) RGB2YUV_UG[i] = -((i<<7)>>2)-((i<<7)>>4)-((i<<7)>>6);
//  for (i = 0; i < 256; i++) RGB2YUV_UR[i] = -((i<<7)>>2)+((i<<7)>>4)+((i<<7)>>6);
//  for (i = 0; i < 256; i++) RGB2YUV_VB[i] = -((i<<7)>>4)-((i<<7)>>6);
//  for (i = 0; i < 256; i++) RGB2YUV_VG[i] = -((i<<7)>>1)+((i<<7)>>4)+((i<<7)>>6);
//  for (i = 0; i < 256; i++) RGB2YUV_VR[i] =  ((i<<7)>>1);
//}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     Constructor of CYuvStream.
/// \return
///     none
///////////////////////////////////////////////////////////////////////
CFrameContainer::CFrameContainer( uint16_t const    nYWidth_in, 
                                  uint16_t const    nYHeight_in, 
                                  YUVTYPE const     YuvType_in ):
m_YuvBuffer ( NULL ),
m_BmpBuffer ( NULL ),
m_pIplImage ( NULL )
{
  reinit( nYWidth_in, 
          nYHeight_in, 
          YuvType_in );
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     reinit of CYuvStream. 
///     can be used to change the frame size and type
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void
CFrameContainer::reinit( uint16_t const    nYWidth_in, 
                         uint16_t const    nYHeight_in, 
                         YUVTYPE const     YuvType_in )
{

  //===== check data range =====
  EXM_TRUE( ( nYWidth_in > YWIDTH_MAX || nYHeight_in > YHEIGHT_MAX ), "YWidth or YHeight of CFrameContainer is too large!");

  this->~CFrameContainer();

  m_ImgSize.m_nYWidth      = nYWidth_in;
  m_ImgSize.m_nYHeight     = nYHeight_in;
  m_YuvType      = YuvType_in;

  //===== init Yuv image size =====
  switch( m_YuvType ) 
  {
  case YUVTYPE_4: 
    m_nYuvByteSize[0]  = uint32_t( m_ImgSize.m_nYWidth ) *  m_ImgSize.m_nYHeight ;
    m_nYuvByteSize[1]  = m_nYuvByteSize[2] = 0;
    m_nYuvByteSize[3]  = m_nYuvByteSize[0] + m_nYuvByteSize[1] + m_nYuvByteSize[2];
    break;
  case YUVTYPE_420: 
    m_nYuvByteSize[0]  = uint32_t( m_ImgSize.m_nYWidth ) *  m_ImgSize.m_nYHeight ;
    m_nYuvByteSize[1]  = m_nYuvByteSize[2] = m_nYuvByteSize[0] / 4;
    m_nYuvByteSize[3]  = m_nYuvByteSize[0] + m_nYuvByteSize[1] + m_nYuvByteSize[2];
    break;
  case YUVTYPE_422: 
    m_nYuvByteSize[0]  = uint32_t( m_ImgSize.m_nYWidth ) *  m_ImgSize.m_nYHeight ;
    m_nYuvByteSize[1]  = m_nYuvByteSize[2] = m_nYuvByteSize[0] / 2;
    m_nYuvByteSize[3]  = m_nYuvByteSize[0] + m_nYuvByteSize[1] + m_nYuvByteSize[2];
    break;
  case YUVTYPE_444:
    m_nYuvByteSize[0]  = m_nYuvByteSize[1] = m_nYuvByteSize[2] = uint32_t( m_ImgSize.m_nYWidth ) *  m_ImgSize.m_nYHeight ;
    m_nYuvByteSize[3]  = m_nYuvByteSize[0] + m_nYuvByteSize[1] + m_nYuvByteSize[2];
    break;
  default :
      ;
//     error( "switch (m_YUVType)", -1 );
  }

  //===== init Yuv buffer and connect with m_YuvPlane =====
  if( m_nYuvByteSize[3] ) // if buffersize is not zero
  {
    m_YuvBuffer = new uint8_t [m_nYuvByteSize[3]]; 
    ASSERT( m_YuvBuffer );

    memset( m_YuvBuffer, NULL, m_nYuvByteSize[3]*sizeof(m_YuvBuffer[0]) );

    // Y, U, V point to the specific position in YUVBUFFER
    m_YuvPlane[0] = &m_YuvBuffer[0];
    m_YuvPlane[1] = &m_YuvBuffer[m_nYuvByteSize[0]];
    m_YuvPlane[2] = &m_YuvBuffer[m_nYuvByteSize[0]+m_nYuvByteSize[1]];
  }


  //===== init Rgb image size =====
  m_nRgbByteSize[0] = m_nRgbByteSize[1] = m_nRgbByteSize[2] = m_nYuvByteSize[0];
  m_nRgbByteSize[3] = m_nRgbByteSize[0] + m_nRgbByteSize[1] + m_nRgbByteSize[2];



  //===== init m_BmpBufferGdiCfg ===== 
  m_BmpBufferGdiCfg.bmiColors->rgbBlue=\
    m_BmpBufferGdiCfg.bmiColors->rgbGreen=\
    m_BmpBufferGdiCfg.bmiColors->rgbRed=\
    m_BmpBufferGdiCfg.bmiColors->rgbReserved=171; //this value is observed from the function "StreamNotifyMV800(const BYTE* pDIBHead_in," in TestGUI.

  m_BmpBufferGdiCfg.bmiHeader.biSize          = sizeof( m_BmpBufferGdiCfg.bmiHeader );
  m_BmpBufferGdiCfg.bmiHeader.biWidth         = (LONG)m_ImgSize.m_nYWidth;
  m_BmpBufferGdiCfg.bmiHeader.biHeight        = -(LONG)m_ImgSize.m_nYHeight; //minus value for top-down image
  m_BmpBufferGdiCfg.bmiHeader.biPlanes        = 1; // this value is observed from the function "StreamNotifyMV800(const BYTE* pDIBHead_in," in TestGUI.
  m_BmpBufferGdiCfg.bmiHeader.biBitCount      = 24; // because of PixelFormat24bppRGB 
  m_BmpBufferGdiCfg.bmiHeader.biCompression   = 0; // this value is observed from the function "StreamNotifyMV800(const BYTE* pDIBHead_in," in TestGUI.
  m_BmpBufferGdiCfg.bmiHeader.biSizeImage     = 3 * (DWORD)m_ImgSize.m_nYWidth * (DWORD)m_ImgSize.m_nYHeight; // because of PixelFormat24bppRGB
  m_BmpBufferGdiCfg.bmiHeader.biXPelsPerMeter = 0; // this value is observed from the function "StreamNotifyMV800(const BYTE* pDIBHead_in," in TestGUI.
  m_BmpBufferGdiCfg.bmiHeader.biYPelsPerMeter = 0; // this value is observed from the function "StreamNotifyMV800(const BYTE* pDIBHead_in," in TestGUI.
  m_BmpBufferGdiCfg.bmiHeader.biClrUsed       = 0; // this value is observed from the function "StreamNotifyMV800(const BYTE* pDIBHead_in," in TestGUI.
  m_BmpBufferGdiCfg.bmiHeader.biClrImportant  = 0; // this value is observed from the function "StreamNotifyMV800(const BYTE* pDIBHead_in," in TestGUI.
  
  //===== init m_BmpBufferGdiPlusCfg =====
  m_BmpBufferGdiPlusCfg.Width    = m_ImgSize.m_nYWidth;
  m_BmpBufferGdiPlusCfg.Height   = m_ImgSize.m_nYHeight;
  m_BmpBufferGdiPlusCfg.Stride   = 3*m_BmpBufferGdiPlusCfg.Width;
  m_BmpBufferGdiPlusCfg.PixelFormat  = PixelFormat24bppRGB;
  m_BmpBufferGdiPlusCfg.Scan0    = NULL;
  m_BmpBufferGdiPlusCfg.Reserved = NULL;

  //===== init Rgb buffer and connect with "m_BmpBufferGdiPlusCfg.Scan0" =====
  if( m_nRgbByteSize[3] ) // if buffer size is not zero
  {
    m_BmpBuffer = new uint8_t [m_nRgbByteSize[3]]; 
    ASSERT( m_BmpBuffer );

    memset( m_BmpBuffer, NULL, m_nRgbByteSize[3]*sizeof(m_BmpBuffer[0]) );

    m_BmpBufferGdiPlusCfg.Scan0 = (void*) m_BmpBuffer;

    m_pIplImage = cvCreateImageHeader( cvSize(nYWidth_in, nYHeight_in), 8, 3);
    m_pIplImage->imageData = (char*)m_BmpBuffer;
  }

}


///////////////////////////////////////////////////////////////////////
/// \brief 
///     copy constructor of CFrameContainer.
/// \return
///     none
///////////////////////////////////////////////////////////////////////
CFrameContainer::CFrameContainer( const CFrameContainer &frame_in )
                                  :m_YuvBuffer       ( NULL ),
                                  m_BmpBuffer       ( NULL ),
                                  m_pIplImage ( NULL )
{
  *this = frame_in;
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     Destructor of CFrameContainer.
/// \return
///     none
///////////////////////////////////////////////////////////////////////
CFrameContainer::~CFrameContainer()
{
  //===== release buffer =====
  SAFEDELETEARRAY( m_YuvBuffer );
  SAFEDELETEARRAY( m_BmpBuffer );

  cvReleaseImageHeader(&m_pIplImage);
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     = operator overload of CFrameContainer.
/// \return
///     none
///////////////////////////////////////////////////////////////////////
const CFrameContainer & 
CFrameContainer::operator=(const CFrameContainer &frame_in)
{
  if( this == &frame_in ) // object assigned to itself
    return *this; // all done

  //===== copy image size =====
  m_ImgSize.m_nYWidth       = frame_in.m_ImgSize.m_nYWidth;
  m_ImgSize.m_nYHeight      = frame_in.m_ImgSize.m_nYHeight;
  m_YuvType                     = frame_in.m_YuvType;
  memcpy( m_nYuvByteSize, frame_in.m_nYuvByteSize, sizeof(m_nYuvByteSize) );
  memcpy( m_nRgbByteSize, frame_in.m_nRgbByteSize, sizeof(m_nYuvByteSize) );

  //===== reinit Yuv buffer =====
  SAFEDELETEARRAY( m_YuvBuffer );
  m_YuvPlane[0] = m_YuvPlane[1] = m_YuvPlane[2] = NULL;

  if( m_nYuvByteSize[3] )
  {
    m_YuvBuffer = new uint8_t [m_nYuvByteSize[3]]; 
    ASSERT( m_YuvBuffer );

    // duplicate Yuv
    memcpy( m_YuvBuffer, frame_in.m_YuvBuffer, m_nYuvByteSize[3]*sizeof(m_YuvBuffer[0]) );

    // Y, U, V point to the specific position in YUVBUFFER
    m_YuvPlane[0] = &m_YuvBuffer[0];
    m_YuvPlane[1] = &m_YuvBuffer[m_nYuvByteSize[0]];
    m_YuvPlane[2] = &m_YuvBuffer[m_nYuvByteSize[0]+m_nYuvByteSize[1]];
  }

  //===== reinit Rgb buffer =====
  memcpy( &m_BmpBufferGdiPlusCfg, &frame_in.m_BmpBufferGdiPlusCfg, sizeof(m_BmpBufferGdiPlusCfg) );
  memcpy( &m_BmpBufferGdiCfg, &frame_in.m_BmpBufferGdiCfg, sizeof(m_BmpBufferGdiCfg) );

  SAFEDELETEARRAY( m_BmpBuffer );
  m_BmpBufferGdiPlusCfg.Scan0 = NULL;

  if( m_nRgbByteSize[3] ) // if buffersize is not zero
  {
    m_BmpBuffer = new uint8_t [m_nRgbByteSize[3]]; 
    ASSERT( m_BmpBuffer );

    // duplicate the content of incoming YUVBUFFER
    memcpy( m_BmpBuffer, frame_in.m_BmpBuffer, m_nRgbByteSize[3]*sizeof(m_BmpBuffer[0]) );

    // update Rgb pointer
    m_BmpBufferGdiPlusCfg.Scan0 = (void*) m_BmpBuffer;

    m_pIplImage = cvCreateImageHeader( cvSize(m_ImgSize.m_nYWidth, m_ImgSize.m_nYHeight), 8, 3);
    m_pIplImage->imageData = (char*)m_BmpBuffer;
  }


  return *this; // return reference to invoking object
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     convert YUV444(m_YuvBuffer) to RGB24(m_BmpBuffer).
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CFrameContainer::updateRGB24FromYUV444( void )
{
  int16_t   yVal = 0, uVal = 0, vVal = 0;
  uint32_t  bmp_pos = 0, yuv_pos = 0;

  // ITU-R version of the conversion formula 
  // m_nYuvByteSize[0] is YByteSize
  for( yuv_pos = 0, bmp_pos = 0;  yuv_pos < m_nYuvByteSize[0]; ++yuv_pos )
  {

    ////===== floating accurate slow approach =====
    //yVal = m_YuvPlane[0][yuv_pos];
    //uVal = m_YuvPlane[1][yuv_pos]- 128;
    //vVal = m_YuvPlane[2][yuv_pos]- 128;
    //m_BmpBuffer[bmp_pos++] = MIN( 255, MAX( 0, yVal + 1.772 * uVal ) ); //B
    //m_BmpBuffer[bmp_pos++] = MIN( 255, MAX( 0, yVal - 0.344 * uVal ) - 0.714* vVal  ); //G
    //m_BmpBuffer[bmp_pos++] = MIN( 255, MAX( 0, yVal + 1.402 * vVal ) ); //R

    

    //===== integer approximated fast approach =====
    yVal = m_YuvPlane[0][yuv_pos];
    uVal = ((int16_t)m_YuvPlane[1][yuv_pos] - 128)<<7;
    vVal = ((int16_t)m_YuvPlane[2][yuv_pos] - 128)<<7;
    m_BmpBuffer[bmp_pos++]= MIN( 255, MAX( 0, yVal + (( uVal + (uVal>>1) + (uVal>>2) + (uVal>>6) )>>7) ) );  // B
    m_BmpBuffer[bmp_pos++]= MIN( 255, MAX( 0, yVal - (((( (uVal>>2) + (uVal>>4) + (uVal>>5) ) + ( (vVal>>1) + (vVal>>3) + (vVal>>4) + (vVal>>5) ) ))>>7) )); // G
    m_BmpBuffer[bmp_pos++]= MIN( 255, MAX( 0, yVal + (( vVal + (vVal>>2) + (vVal>>3) + (vVal>>5) )>>7) ) );  // R
  }

  return;
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     convert RGB24(m_BmpBuffer) to YUV444(m_YuvBuffer).
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CFrameContainer::updateYUV444FromRGB24( void )
{
  int16_t   rVal = 0, gVal = 0, bVal = 0;
  uint32_t  bmp_pos = 0, yuv_pos = 0;

  // ITU-R version of the conversion formula 
  // m_nYuvByteSize[0] is YByteSize
  for( yuv_pos = 0, bmp_pos = 0; yuv_pos < m_nYuvByteSize[0]; ++yuv_pos, bmp_pos+=3 )
  {

    ////===== floating accurate slow approach =====
    //bVal = m_BmpBuffer[bmp_pos];
    //gVal = m_BmpBuffer[bmp_pos+1];
    //rVal = m_BmpBuffer[bmp_pos+2];
    //m_YuvPlane[0][yuv_pos]= MIN( 255, MAX( 0, 0.299*rVal + 0.587 * gVal + 0.114 * bVal ) ); //Y
    //m_YuvPlane[1][yuv_pos]= MIN( 255, MAX( 0, -0.169*rVal - 0.331 * gVal + 0.499 * bVal +128) ); //U
    //m_YuvPlane[2][yuv_pos]= MIN( 255, MAX( 0, 0.499*rVal -0.418 * gVal - 0.0813 * bVal +128) ); //V

    //===== lookup table approach =====
    ////it seems this approach is slower than "integer approximated fast approach" because the lookup table is not const static
    //m_YuvPlane[0][yuv_pos] = MIN( 255, MAX( 0, (  RGB2YUV_YB[ m_BmpBuffer[bmp_pos] ] + RGB2YUV_YG[ m_BmpBuffer[bmp_pos+1] ] + RGB2YUV_YR[ m_BmpBuffer[bmp_pos+2] ] )>>7 ) ); //Y
    //m_YuvPlane[1][yuv_pos] = MIN( 255, MAX( 0, (( RGB2YUV_UB[ m_BmpBuffer[bmp_pos] ] + RGB2YUV_UG[ m_BmpBuffer[bmp_pos+1] ] + RGB2YUV_UR[ m_BmpBuffer[bmp_pos+2] ] )>>7) +128 ) ); //U
    //m_YuvPlane[2][yuv_pos] = MIN( 255, MAX( 0, (( RGB2YUV_VB[ m_BmpBuffer[bmp_pos] ] + RGB2YUV_VG[ m_BmpBuffer[bmp_pos+1] ] + RGB2YUV_VR[ m_BmpBuffer[bmp_pos+2] ] )>>7) +128 ) ); //V



    //===== integer approximated fast approach =====
    //>>1 = 0.5
    //>>2 = 0.25
    //>>3 = 0.125
    //>>4 = 0.0625
    //>>5 = 0.03125
    //>>6 = 0.015625
    //>>7 = 0.0078125

    //0.299 = 0.250 + 0.03125 + 0.015625 = 0.296875;              dif = -0.002125
    //  >>2+>>5+>>6
    //  rVal>>2+rVal>>5+rVal>>6
    //0.587 = 0.5 + 0.0625 + 0.015625 + 0.0078125= 0.5859375;     dif = -0.0010625
    //  >>1+>>4+>>6+>>7
    //  gVal>>1+gVal>>4+gVal>>6+gVal>>7
    //0.114 = 0.125 - 0.0078125 = 0.1171875;                      dif = +0.0031875
    //  >>3->>7
    //  bVal>>3-bVal>>7

    //-0.169 = -0.25 + 0.0625 +0.015625 = -0.171875;              dif = -0.002875
    //  ->>2+>>4+>>6
    //  -rVal>>2+rVal>>4+rVal>>6
    //-0.331 = -0.25 - 0.0625 -0.015625 = -0.328125;              dif = +0.002875
    //  ->>2->>4->>6
    //  -gVal>>2-gVal>>4-gVal>>6
    // 0.499 =  0.5;                                              dif = +0.001
    //   >>1
    //   bVal>>1

    //-0.418 = -0.5 + 0.0625 + 0.015625 = -0.421875;              dif = -0.003875
    //  ->>1+>>4+>>6
    //  -gVal>>1+gVal>>4+gVal>>6

    //-0.0813 = -0.0625 - 0.015625 = -0.078125;                   dif = +0.003175
    //  ->>4->>6
    //  -bVal>>4-bVal>>6
    
    // Note: we use <<7 instead of <<8, because 255<<8 = 65280, which is larger than 2^7-1=32767, while 255<<7 = 32640 is smaller than 2^7-1=32767.
    bVal = (m_BmpBuffer[bmp_pos])<<7;
    gVal = (m_BmpBuffer[bmp_pos+1])<<7;
    rVal = (m_BmpBuffer[bmp_pos+2])<<7; 


    m_YuvPlane[0][yuv_pos]= MIN( 255, MAX( 0, ((rVal>>2)+(rVal>>5)+(rVal>>6) + (gVal>>1)+(gVal>>4)+(gVal>>6) +(gVal>>7) + (bVal>>3) -(bVal>>7))>>7 ) ); //Y
    m_YuvPlane[1][yuv_pos]= MIN( 255, MAX( 0, ((-(rVal>>2)+(rVal>>4)+(rVal>>6) -(gVal>>2)-(gVal>>4)-(gVal>>6) +(bVal>>1))>>7) +128) ); //U
    m_YuvPlane[2][yuv_pos]= MIN( 255, MAX( 0, (((rVal>>1) -(gVal>>1)+(gVal>>4)+(gVal>>6) -(bVal>>4)-(bVal>>6))>>7) +128) ); //V




  }

  return;
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     set U and V to be 128.
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CFrameContainer::setChromaTo128( void )
{
  if( m_nYuvByteSize[1] && m_nYuvByteSize[2] )
  {
    memset( m_YuvPlane[1], (uint8_t)128, (m_nYuvByteSize[1]+m_nYuvByteSize[2])*sizeof(m_YuvPlane[1][0]) );
  }
  return;
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     set Y ==1 to be Y = 255 to facilitate display
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CFrameContainer::cvtY1toY255( void )      // set y == 1 to be 255, c coding
{
  uint32_t n32Temp = 0;
  uint32_t const& nYsize = m_nYuvByteSize [0];
  for( n32Temp = 0; n32Temp<nYsize; ++n32Temp )
  {
    m_YuvPlane[0][n32Temp] = m_YuvPlane[0][n32Temp] == 1 ? 255 : m_YuvPlane[0][n32Temp];
  }
}
///////////////////////////////////////////////////////////////////////
/// \brief 
///     ?
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CFrameContainer::randomYUV444( void )      
{
  uint32_t const nYuvSize = getYuvSize();
  for( uint32_t nTemp = 0; nTemp < nYuvSize; ++nTemp )
  {
    m_YuvBuffer[nTemp] = (unsigned char)(rand() / 129 /2 + 0.5);
  }
  return;
}


///////////////////////////////////////////////////////////////////////
/// \brief 
///     Constructor of CFramesBuffer.
/// \return
///     none
///////////////////////////////////////////////////////////////////////
CFramesBuffer::CFramesBuffer( uint32_t const nBufferSize_in,
                              uint16_t const nYWidth_in, 
                              uint16_t const nYHeight_in, 
                              YUVTYPE const  YuvType_in)
                              :m_nYWidth ( nYWidth_in ),
                              m_nYHeight ( nYHeight_in ),
                              m_FramesBuffer  ( NULL )
{
  uint32_t n32Temp = 0;

  //===== init status =====
  status.nAllocatedFrames = 0;

  //===== check data range =====
  EXM_NTRUE( ( 0 < nBufferSize_in )&&( nBufferSize_in <= FRAMEBUFFER_MAX ), "BufferSize is too large or too small!" );
  m_nFramesBufferSize = nBufferSize_in;

  if( m_nFramesBufferSize )
  {
    m_FramesBuffer = new CFrameContainer* [m_nFramesBufferSize];

    for( n32Temp = 0; n32Temp < m_nFramesBufferSize; ++n32Temp )
    {
      m_FramesBuffer[n32Temp] = new CFrameContainer(nYWidth_in, nYHeight_in, YuvType_in);
    }
  }

}




///////////////////////////////////////////////////////////////////////
/// \brief 
///     Destructor of CFrameContainer.
/// \return
///     none
///////////////////////////////////////////////////////////////////////
CFramesBuffer::~CFramesBuffer()
{
  uint32_t n32Temp = 0;

  //===== release buffer =====
  if( m_FramesBuffer )
  {
    for( n32Temp = 0; n32Temp < m_nFramesBufferSize; ++n32Temp )
    {
      SAFEDELETE( m_FramesBuffer[n32Temp] );
    }
    SAFEDELETEARRAY( m_FramesBuffer );
  }

}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     Get the specific frame.
/// \return
///     frame pointer
///////////////////////////////////////////////////////////////////////
const CFrameContainer* const
CFramesBuffer::getFrame( uint32_t const nFrameNum_in ) const
{
  ASSERT( nFrameNum_in <= status.nAllocatedFrames );
  ASSERT( m_FramesBuffer[nFrameNum_in] );
  return m_FramesBuffer[nFrameNum_in];
}


CFrameContainer* 
CFramesBuffer::operator[] ( uint32_t const nFrameNum_in ) const//for cin>>
{
  ASSERT( nFrameNum_in <= status.nAllocatedFrames );
  ASSERT( m_FramesBuffer[nFrameNum_in] );
  return m_FramesBuffer[nFrameNum_in];
}


///////////////////////////////////////////////////////////////////////
/// \brief 
///     Push the specific frame to the top of buffer .
/// \return
///     frame pointer
///////////////////////////////////////////////////////////////////////
void 
CFramesBuffer::pushFrame( const CFrameContainer* const pFrame_in )
{
  ASSERT( pFrame_in );

  int32_t n32Temp = 0;

  if( 1 == m_nFramesBufferSize )  
  {// buffer size is equal to one
    // copy the data of YUVFRAME_IN into the top of buffer.
    *m_FramesBuffer[0] = *pFrame_in;
  }
  else
  {// buffer size is larger than one
    CFrameContainer* pLastFrame = m_FramesBuffer[m_nFramesBufferSize-1];

    // shift buffer pointers
    for( n32Temp = m_nFramesBufferSize-1; n32Temp > 0; --n32Temp )
    {
      m_FramesBuffer[n32Temp] = m_FramesBuffer[n32Temp-1];
    }
    m_FramesBuffer[0] = pLastFrame;

    // update head frame
    *m_FramesBuffer[0] = *pFrame_in;
  }
  
  // update status
  status.nAllocatedFrames = MIN( status.nAllocatedFrames+1, m_nFramesBufferSize ); 

  return;
}






//
//
//CFrameContainer* popFrame();
//void pushFrame( CFrameContainer const* pFrame_in );
//
//
//
//void CFramesBuffer::shift()
//{
//  CFrameContainer *frame = m_FramesBuffer[status.nAllocatedFrames];
//  uint32_t nTemp = 0;
//
//  for( nTemp = status.nAllocatedFrames; nTemp > 0; --nTemp )
//    m_FramesBuffer[nTemp] = m_FramesBuffer[nTemp-1];
//
//  m_FramesBuffer[0] = frame;
//
//  // update status
//  ++status.nAllocatedFrames;
//
//  return;
//}
//
//void CFramesBuffer::unshift()
//{
//  CFrameContainer *frame = m_FramesBuffer[status.nAllocatedFrames];
//  uint32_t nTemp = 0;
//
//  for( nTemp = status.nAllocatedFrames; nTemp > 0; --nTemp )
//    m_FramesBuffer[nTemp] = m_FramesBuffer[nTemp-1];
//
//  m_FramesBuffer[0] = frame;
//
//  // update status
//  --status.nAllocatedFrames;
//
//  return;
//}

//void x264_frame_unshift( x264_frame_t **list, x264_frame_t *frame )
//{
//  int i = 0;
//  while( list[i] ) i++;
//  while( i-- )
//    list[i+1] = list[i];
//  list[0] = frame;
//}
//class CFramesBuffer
//{
//private:
//  CFrameContainer**     m_FramesBuffer;
//  uint32_t        m_nFramesBufferSize;  //!< Total frames allowed in the picture buffer
//
//public:
//  CYuvFrameBuffer( uint32_t const FramesBufferSize_in = 0,
//    uint16_t const nYWidth_in  = 0, 
//    uint16_t const nYHeight_in = 0, 
//    YUVTYPE const  YuvType_in = YUVTYPE_444 );
//  virtual ~CYuvFrameBuffer();
//
//  struct TStatus
//  {
//    uint32_t      nAllocatedFrames;           //!< Number of frames allocated in the picture buffer
//  } status;
//
//  CFrameContainer const* const getFrame( uint32_t const nFrameNum_in = 0 ) const;
//  CFrameContainer* updateFrame( uint32_t const nFrameNum_in = 0 );
//  CFrameContainer* shiftFrames();
//
//};