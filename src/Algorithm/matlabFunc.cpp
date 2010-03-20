/*!
************************************************************************
* \file
*    pMatlabFunc.cpp
* \brief
*    <<?? 
* \date
*    2008_04_13
* \author
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
************************************************************************
*/

#include "stdafx.h"
#include "matlabFunc.h"

#include <iostream>
using namespace std;

//Use MemLeak Detector. Note: Put it after last #include
#include "Common/leakWatcher.h"
#ifdef _DEBUG
#   define new DEBUG_NEW
#   undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*test*/
int mergeRLcodeTimes;
uint32_t RLcodeTotalLength;
/*test end*/

// #if( 1 == MATLABFUNC ) 

///////////////////////////////////////////////////////////////////////
/// \brief 
///     initiate CMatlabFunc and do memory allocation
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
ErrVal 
CMatlabFunc::init( uint8_t nRgbThreshold_in )
{
  m_nRgbThreshold = nRgbThreshold_in;


  ROK();
}




///////////////////////////////////////////////////////////////////////
/// \brief 
///     clear allocated memory
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
ErrVal 
CMatlabFunc::clear()
{
  ROK();
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     set RgbThreshold for function "binarizeY_fromRgbBkgnd" 
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CMatlabFunc::setRgbThreshold( uint8_t nRgbThreshold_in )
{
  m_nRgbThreshold = nRgbThreshold_in;
  return;
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     smooth the incoming decoded frame 
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
ErrVal 
CMatlabFunc::smoothRgb( CFrameContainer* pFrame_RgbSmoothed_inout, \
                       CFrameContainer* pFrame_decoded_in ) const
{
  ASSERT( pFrame_RgbSmoothed_inout );
  ASSERT( pFrame_decoded_in );
  ROK();
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     detect active object
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
ErrVal
CMatlabFunc::binarizeY_fromRgbBkgnd( CFrameContainer* pFrame_RgbtoYBinarized_inout, \
                                    CFrameContainer const* pFrame_curr_in, \
                                    CFrameContainer const* pFrame_bkgnd_in ) const
{
  ASSERT( pFrame_RgbtoYBinarized_inout );
  ASSERT( pFrame_curr_in );
  ASSERT( pFrame_bkgnd_in );

  // init pointer
  const uint8_t* const pRgb_bkgnd = pFrame_bkgnd_in->m_BmpBuffer;
  const uint8_t* const pRgb_curr = pFrame_curr_in->m_BmpBuffer;
  uint8_t* pY_RgbtoYBinarized = pFrame_RgbtoYBinarized_inout->m_YuvPlane[0];

  uint32_t posRgb = 0, posY = 0;
  uint32_t const nRgbByteSize = pFrame_curr_in->getRgbSize();

  // Y = ( abs(B-B')+abs(G-G')+abs(R-R') >= threshold ? 255 : 0
  for( posRgb = 0, posY = 0; posRgb < nRgbByteSize; posRgb += 3, ++posY )
  {
    pY_RgbtoYBinarized[posY] = \
      abs( (int16_t)pRgb_bkgnd[posRgb]   - pRgb_curr[posRgb] )   + \
      abs( (int16_t)pRgb_bkgnd[posRgb+1] - pRgb_curr[posRgb+1] ) + \
      abs( (int16_t)pRgb_bkgnd[posRgb+2] - pRgb_curr[posRgb+2] ) \
      >= m_nRgbThreshold \
      ? WHITESPOT : BLACKSPOT;
  }

  // set U, V to 128
  //pFrame_RgbtoYBinarized_inout->setChromaTobe128();

  ROK();
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     erode
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
//todo: clear and optimize the following function
ErrVal
CMatlabFunc::erodeY( CFrameContainer* const pFrame_in, uint32_t nErodeTimesPerPixel_in ) const
{
  ASSERT( pFrame_in );

  int m,n,i=0,j=0;
  uint32_t k=0;

  uint16_t const nWidth=pFrame_in->getWidth();//Í¼ÏñµÄ¿í
  uint16_t const nHeight=pFrame_in->getHeight();//Í¼Ïñ¸ß

  uint8_t mpiex,mpiex1;
  bool flag=FALSE;
  uint8_t *pData = new uint8_t[nWidth*nHeight];

  ASSERT( pData );

  for( k = 0; k < nErodeTimesPerPixel_in; ++k )
  {
    memcpy(pData,pFrame_in->m_YuvPlane[0],nWidth*nHeight*sizeof(pData[0]));
    for( i = 1; i < nHeight-1; ++i )
    {
      for( j = 1; j < nWidth-1; ++j )
      {
        mpiex=pData[i*nWidth+j];
        if ( WHITESPOT == mpiex )
        {
          for (m=-1;m<2;m++)
          {
            for (n=-1;n<2;n++)
            {
              mpiex1=pData[(i+m)*nWidth+(j+n)];
              if( BLACKSPOT == mpiex1 )
              {
                pFrame_in->m_YuvPlane[0][i*nWidth+j] = BLACKSPOT;
                flag=TRUE;
                break;
              }
            }
            if(flag)
            {
              flag=FALSE;
              break;
            }
          }
        }
      }
    }  
  }

  if(pData)
    delete [] pData;

  ROK();
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     detect active object
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
//todo: clear and optimize the following function
ErrVal
CMatlabFunc::dilateY( CFrameContainer* const pFrame_in, uint32_t nDilateTimesPerPixel_in ) const
{ 
  ASSERT( pFrame_in );

  int m,n,i=0,j=0;
  uint32_t k=0;
  uint16_t nWidth=pFrame_in->getWidth();//Í¼ÏñµÄ¿í
  uint16_t nHeight=pFrame_in->getHeight();//Í¼Ïñ¸ß
  uint8_t mpiex,mpiex1;
  bool flag=FALSE;
  uint8_t *pData = new uint8_t[nWidth*nHeight];

  ASSERT( pData );

  for( k = 0; k < nDilateTimesPerPixel_in; ++k )
  {
    memcpy(pData,pFrame_in->m_YuvPlane[0],nWidth*nHeight*sizeof(pData[0]));
    for( i = 1; i < nHeight-1; ++i )
    {
      for( j = 1; j < nWidth-1; ++j )
      {
        mpiex=pData[i*nWidth+j];
        if( BLACKSPOT == mpiex )
        {
          for( m = -1; m < 2; ++m )
          {
            for( n = -1; n < 2; ++n )
            {
              mpiex1=pData[(i+m)*nWidth+(j+n)];
              if( WHITESPOT == mpiex1 )
              {
                pFrame_in->m_YuvPlane[0][i*nWidth+j]=WHITESPOT;
                flag=TRUE;
                break;
              }
            }
            if(flag)
            {
              flag=FALSE;
              break;
            }
          }
        }
      }
    }  
  }

  if(pData)
    delete [] pData;

  ROK();
}

void
CMatlabFunc::drawObjRgbOutRect( CFrameContainer* const pFrame_inout, const CDList< CObjLabeled*, CPointerDNode >* const ObjDList_in ) const
{
  ASSERT( pFrame_inout );
  ASSERT( ObjDList_in );

  //const CFrameContainer* const pFrame_inout = m_pFrame_temp[9];
  for( int nObjNumTemp = 1; nObjNumTemp <= ObjDList_in->GetCount(); ++nObjNumTemp )
  {
    const CObjLabeled* const pObjTemp = ObjDList_in->GetAt(nObjNumTemp);

    uint8_t Color_B = pObjTemp->m_nBGRPlus[0] / pObjTemp->m_nXYDotPlus[2];
    uint8_t Color_G = pObjTemp->m_nBGRPlus[1] / pObjTemp->m_nXYDotPlus[2];
    uint8_t Color_R = pObjTemp->m_nBGRPlus[2] / pObjTemp->m_nXYDotPlus[2];


    //if( Color_B == Color_G == Color_R )
    //{
    //  //Color_B = Color_G = Color_R = 128;
    //}
    //else if( Color_B == Color_G )
    //{
    //  //Color_B = Color_G = 128;
    //  Color_R = 0;
    //}
    //else if( Color_G == Color_R )
    //{
    //  //Color_G = Color_R = 128;
    //  Color_B = 0;
    //}
    //else if( Color_B == Color_R )
    //{
    //  //Color_B = Color_R = 128;
    //  Color_G = 0;
    //}
    //else
    //{
      //const uint8_t Color_maxTemp = MAX( Color_B, MAX( Color_G, Color_R ) );

      //if( Color_maxTemp == Color_B ) 
      //{
      //  Color_B = 255;
      //  Color_G = Color_R = 0;
      //}
      //else if( Color_maxTemp == Color_G ) 
      //{
      //  Color_G = 255;
      //  Color_B = Color_R = 0;
      //}
      //else
      //{
      //  Color_R = 255;
      //  Color_B = Color_G = 0;
      //}
    //}

    uint8_t const Color_Rgb[3] = {Color_B, Color_G, Color_R};
    //todo: RGB and HSV interchange
    //uint8_t Color_Rgb[3] = {0,0,0,};
    //uint16_t ColorHsv[3] = {0,0,0};
    //BGR_to_HSV( ColorHsv, Color_RgbTemp );
    //// S = 90%
    ////ColorHsv[1] = 90;
    //// V = 90%
    ////ColorHsv[2] = 205;
    ////uint8_t ColorRgbTemp[3] = {0,0,0};
    //HSV_to_BGR( Color_Rgb, ColorHsv );


    // draw double lines

    {
      uint16_t const nOuterRectOuterTemp[4] = {
        pObjTemp->m_nOuterRect[0], 
        pObjTemp->m_nOuterRect[1],
        MAX(0, pObjTemp->m_nOuterRect[2]-1),
        MAX(0, pObjTemp->m_nOuterRect[3]-1)};

      uint16_t const nOuterRectInnerTemp[4] = {
        MIN(pFrame_inout->getWidth()-1 ,pObjTemp->m_nOuterRect[0]+1), 
        MIN(pFrame_inout->getHeight()-1 ,pObjTemp->m_nOuterRect[1]+1), 
        MAX(0, pObjTemp->m_nOuterRect[2]-3),
        MAX(0, pObjTemp->m_nOuterRect[3]-3)};
        
        //top
        for( int i = 0; i < nOuterRectOuterTemp[2]; ++i ) 
        {
          memcpy( &pFrame_inout->m_BmpBuffer[(nOuterRectOuterTemp[1]*pFrame_inout->getWidth()+nOuterRectOuterTemp[0]+i)*3], 
            Color_Rgb, 
            sizeof(Color_Rgb) );
        }

        //double top line
        memcpy( &pFrame_inout->m_BmpBuffer[(nOuterRectInnerTemp[1]*pFrame_inout->getWidth()+nOuterRectInnerTemp[0])*3],
          &pFrame_inout->m_BmpBuffer[(nOuterRectOuterTemp[1]*pFrame_inout->getWidth()+nOuterRectOuterTemp[0])*3],
          sizeof(Color_Rgb)*nOuterRectInnerTemp[2] );

        //down
        for( int i = 0; i < nOuterRectOuterTemp[2]; ++i ) 
        {
          memcpy( &pFrame_inout->m_BmpBuffer[((nOuterRectOuterTemp[1]+nOuterRectOuterTemp[3])*pFrame_inout->getWidth()+nOuterRectOuterTemp[0]+i)*3], 
            Color_Rgb, 
            sizeof(Color_Rgb) );
        }

        //double down line
        memcpy( &pFrame_inout->m_BmpBuffer[((nOuterRectInnerTemp[1]+nOuterRectInnerTemp[3])*pFrame_inout->getWidth()+nOuterRectInnerTemp[0])*3],
          &pFrame_inout->m_BmpBuffer[((nOuterRectOuterTemp[1]+nOuterRectOuterTemp[3])*pFrame_inout->getWidth()+nOuterRectOuterTemp[0])*3],
          sizeof(Color_Rgb)*nOuterRectInnerTemp[2] );

        //left
        for( int i = 0; i < nOuterRectOuterTemp[3]; ++i ) 
        {
          memcpy( &pFrame_inout->m_BmpBuffer[(((nOuterRectOuterTemp[1]+nOuterRectOuterTemp[3])-i)*pFrame_inout->getWidth()+nOuterRectOuterTemp[0])*3], 
            Color_Rgb, 
            sizeof(Color_Rgb) );
        }

        //right
        for( int i = 0; i < nOuterRectOuterTemp[3]; ++i ) 
        {
          memcpy( &pFrame_inout->m_BmpBuffer[(((nOuterRectOuterTemp[1]+nOuterRectOuterTemp[3])-i)*pFrame_inout->getWidth()+nOuterRectOuterTemp[0]+nOuterRectOuterTemp[2])*3], 
            Color_Rgb, 
            sizeof(Color_Rgb) );
        }

        // double left line
        for( int i = 0; i < nOuterRectOuterTemp[3]; ++i ) 
        {
          memcpy( &pFrame_inout->m_BmpBuffer[(((nOuterRectInnerTemp[1]+nOuterRectInnerTemp[3])-i)*pFrame_inout->getWidth()+nOuterRectInnerTemp[0])*3], 
            Color_Rgb, 
            sizeof(Color_Rgb) );
        }

        // double right line
        for( int i = 0; i < nOuterRectOuterTemp[3]; ++i ) 
        {
          memcpy( &pFrame_inout->m_BmpBuffer[(((nOuterRectInnerTemp[1]+nOuterRectInnerTemp[3])-i)*pFrame_inout->getWidth()+nOuterRectInnerTemp[0]+nOuterRectInnerTemp[2])*3], 
            Color_Rgb, 
            sizeof(Color_Rgb) );
        }
    }
  }
}

#if( 1 == MMXTEST1 )

///////////////////////////////////////////////////////////////////////
/// \brief 
///     detect active object
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
void 
CMatlabFunc::MMXtestInit( void )
{
  MMXtest_dct = &MMXtest_dct_mmx;
}


void 
MMXtest_dct_c  ( int16_t dct[4][4] )
{
  dct[0][0] = 5;
  return;
}

void
MMXtest_dct_mmx2 ( int16_t dct[4][4] )
{
  dct[0][0] = 5;
  return;
}

#endif



///////////////////////////////////////////////////////////////////////
/// \brief 
///     constructor of CObjLabeled
/// \return
///     none
///////////////////////////////////////////////////////////////////////
//RLcodeBufferMinusY      = -j;
//RLcodeBufferSectNum     = 0;  // SectNum
//RLcodeBufferXStart      = i;  // XStart
//RLcodeBufferXLength     = 0;  // XLength
CObjLabeled::CObjLabeled( int16_t const RLcodeMinusY_in, \
                                int16_t const RLcodeSectNum_in, \
                                int16_t const RLcodeXStart_in, \
                                int16_t const RLcodeXLength_in )
                                :m_RLcode( NULL ),
                                m_nRLcodeSize( 4 )
{
  ASSERT( RLcodeMinusY_in  <= 0 );
  ASSERT( RLcodeSectNum_in >= 0 );
  ASSERT( RLcodeXStart_in  >= 0 );
  ASSERT( RLcodeXLength_in >= 0 );

  //===== init m_nOuterRect =====
  m_nOuterRect[0] = RLcodeXStart_in;
  m_nOuterRect[1] = -RLcodeMinusY_in;
  m_nOuterRect[2] = RLcodeXLength_in + 1;
  m_nOuterRect[3] = 1;

  //===== init m_nFirstLineInfo, m_nLastLineInfo =====
  m_nFirstLineInfo[0] = m_nLastLineInfo[0] = RLcodeMinusY_in;
  m_nFirstLineInfo[1] = m_nLastLineInfo[1] = RLcodeSectNum_in;

  //===== init m_nXYDotPlus =====
  m_nXYDotPlus[0] = ((( (RLcodeXStart_in<<1) + RLcodeXLength_in ) * (RLcodeXLength_in + 1) ) >> 1);//xPlus //((( (m_nOuterRect[0]<<1) + m_nOuterRect[2] - 1 ) * m_nOuterRect[2]) >> 1); 
  m_nXYDotPlus[1] = (-RLcodeMinusY_in) * (RLcodeXLength_in + 1);//yPlus //m_nOuterRect[1] * m_nOuterRect[2]; 
  m_nXYDotPlus[2] = RLcodeXLength_in + 1; //dotPlus //m_nOuterRect[2]; 

  //===== init m_RLcode =====
  m_RLcode    = new int16_t [RLHEADSTRIDE+RLSECTIONSTRIDE];
  m_RLcode[0] = RLcodeMinusY_in;
  m_RLcode[1] = RLcodeSectNum_in;
  m_RLcode[2] = RLcodeXStart_in;
  m_RLcode[3] = RLcodeXLength_in;

  m_nBGRPlus[0] = 0;
  m_nBGRPlus[1] = 0;
  m_nBGRPlus[2] = 0;

}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     copy constructor of CObjLabeled
/// \return
///     none
///////////////////////////////////////////////////////////////////////
CObjLabeled::CObjLabeled( CObjLabeled const& obj_in )          // copy constructor
:m_RLcode( NULL ),
m_nRLcodeSize( 0 )
{
  *this = obj_in; //use function overload of CObjLabeled
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     destructor of CObjLabeled
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
CObjLabeled::~CObjLabeled()
{
  SAFEDELETEARRAY( m_RLcode );
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     function overload of CObjLabeled
/// \return
///     new CObjLabeled
///////////////////////////////////////////////////////////////////////
CObjLabeled const& 
CObjLabeled::operator=( CObjLabeled const& obj_in ) 
{
  if( this == &obj_in ) // object assigned to itself
    return *this;          // all done

  // update misc information
  memcpy( m_nXYDotPlus,     obj_in.m_nXYDotPlus,     sizeof(m_nXYDotPlus) );
  memcpy( m_nOuterRect,     obj_in.m_nOuterRect,     sizeof(m_nOuterRect) );
  memcpy( m_nFirstLineInfo, obj_in.m_nFirstLineInfo, sizeof(m_nFirstLineInfo) );
  memcpy( m_nLastLineInfo,  obj_in.m_nLastLineInfo,  sizeof(m_nLastLineInfo) );
  
  // update m_nRLcodeSize
  SAFEDELETEARRAY( m_RLcode );
  if( m_nRLcodeSize )
  {
    m_RLcode = new int16_t [m_nRLcodeSize]; 
    ASSERT( m_RLcode ); 
    memcpy( m_RLcode, obj_in.m_RLcode, m_nRLcodeSize*sizeof(m_RLcode[0]) );
  }

  m_nRLcodeSize = obj_in.m_nRLcodeSize;

  return *this; // return reference to invoking object
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     merge two objects into one object,
///     achieve absolute ordered mergence. 
/// \return
///     new CObjLabeled
///////////////////////////////////////////////////////////////////////
//todo: optimize by using FirstLineInfo and LastLineInfo
uint32_t 
CObjLabeled::mergeRLcodeWith( int16_t const* src_in, uint32_t const srcSize_in )
{
  //RLcoding style: -Y, sectNum, XStart, XLength, XStart, XLength, -Y, sectNum 
  ASSERT( src_in );
  
  uint32_t nRLcodeSize0Temp, nRLcodeSize1Temp, nRLcodeSize2Temp;
  uint32_t nSectNum0, nSectNum1, nSectNum2;
  uint32_t nSectNum0Temp, nSectNum1Temp;
  uint32_t nRLcodeSize0, nRLcodeSize1;
  uint32_t nRLcodeLineLengthTemp;

  // init pRLcode0Temp, pRLcode1Temp, pRLcode2Temp, setup a RLcode cache
  nRLcodeSize0 = m_nRLcodeSize;
  nRLcodeSize1 = srcSize_in;
  int16_t* RLcodeCacheFirst = new int16_t [nRLcodeSize0+nRLcodeSize1]; 
  ASSERT( RLcodeCacheFirst );

  int16_t* const& pRLcode0Temp = m_RLcode;
  const int16_t* const& pRLcode1Temp = src_in;
  int16_t* const& pRLcode2Temp = RLcodeCacheFirst;

  // init pointers and temporary variables
  nRLcodeSize0Temp = 0;
  nRLcodeSize1Temp = 0;
  nRLcodeSize2Temp = 0;

  while( (nRLcodeSize0Temp < nRLcodeSize0) || (nRLcodeSize1Temp < nRLcodeSize1) ) // while code remain in org or src
  {
    if( pRLcode0Temp[nRLcodeSize0Temp] > pRLcode1Temp[nRLcodeSize1Temp] ) // -y0 > -y1, means y0 < y1
    {
      ASSERT( pRLcode0Temp[nRLcodeSize0Temp+1] >= 0 ); // sectNum is nonnegative
      //===== copy y0 to RLcodeCacheFirst =====
      nRLcodeLineLengthTemp = ( (pRLcode0Temp[nRLcodeSize0Temp+1]+1) * RLSECTIONSTRIDE + RLHEADSTRIDE );
      memcpy( &pRLcode2Temp[nRLcodeSize2Temp], &pRLcode0Temp[nRLcodeSize0Temp], nRLcodeLineLengthTemp*sizeof(int16_t) ); //SECTION+1 
      nRLcodeSize2Temp += nRLcodeLineLengthTemp;
      nRLcodeSize0Temp += nRLcodeLineLengthTemp;
    }
    else if( pRLcode0Temp[nRLcodeSize0Temp] < pRLcode1Temp[nRLcodeSize1Temp] ) // -y0 < -y1, means y0 > y1
    {
      ASSERT( pRLcode1Temp[nRLcodeSize1Temp+1] >= 0 ); // sectNum is nonnegative
      //===== copy y1 to RLcodeCacheFirst =====
      nRLcodeLineLengthTemp = ( (pRLcode1Temp[nRLcodeSize1Temp+1]+1) * RLSECTIONSTRIDE + RLHEADSTRIDE );
      memcpy( &pRLcode2Temp[nRLcodeSize2Temp], &pRLcode1Temp[nRLcodeSize1Temp], nRLcodeLineLengthTemp*sizeof(int16_t) ); 
      nRLcodeSize2Temp += nRLcodeLineLengthTemp;
      nRLcodeSize1Temp += nRLcodeLineLengthTemp;
    }
    else // y0 == y1
    {
      pRLcode2Temp[nRLcodeSize2Temp] = (pRLcode0Temp[nRLcodeSize0Temp]); // copy new y0 value to RLcodeCacheFirst
      nRLcodeSize2Temp ++;

      nSectNum0 = pRLcode0Temp[nRLcodeSize0Temp+1] + 1;
      nSectNum1 = pRLcode1Temp[nRLcodeSize1Temp+1] + 1;
      nSectNum2 = nSectNum0 + nSectNum1 - 1; //SECTION

      pRLcode2Temp[nRLcodeSize2Temp] = nSectNum2; //copy new section number
      nRLcodeSize2Temp ++;

      nRLcodeSize0Temp += RLHEADSTRIDE;
      nRLcodeSize1Temp += RLHEADSTRIDE;

      nSectNum0Temp = 0;
      nSectNum1Temp = 0;

      while( (nSectNum0Temp < nSectNum0) || (nSectNum1Temp < nSectNum1) )
      {
        if( pRLcode0Temp[nRLcodeSize0Temp] < pRLcode1Temp[nRLcodeSize1Temp] ) // x0 < x1
        {
          memcpy( &pRLcode2Temp[nRLcodeSize2Temp], &pRLcode0Temp[nRLcodeSize0Temp], RLSECTIONSTRIDE*sizeof(int16_t) );
          nRLcodeSize2Temp += RLSECTIONSTRIDE;
          nRLcodeSize0Temp += RLSECTIONSTRIDE;
          nSectNum0Temp++;
        }
        else if( pRLcode0Temp[nRLcodeSize0Temp] > pRLcode1Temp[nRLcodeSize1Temp] ) // x0 > x1
        {
          memcpy( &pRLcode2Temp[nRLcodeSize2Temp], &pRLcode1Temp[nRLcodeSize1Temp], RLSECTIONSTRIDE*sizeof(int16_t) );
          nRLcodeSize2Temp += RLSECTIONSTRIDE;
          nRLcodeSize1Temp += RLSECTIONSTRIDE;
          nSectNum1Temp++;
        }
        else
        {
          error( "RLcode error!", -1 );
        }

        if( nSectNum0Temp == nSectNum0 )
        {
          // copy remaining y1 and x1 of this section to RLcodeCacheFirst
          nRLcodeLineLengthTemp = (nSectNum1 - nSectNum1Temp)*RLSECTIONSTRIDE;
          memcpy( &pRLcode2Temp[nRLcodeSize2Temp], &pRLcode1Temp[nRLcodeSize1Temp], nRLcodeLineLengthTemp*sizeof(int16_t) );
          nRLcodeSize2Temp += nRLcodeLineLengthTemp;
          nRLcodeSize1Temp += nRLcodeLineLengthTemp;
          nSectNum1Temp += (nSectNum1 - nSectNum1Temp);
        }
        else if( nSectNum1Temp == nSectNum1 )
        {
          // copy remaining y0 and x0 of this section to RLcodeCacheFirst
          nRLcodeLineLengthTemp = (nSectNum0 - nSectNum0Temp)*RLSECTIONSTRIDE;
          memcpy( &pRLcode2Temp[nRLcodeSize2Temp], &pRLcode0Temp[nRLcodeSize0Temp], nRLcodeLineLengthTemp*sizeof(int16_t) );
          nRLcodeSize2Temp += nRLcodeLineLengthTemp;
          nRLcodeSize0Temp += nRLcodeLineLengthTemp;
          nSectNum0Temp += (nSectNum0 - nSectNum0Temp);
        }
      } // while( (nSectNum0Temp < nSectNum0) || (nSectNum1Temp < nSectNum1) )
    }

    if( nRLcodeSize0Temp == nRLcodeSize0 ) 
    {
      // copy remaining y1 and x1 to RLcodeCacheFirst
      nRLcodeLineLengthTemp = (nRLcodeSize1 - nRLcodeSize1Temp);
      memcpy( &pRLcode2Temp[nRLcodeSize2Temp], &pRLcode1Temp[nRLcodeSize1Temp], nRLcodeLineLengthTemp*sizeof(int16_t) );
      nRLcodeSize2Temp += nRLcodeLineLengthTemp;
      nRLcodeSize1Temp += nRLcodeLineLengthTemp;
    }
    else if( nRLcodeSize1Temp == nRLcodeSize1 )
    {
      // copy remaining y0 and x0 to RLcodeCacheFirst
      nRLcodeLineLengthTemp = (nRLcodeSize0 - nRLcodeSize0Temp);
      memcpy( &pRLcode2Temp[nRLcodeSize2Temp], &pRLcode0Temp[nRLcodeSize0Temp], nRLcodeLineLengthTemp*sizeof(int16_t) ); 
      nRLcodeSize2Temp += nRLcodeLineLengthTemp;
      nRLcodeSize0Temp += nRLcodeLineLengthTemp;
    }

  } //while( (nRLcodeSize0Temp < nRLcodeSize0) || (nRLcodeSize1Temp < nRLcodeSize1) )

  // update final 
  int16_t* RLcodeCacheFinal = NULL;
  if( nRLcodeSize0 + nRLcodeSize1 != nRLcodeSize2Temp ) // the different size of input and output RLcode
  {
    RLcodeCacheFinal = new int16_t [nRLcodeSize2Temp]; 
    ASSERT( RLcodeCacheFinal ); 
    memcpy( RLcodeCacheFinal, RLcodeCacheFirst, nRLcodeSize2Temp*sizeof(RLcodeCacheFinal[0]) );
    SAFEDELETEARRAY( RLcodeCacheFirst );

    RLcodeTotalLength+=nRLcodeSize2Temp;
    mergeRLcodeTimes++;
  }
  else
  {
    RLcodeCacheFinal = RLcodeCacheFirst;
  }
  SAFEDELETEARRAY( m_RLcode );
  m_RLcode = RLcodeCacheFinal;
  
  return m_nRLcodeSize = nRLcodeSize2Temp;
}
///////////////////////////////////////////////////////////////////////
/// \brief 
///     merge two objects into one object,
///     achieve absolute ordered mergence. 
/// \return
///     new CObjLabeled
///////////////////////////////////////////////////////////////////////
//todo: optimize by using FirstLineInfo and LastLineInfo
void 
CObjLabeled::mergeWith( CObjLabeled const& obj_in )
{
  if( this == &obj_in ) // object assigned to itself
    return;                // all done

  //===== update m_RLcode =====
  if( 0 == obj_in.m_nRLcodeSize )
  {
    // if NULL input->RLcode, do nothing.
    return;
  }
  else if( 0 == m_nRLcodeSize )
  {
    // if this->RLcode == NULL, copy input->RLcode to this->RLcode.
    m_RLcode = new int16_t [obj_in.m_nRLcodeSize];
    ASSERT( m_RLcode );
    memcpy( m_RLcode, obj_in.m_RLcode, obj_in.m_nRLcodeSize*sizeof(m_RLcode[0]) );
    m_nRLcodeSize = obj_in.m_nRLcodeSize;
  }
  else
  {
    mergeRLcodeWith( obj_in.m_RLcode, obj_in.m_nRLcodeSize );
  }
  
  // update MiscInfo, there is no need to 
  mergeMiscInfoWith( obj_in );
  //RGBPLANE
  mergeColorInfoWith( obj_in );

  return;
}


///////////////////////////////////////////////////////////////////////
/// \brief 
///     push RLcode into this->m_RLcode
///     note the RLcode should indicate the right and down pixels in current and following lines
/// \return
///     the number of pushed RLcode, with sizeof(int16_t*)
///     0 for failure
///     others for success
///////////////////////////////////////////////////////////////////////
//todo: const this function
uint32_t 
CObjLabeled::tailRLcodeBy( int16_t const* src_in, uint32_t const srcSize_in )
{
  ASSERT( src_in );

  uint32_t wrtSizeTemp = 0;
  int16_t* RLcodeTemp = NULL;

  if( NULL == src_in || 0 == srcSize_in )
    return m_nRLcodeSize;

  // verify the incoming code is the defined RLcode
  // we assume the incoming RLcode is with format [-y, sectNum, XStart, XLength...]
  ASSERT( ( src_in[0] <= 0 ) && ( (srcSize_in - RLHEADSTRIDE) % RLSECTIONSTRIDE == 0 ) );
  ASSERT( src_in[0] <= m_nLastLineInfo[0] );

  // (-y_in < -y_org, means y_in > y_org) || ( -y_in == -y_org and there is no code in current object)
  if( src_in[0] < m_nLastLineInfo[0] || ((src_in[0] == m_nLastLineInfo[0]) && !m_nRLcodeSize ) )  
  {

    RLcodeTemp = new int16_t [m_nRLcodeSize+srcSize_in]; 
    ASSERT( RLcodeTemp ); 

    //===== add "src_in" to the tail of "m_RLcode" =====;
    if( m_nRLcodeSize )
    {
      memcpy( RLcodeTemp, m_RLcode, m_nRLcodeSize*sizeof(RLcodeTemp[0]));
    }
    memcpy( &RLcodeTemp[m_nRLcodeSize], src_in, srcSize_in*sizeof(RLcodeTemp[0]));

    //===== update FirstLineInfo and LastLineInfo =====;
    //memcpy( m_nLastLineInfo, src_in, sizeof(m_nLastLineInfo));
    //if( !m_nRLcodeSize ) // for NULL pointer, FirstLineInfo should be equal to m_nLastLineInfo
    //  memcpy( m_nFirstLineInfo, m_nLastLineInfo, sizeof(m_nFirstLineInfo)); // update FirstLineInfo for only one line

    wrtSizeTemp = srcSize_in;

  }
  else //-y_in == -y_org  and  m_nRLcodeSize != 0
  {

    //===== add "src_in" to the tail of "m_RLcode" =====;
    RLcodeTemp = new int16_t [m_nRLcodeSize+srcSize_in-RLHEADSTRIDE]; 
    ASSERT( RLcodeTemp );

    // update last sectNum in m_RCcode;
    m_RLcode[m_nRLcodeSize-(m_nLastLineInfo[1]+1)*RLSECTIONSTRIDE-1] = m_nLastLineInfo[1] + src_in[1] + 1; //sectNum +1

    memcpy( RLcodeTemp,                 m_RLcode,                  m_nRLcodeSize*sizeof(RLcodeTemp[0]) );
    memcpy( &RLcodeTemp[m_nRLcodeSize], &src_in[RLHEADSTRIDE], (srcSize_in-RLHEADSTRIDE)*sizeof(RLcodeTemp[0] ) );

    wrtSizeTemp = srcSize_in - RLHEADSTRIDE;

  }



  // substitute "m_RLcode" with "RLcodeTemp"
  SAFEDELETEARRAY( m_RLcode );
  m_RLcode = RLcodeTemp;

  // update m_nRLcodeSize
  return m_nRLcodeSize += wrtSizeTemp;

}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     push the RLcode of obj_in into current object,
///     achieve absolute ordered mergence. 
/// \return
///     none
///////////////////////////////////////////////////////////////////////
//todo: optimize by using FirstLineInfo and LastLineInfo
void 
CObjLabeled::tailBy( CObjLabeled const& obj_in )
{
  if( this == &obj_in ) // object assigned to itself
    return;                // all done

  

  //todo: should only check "NULL == obj_in.m_RLcode" 
  if( 0 == obj_in.m_nRLcodeSize )
  {
    // if input->RLcode == NULL, do nothing.
    return;
  }
  else if( 0 == m_nRLcodeSize )
  {
    // if this->RLcode == NULL, copy input->RLcode to this->RLcode.
    m_RLcode = new int16_t [obj_in.m_nRLcodeSize];
    ASSERT( m_RLcode );
    memcpy( m_RLcode, obj_in.m_RLcode, obj_in.m_nRLcodeSize*sizeof(m_RLcode[0]) );
    m_nRLcodeSize = obj_in.m_nRLcodeSize;
  }
  else
  {
    tailRLcodeBy( obj_in.m_RLcode, obj_in.m_nRLcodeSize );
  }

  // update misc information after function "tailRLcodeBy"
  mergeMiscInfoWith( obj_in );
  //RGBPLANE
  mergeColorInfoWith( obj_in );
  
  return;
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     merge misc information 
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CObjLabeled::mergeMiscInfoWith( const CObjLabeled &obj_in )
{
  //===== update m_nOuterRect =====
  // do not change the order of following functions !
  uint16_t nRectTemp[4];
  nRectTemp[0] = MIN( m_nOuterRect[0], obj_in.m_nOuterRect[0] );
  nRectTemp[1] = MIN( m_nOuterRect[1], obj_in.m_nOuterRect[1] );
  nRectTemp[2] = MAX( m_nOuterRect[0] + m_nOuterRect[2], obj_in.m_nOuterRect[0] + obj_in.m_nOuterRect[2] ) - nRectTemp[0];
  nRectTemp[3] = MAX( m_nOuterRect[1] + m_nOuterRect[3], obj_in.m_nOuterRect[1] + obj_in.m_nOuterRect[3] ) - nRectTemp[1];
  memcpy( m_nOuterRect, nRectTemp, sizeof(m_nOuterRect));

  //===== update m_nFirstLineInfo, m_nLastLineInfo =====
  /* Note: m_nFirstLineInfo[0] = -y0 */
  // merge section number
  m_nFirstLineInfo[1] = (m_nFirstLineInfo[0] == obj_in.m_nFirstLineInfo[0]) ? ( m_nFirstLineInfo[1] + obj_in.m_nFirstLineInfo[1] + 1 ) :\
    ((m_nFirstLineInfo[0] < obj_in.m_nFirstLineInfo[0]) ? obj_in.m_nFirstLineInfo[1] : m_nFirstLineInfo[1] );

  // merge MinusY
  m_nFirstLineInfo[0] = MAX( m_nFirstLineInfo[0], obj_in.m_nFirstLineInfo[0] );

  // merge section number
  m_nLastLineInfo[1] = (m_nLastLineInfo[0] == obj_in.m_nLastLineInfo[0]) ? (m_nLastLineInfo[1] + obj_in.m_nLastLineInfo[1] + 1 ) :\
    ((m_nLastLineInfo[0] > obj_in.m_nLastLineInfo[0]) ? obj_in.m_nLastLineInfo[1] : m_nLastLineInfo[1] );

  // merge MinusY
  m_nLastLineInfo[0] = MIN( m_nLastLineInfo[0], obj_in.m_nLastLineInfo[0] );

  //===== update m_nXYDotPlus =====
  m_nXYDotPlus[0] += obj_in.m_nXYDotPlus[0]; //xPlus
  m_nXYDotPlus[1] += obj_in.m_nXYDotPlus[1]; //yPlus
  m_nXYDotPlus[2] += obj_in.m_nXYDotPlus[2]; //dotPlus
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     merge color information 
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CObjLabeled::mergeColorInfoWith( const CObjLabeled &obj_in )
{
  //===== update m_nBGRPlus =====
  m_nBGRPlus[0] += obj_in.m_nBGRPlus[0]; //BPlus
  m_nBGRPlus[1] += obj_in.m_nBGRPlus[1]; //GPlus
  m_nBGRPlus[2] += obj_in.m_nBGRPlus[2]; //RPlus
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     update MiscInfo and the end of one section
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void 
CObjLabeled::updateMiscInfoForOneSection( void )
{
  // do not change the order of this updating!

  //===== update m_nOuterRect =====
  m_nOuterRect[2] = m_RLcode[3] + 1; // update width

  /* no need to update m_nFirstLineInfo, m_nLastLineInfo =====*/

  //===== update m_nXYDotPlus =====
  m_nXYDotPlus[0] = ((( (m_nOuterRect[0]<<1) + m_nOuterRect[2] - 1 ) * m_nOuterRect[2]) >> 1); //xPlus
  m_nXYDotPlus[1] = m_nOuterRect[1] * m_nOuterRect[2]; //yPlus
  m_nXYDotPlus[2] = m_nOuterRect[2]; //dotPlus
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     set newAddress to pAddrLine0_inout, pAddrLine1_inout, based on the LastLineInfo in RLcode
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void
CObjLabeled::resetLastTwoAddrLinesByUsingRLcode( CObjLabeled** pAddrLine0_inout, \
                                                CObjLabeled** pAddrLine1_inout, \
                                                int16_t const nAddrLine0MinusJ_in,      \
                                                CObjLabeled* const newAddr_in ) const
{
  // do some verify
  ASSERT( pAddrLine0_inout ); 
  ASSERT( pAddrLine1_inout );
  ASSERT( newAddr_in );
  ASSERT( m_nRLcodeSize != 0 );
  
  int16_t const  nLine0MinusJ = nAddrLine0MinusJ_in;
  int16_t const  nLine1MinusJ = nAddrLine0MinusJ_in + 1;
  
  int16_t const& nFirstLineMinusJ     = m_nFirstLineInfo[0];
  int16_t const& nFirstLineSectNum    = m_nFirstLineInfo[1];
  int16_t const& nLastLineMinusJ      = m_nLastLineInfo[0];
  int16_t const& nLastLineSectNum     = m_nLastLineInfo[1];

  uint32_t nPosTemp = 0;
  uint32_t SearchStartPosTemp = 0;
  uint32_t nRLcodeLineStartPosTemp = 0;

  if( nLastLineMinusJ == nLine1MinusJ )
  {
    nRLcodeLineStartPosTemp = m_nRLcodeSize - ( nLastLineSectNum + 1 ) * RLSECTIONSTRIDE - RLHEADSTRIDE;

    //rewrite AddressLine1
    resetOneAddressLine( pAddrLine1_inout, nLine1MinusJ, nRLcodeLineStartPosTemp, newAddr_in );

  }
  else if( nLastLineMinusJ == nLine0MinusJ )
  {
    nRLcodeLineStartPosTemp = m_nRLcodeSize - ( nLastLineSectNum + 1 ) * RLSECTIONSTRIDE - RLHEADSTRIDE;

    //rewrite AddressLine0
    resetOneAddressLine( pAddrLine0_inout, nLine0MinusJ, nRLcodeLineStartPosTemp, newAddr_in );

    // more than one line in this object
    if( m_nFirstLineInfo[0] != m_nLastLineInfo[0] )
    {
      if( m_nFirstLineInfo[0] == 0 && m_nLastLineInfo[0] == -1 )
      {
        // the first two lines
        nRLcodeLineStartPosTemp = 0;
      }
      else
      {
        /* since there are more than one line in this object, nRLcodeLineStartPosTemp - 1 > 0; */
        // search MinusY of the line before last line
        for( nPosTemp = nRLcodeLineStartPosTemp - 1; nPosTemp > 0; --nPosTemp )
        {
          if( m_RLcode[nPosTemp] < 0 ) // It is safe here because, MinusY == 0 is excluded by the case of "the first two lines"
          {
            // find the latest MinusY?
            nRLcodeLineStartPosTemp = ( m_RLcode[nPosTemp] == nLine1MinusJ ) ? nPosTemp : 0;
            break;
          }
        }

        if( nRLcodeLineStartPosTemp == 0 ) // no matching line
          return;  
      }

      //rewrite AddressLine1
      resetOneAddressLine( pAddrLine1_inout, nLine1MinusJ, nRLcodeLineStartPosTemp, newAddr_in );
    }
    
  }
  else
  {
    error( "nMinusY!=nAddrLine0MinusJ_in && nMinusY!=nLine1MinusJ_in", -1 );
  }

  return;

}


///////////////////////////////////////////////////////////////////////
/// \brief 
///     set newAddress to pLine_inout based on the LastLineInfo in RLcode
/// \return
///     OK for success.
///     ERR for LineMinusY mismatch.
///////////////////////////////////////////////////////////////////////

ErrVal     
CObjLabeled::resetOneAddressLine( CObjLabeled** pLine_inout, int16_t const nLineMinusY_in,\
                                     uint32_t const nRLcodeLineStartPos_in, CObjLabeled* const newAddress_in ) const
{
  // do some verify
  ASSERT( pLine_inout );
  ASSERT( newAddress_in );
  ASSERT( nLineMinusY_in <= 0 );
  ASSERT( m_nRLcodeSize != 0 );

  ASSERT( nRLcodeLineStartPos_in + 1 < m_nRLcodeSize );
  int16_t const& nMinusY     = m_RLcode[nRLcodeLineStartPos_in];
  int16_t const& nSectNum    = m_RLcode[nRLcodeLineStartPos_in+1]+1;
  ASSERT( nRLcodeLineStartPos_in + nSectNum * RLSECTIONSTRIDE + RLHEADSTRIDE <= m_nRLcodeSize );

  // assure line number match
  ASSERT( nMinusY == nLineMinusY_in );

  // rewrite this line
  for( int16_t nSectNumTemp = 0; nSectNumTemp < nSectNum; ++nSectNumTemp )
  {
    uint32_t const   nRLcodeSectionStartPosTemp  = nRLcodeLineStartPos_in + RLHEADSTRIDE + nSectNumTemp * RLSECTIONSTRIDE;
    CObjLabeled** pStartPosTemp   = &pLine_inout[m_RLcode[nRLcodeSectionStartPosTemp]]; //point to start
    int16_t const    nLength         =              m_RLcode[nRLcodeSectionStartPosTemp+1]+1; // get length

    // memset can not be used here.
    for( int16_t nLengthTemp = 0; nLengthTemp < nLength; ++nLengthTemp )
    {
      *( pStartPosTemp++ ) = newAddress_in;
    }
  }

  ROK();
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     this decoding function can be used in None strictly arranged RL coded Objects
/// \return
///     none
///////////////////////////////////////////////////////////////////////
void     
CObjLabeled::decodeRLcodeAndDrawTo( uint8_t* pBuffer_inout,  int16_t const nWidth_in ) const
{
  //do verification
  ASSERT( pBuffer_inout );
  ASSERT( nWidth_in > 0 );

  uint32_t nLineStartPos = 0;
  int16_t  nStartPos     = 0;
  int16_t  nLength       = 0;
  int16_t  nSectNum      = 0;
 
  if( m_nRLcodeSize ) // not zero code 
  {

    for( uint32_t nRLcodePosTemp = 0; nRLcodePosTemp < m_nRLcodeSize; )
    {
      //write one line

      ASSERT( -m_RLcode[nRLcodePosTemp] >= 0 ); // assert MinusY
      nLineStartPos = -m_RLcode[nRLcodePosTemp++] * nWidth_in; //point to line start

      ASSERT( m_RLcode[nRLcodePosTemp] >= 0 );  // assert sectNum
      nSectNum      = m_RLcode[nRLcodePosTemp++]+1;

      for( int16_t nSectNumTemp = 0; nSectNumTemp < nSectNum; ++nSectNumTemp )
      {
        ASSERT( m_RLcode[nRLcodePosTemp] >= 0 );
        nStartPos = m_RLcode[nRLcodePosTemp++]; // Start
        ASSERT( m_RLcode[nRLcodePosTemp] >= 0 );
        nLength   = m_RLcode[nRLcodePosTemp++]+1; // Length

        memset( &pBuffer_inout[nLineStartPos + nStartPos], WHITESPOT, nLength*sizeof(pBuffer_inout[0]));
      }
    }
  }
}



///////////////////////////////////////////////////////////////////////
/// \brief 
///     label object in one frame
///     basic idea: 1. store RLcode for object information.
///                 2. reset address lines to achieve object labeling.
/// \param objDList_inout
///     empty list
/// \param pFrame_RgbtoYBinarized_in
///     binarized frame
/// \param pFrame_Rgb_in
///     original frame to provide RGB information
/// \return
///     OK for success
///     ERR for failure
///////////////////////////////////////////////////////////////////////
ErrVal 
CMatlabFunc::labelObj( CDList< CObjLabeled*, CPointerDNode >* objDList_inout,
                         const CFrameContainer* const pFrame_RgbtoYBinarized_in, 
                         const CFrameContainer* const pFrame_Rgb_in ) const
{

  ASSERT( objDList_inout );
  ASSERT( pFrame_RgbtoYBinarized_in );
  ASSERT( 0 == objDList_inout->GetCount() ); // assure empty list
  
  const uint8_t* const YPlane = pFrame_RgbtoYBinarized_in->m_YuvPlane[0];
  uint16_t const nFrameWidth  = pFrame_RgbtoYBinarized_in->getWidth();
  uint16_t const nFrameHeight = pFrame_RgbtoYBinarized_in->getHeight();

  /* test */
//#define nFrameWidthTest 8
//#define nFrameHeightTest 3
//
//  nFrameWidth = nFrameWidthTest;
//  nFrameHeight = nFrameHeightTest;
  /* test end */

  //RGBPLANE
  uint8_t* RgbPlane = NULL;
  if( pFrame_Rgb_in )
  {
    RgbPlane = pFrame_Rgb_in->m_BmpBuffer;
  }

  uint8_t* colorLine0 = NULL;
  if( pFrame_Rgb_in )
  {
    colorLine0 = new uint8_t [3*nFrameWidth]; 
    ASSERT( colorLine0 );
    memset( colorLine0, NULL, 3*nFrameWidth*sizeof(colorLine0[0]) );
  }


  uint8_t* pixelLine0 = new uint8_t [nFrameWidth]; 
  ASSERT( pixelLine0 );
  memset( pixelLine0, NULL, nFrameWidth * sizeof(pixelLine0[0]) );

  CObjLabeled** objAddressLine0 = new CObjLabeled* [nFrameWidth]; 
  ASSERT( objAddressLine0 );
  CObjLabeled** objAddressLine1 = new CObjLabeled* [nFrameWidth];
  ASSERT( objAddressLine1 );
  CObjLabeled** objectAddressLineTemp = NULL; //<! used to swap objAddressLine0 and objAddressLine1
  CObjLabeled*  pObjectTemp = NULL;
  memset( objAddressLine0, NULL, nFrameWidth*sizeof(objAddressLine0[0]) );
  memset( objAddressLine1, NULL, nFrameWidth*sizeof(objAddressLine1[0]) );

  uint16_t iCurr; //<! current i for merge information search

  bool    bNewObj = FALSE;
  uint8_t nDetectStartPos = 0;
  uint8_t nDetectEndPos = 0;

  /* test */
   
  //uint8_t* pixelLine0test0 = new uint8_t [nFrameWidth*nFrameHeight];
  //for( int itemp = 0; itemp < nFrameWidth*nFrameHeight; ++itemp )
  //{
  //  pixelLine0test0[itemp] = ( rand() > RAND_MAX/2 ) ? 1 : 0;
  //}

  //uint8_t* pixelLine0test0 = new uint8_t [nFrameWidth*nFrameHeight];
  //const uint8_t pixelLine0test0temp[] = { 0,1,1,0,0,0,1,1,\
  //                                        1,0,0,1,0,1,0,0,\
  //                                        0,1,1,0,1,0,1,0};
  //memcpy( pixelLine0test0, pixelLine0test0temp, sizeof(pixelLine0test0temp));

  /* test end */

  CObjLabeled* pObjCurr = NULL;
  CObjLabeled* pObjCurrMerger = NULL;      //<! who will merge this Object

  for( uint16_t j = 0; j < nFrameHeight; ++j )
  {
    // copy pixel values from incoming YuvFrame
    //RGBPLANE
    if( pFrame_Rgb_in )
    {
      memcpy( colorLine0, &RgbPlane[3*j*nFrameWidth], 3*nFrameWidth*sizeof(pixelLine0[0]) );
    }
    

    //===== prepare pixelLine0, pixelLine1, objAddressLine0, objAddressLine1 =====
    // copy pixel values from incoming YuvFrame
    memcpy( pixelLine0, &YPlane[j*nFrameWidth], nFrameWidth*sizeof(pixelLine0[0]) );

    /* test */
    //memcpy( pixelLine0, &pixelLine0test0[j*nFrameWidth], nFrameWidth*sizeof(pixelLine0[0]) );
    /* test end */

    SWAPTWO( objAddressLine0, objAddressLine1, objectAddressLineTemp );

    pObjCurr = NULL;

    for( uint16_t i = 0; i < nFrameWidth; ++i )
    {
      //===== generate current object (pObjCurr) =====
      if( WHITESPOT == pixelLine0[i] ) // white spot
      {
        if( pObjCurr ) // has current object
        {
          bNewObj = FALSE;
          // update RLcode if RLcode is enabled.
          pObjCurr->m_RLcode[3]++; // XLength++;

          //RGBPLANE
          if( pFrame_Rgb_in )
          {
            pObjCurr->m_nBGRPlus[0] += colorLine0[i*3];   //B
            pObjCurr->m_nBGRPlus[1] += colorLine0[i*3+1]; //G
            pObjCurr->m_nBGRPlus[2] += colorLine0[i*3+2]; //R
          }
        }
        else           // no current object
        {
          bNewObj = TRUE;
          pObjCurrMerger = NULL;

          //===== generate current object =====
          pObjCurr = new CObjLabeled( -j, 0, i, 0 ); 

          //RLcodeBufferMinusY      = -j;
          //RLcodeBufferSectNum     = 0;  // SectNum
          //RLcodeBufferXStart      = i;  // XStart
          //RLcodeBufferXLength     = 0;  // XLength
          
          //RGBPLANE
          if( pFrame_Rgb_in )
          {
            pObjCurr->m_nBGRPlus[0] = colorLine0[i*3];   //B
            pObjCurr->m_nBGRPlus[1] = colorLine0[i*3+1]; //G
            pObjCurr->m_nBGRPlus[2] = colorLine0[i*3+2]; //R
          }
        }

        //===== get merge relationship =====
        /* pos:       NONE; L; LU; U; UR;   
           posNum:       0; 1;  2; 3;  4; 5; */
        if( 0==j && 0==i )
        {
          //None
          //nDetectStartPos = 0;
          //nDetectEndPos   = 1;

          /* no need to check none */
          nDetectStartPos = nDetectEndPos = 5;
        }
        else if( 0==j )
        {
          //L;
          //nDetectStartPos = 1;
          //nDetectEndPos   = 2;
          
          /* no need to check left */
          nDetectStartPos = nDetectEndPos = 5;
        }
        else if( 0==i )
        {
          //U;UR;
          nDetectStartPos = 3;
          nDetectEndPos   = 5;

          /* it is impossible for "bNewObj == TRUE" here, thus U and UR should be checked */
          /* Further in the following codes, if U is available, UR will be skipped */
        }
        else if( (nFrameWidth-1)==i )
        {
          //L;LU;U
          //nDetectStartPos = 1;
          //nDetectEndPos   = 4;

          /* no need to check left */
          //LU;U
          nDetectStartPos = 2;
          nDetectEndPos   = 4;
          /* Further in the following codes, if LU is available, U will be skipped */
        }
        else
        {
          //L;LU;U;UR
          //nDetectStartPos = 1;
          //nDetectEndPos   = 5;

          /* different policies for new object and continuous object */
          switch( bNewObj )
          {
          case TRUE:
            //LU;U;UR;
            nDetectStartPos = 2;
            nDetectEndPos   = 5;
            /* Further in the following codes, if LU is available, U will be skipped */
            /* Further in the following codes, if U is available, UR will be skipped */
            break;
          case FALSE:
            //UR;
            nDetectStartPos = 4;
            nDetectEndPos   = 5;
            break;
          }
        }

        for( uint8_t nDetectPos = nDetectStartPos; nDetectPos < nDetectEndPos; ++nDetectPos )
        {
          switch( nDetectPos )
          {
          case 2: //LU
            iCurr = i - 1;
            break;
          case 3: //U
            iCurr = i;
            break;
          case 4: //UR
            iCurr = i + 1;
            break;
          default:
            error( "switch (nDetectPos)", -1 );            
            break;
          }

          // update position

          if( objAddressLine1[iCurr] ) // the reference pixel is white
          {
            if( pObjCurr != objAddressLine1[iCurr] ) // current object does not contain the reference pixel
            {
              if( !pObjCurrMerger ) // ObjCurr has no merger 
              {
                pObjCurrMerger = objAddressLine1[iCurr];
              }
              else // ObjCurr has merger
              {
                if( pObjCurrMerger != objAddressLine1[iCurr] )
                {
                  pObjCurrMerger->mergeWith( *objAddressLine1[iCurr] );
                  pObjectTemp = objAddressLine1[iCurr];
                  objAddressLine1[iCurr]->resetLastTwoAddrLinesByUsingRLcode( objAddressLine0, objAddressLine1, -j, pObjCurrMerger );
                  objDList_inout->DestroyAtAddr( objDList_inout->FindAddrForData( pObjectTemp )); //faster version
                }
              }
            }

            /* if LU is available, U will be skipped */
            /* if U is available, UR will be skipped */
            switch( nDetectPos )
            {
            case 2: //LU
              nDetectPos = MIN( nDetectPos+1, nDetectEndPos );
              break;
            case 3: //U
              nDetectPos = MIN( nDetectPos+1, nDetectEndPos );
              break;
            default:
              break;
            }
          } //if( objAddressLine1[iCurr] ) // the reference pixel is white
        } //for( uint8_t nDetectPos = nDetectStartPos; nDetectPos < nDetectEndPos; ++nDetectPos )
      } //if( WHITESPOT == pixelLine0[i] ) // white spot
      
      // update objAddressLine0 based on the above information
      objAddressLine0[i] = pixelLine0[i] ? pObjCurr : NULL;

      if( BLACKSPOT == pixelLine0[i] || (nFrameWidth-1) == i) // black spot or the line end pixel
      {
        if( pObjCurr )
        {
          pObjCurr->updateMiscInfoForOneSection(); // update MiscInfo before storage or mergence
          // update the merger RLcode or new object RLcode
          if( pObjCurrMerger ) // it is a shadow object
          {
            pObjCurrMerger->tailBy( *pObjCurr ); 
            pObjCurr->resetLastTwoAddrLinesByUsingRLcode(objAddressLine0, objAddressLine1, -j, pObjCurrMerger );
            SAFEDELETE( pObjCurr ); // Delete this pointer 
          }
          else // it is a new object
          {
            objDList_inout->AddTail( pObjCurr );
            pObjCurr = NULL; // Not delete this pointer, since Dlist will be responsible for the memory releasing issue.
          }
        }
      }
    } //for( uint16_t i = 0; i < nFrameWidth; ++i )
  } //for( uint16_t j = 0; j < nFrameHeight; ++j )


  ///* test 1 */
  //uint8_t* pixelLine0test0_drawed = new uint8_t [nFrameWidth*nFrameHeight];
  //memset( pixelLine0test0_drawed, NULL, nFrameWidth*nFrameHeight * sizeof(uint8_t) );
  //for( int dlistcount = 1; dlistcount <= objDList_inout->GetCount(); ++dlistcount )
  //{
  //  objDList_inout->GetAt( dlistcount )->decodeRLcodeAndDrawTo( pixelLine0test0_drawed, nFrameWidth );
  //}

  //for( int itemp = 0; itemp < nFrameHeight*nFrameWidth; ++itemp )
  //{
  //  if( pixelLine0test0_drawed[itemp] != pixelLine0test0[itemp] )
  //  {
  //    cout<<"errorforpixelLine0test0_drawedpixelLine0test0_drawed "<<endl;
  //  }
  //}
  //SAFEDELETEARRAY( pixelLine0test0_drawed );
  ///* test 1 end */

  /* test 2 */
  //CFrameContainer Frame_test(*pFrame_RgbtoYBinarized_in);
  //memset( Frame_test.m_YuvPlane[0], NULL, Frame_test.getYSize()*sizeof(*Frame_test.m_YuvPlane[0]));
  //for( int dlistcount = 1; dlistcount <= objDList_inout->GetCount(); ++dlistcount )
  //{
  //  objDList_inout->GetAt( dlistcount )->decodeRLcodeAndDrawTo( Frame_test.m_YuvPlane[0], nFrameWidth );
  //}

  //for( int itemp = 0; itemp < nFrameHeight*nFrameWidth; ++itemp )
  //{
  //  if( Frame_test.m_YuvPlane[0][itemp] != pFrame_RgbtoYBinarized_in->m_YuvPlane[0][itemp] )
  //  {
  //    cout<<"errorforpixelLine0test0_drawedpixelLine0test0_drawed "<<endl;
  //  }
  //}
  /* test 2 end */

#if !defined (DISABLELOG)
  cout<<objDList_inout->GetCount()<<" objects"<<endl;
#endif

  SAFEDELETEARRAY( pixelLine0 );
  SAFEDELETEARRAY( objAddressLine0 );
  SAFEDELETEARRAY( objAddressLine1 );
  SAFEDELETEARRAY( colorLine0 );
  ROK();

}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     refine obj list to get the upper 25% Rgb color
/// \param objDList_inout
///     the labeled object list with 100% Rgb color
/// \param pFrame_Rgb_in
///     original frame to provide RGB information
/// \return
///     OK for success
///     ERR for failure
///////////////////////////////////////////////////////////////////////
void 
CMatlabFunc::refineObjListRgb( CDList< CObjLabeled*, CPointerDNode >* ObjDList_inout, 
                              const CFrameContainer* const pFrame_Rgb_in ) const
{
  
  int16_t const nFrameYWidth = pFrame_Rgb_in->getWidth();

  //const CFrameContainer* const pFrame_inout = m_pFrame_temp[9];
  for( int nObjNumTemp = 1; nObjNumTemp <= ObjDList_inout->GetCount(); ++nObjNumTemp )
  {
    CObjLabeled* const pObjTemp = ObjDList_inout->GetAt(nObjNumTemp);
    
    // Ystart = y+ 25%height
    int16_t const Ystart = pObjTemp->m_nOuterRect[1]+pObjTemp->m_nOuterRect[3]*2/5;
    // Ystop  = y+ 50%height
    int16_t const Ystop = pObjTemp->m_nOuterRect[1]+pObjTemp->m_nOuterRect[3]/2;

    uint32_t nLineStartPos = 0;
    int16_t  nY            = 0;
    int16_t  nStartPos     = 0;
    int16_t  nLength       = 0;
    int16_t  nSectNum      = 0;

    if( pObjTemp->m_nRLcodeSize ) // not zero code 
    {
      // zero BGR plus
      pObjTemp->m_nBGRPlus[0] = 
        pObjTemp->m_nBGRPlus[1] = 
        pObjTemp->m_nBGRPlus[2] = 0;

      // zero XYDot plus
      pObjTemp->m_nXYDotPlus[0] = 
        pObjTemp->m_nXYDotPlus[1] = 
        pObjTemp->m_nXYDotPlus[2] = 0;
      for( uint32_t nRLcodePosTemp = 0; nRLcodePosTemp < pObjTemp->m_nRLcodeSize; )
      {
        ASSERT( -pObjTemp->m_RLcode[nRLcodePosTemp] >= 0 ); // assert MinusY
        nY            = -pObjTemp->m_RLcode[nRLcodePosTemp];
        nLineStartPos = -pObjTemp->m_RLcode[nRLcodePosTemp++] * nFrameYWidth; //point to line start

        ASSERT( pObjTemp->m_RLcode[nRLcodePosTemp] >= 0 );  // assert sectNum
        nSectNum      = pObjTemp->m_RLcode[nRLcodePosTemp++]+1;

        if( nY <= Ystart )
        {
          //for( int16_t nSectNumTemp = 0; nSectNumTemp < nSectNum; ++nSectNumTemp )
          //{
          //  nRLcodePosTemp+=2;
          //}
          nRLcodePosTemp += nSectNum * RLSECTIONSTRIDE;
        }
        else if( nY >= Ystop )
        {
          //for( int16_t nSectNumTemp = 0; nSectNumTemp < nSectNum; ++nSectNumTemp )
          //{
          //  nRLcodePosTemp+=2;
          //}
          break;
        }
        else
        {
          for( int16_t nSectNumTemp = 0; nSectNumTemp < nSectNum; ++nSectNumTemp )
          {
            ASSERT( pObjTemp->m_RLcode[nRLcodePosTemp] >= 0 );
            nStartPos = pObjTemp->m_RLcode[nRLcodePosTemp++]; // Start
            ASSERT( pObjTemp->m_RLcode[nRLcodePosTemp] >= 0 );
            nLength   = pObjTemp->m_RLcode[nRLcodePosTemp++]+1; // Length

            // accumulate RGB for the specific region
            for( int16_t nTemp = 0; nTemp < nLength; ++nTemp)
            {
              uint32_t const nNewPostemp = nLineStartPos + nStartPos + nTemp;
              pObjTemp->m_nBGRPlus[0] += pFrame_Rgb_in->m_BmpBuffer[ ( nLineStartPos + nStartPos + nTemp )*3 ];
              pObjTemp->m_nBGRPlus[1] += pFrame_Rgb_in->m_BmpBuffer[ ( nLineStartPos + nStartPos + nTemp )*3 +1];
              pObjTemp->m_nBGRPlus[2] += pFrame_Rgb_in->m_BmpBuffer[ ( nLineStartPos + nStartPos + nTemp )*3 +2];
              pObjTemp->m_nXYDotPlus[2]++;
            }
              //pObjTemp->m_nBGRPlus[1] = 
              //pObjTemp->m_nBGRPlus[2] = 0;

            //memset( &pFrame_Rgb_in->m_YuvPlane[0][nLineStartPos + nStartPos], WHITESPOT, nLength*sizeof(pFrame_Rgb_in->m_YuvPlane[0][0]));
          }
        }
      }
    }
  }
  ////do verification
  //ASSERT( pBuffer_inout );
  //ASSERT( nWidth_in > 0 );

  //uint32_t nLineStartPos = 0;
  //int16_t  nStartPos     = 0;
  //int16_t  nLength       = 0;
  //int16_t  nSectNum      = 0;

  //if( m_nRLcodeSize ) // not zero code 
  //{
  //  for( uint32_t nRLcodePosTemp = 0; nRLcodePosTemp < m_nRLcodeSize; )
  //  {
  //    //write one line

  //    ASSERT( -m_RLcode[nRLcodePosTemp] >= 0 ); // assert MinusY
  //    nLineStartPos = -m_RLcode[nRLcodePosTemp++] * nWidth_in; //point to line start

  //    ASSERT( m_RLcode[nRLcodePosTemp] >= 0 );  // assert sectNum
  //    nSectNum      = m_RLcode[nRLcodePosTemp++]+1;

  //    for( int16_t nSectNumTemp = 0; nSectNumTemp < nSectNum; ++nSectNumTemp )
  //    {
  //      ASSERT( m_RLcode[nRLcodePosTemp] >= 0 );
  //      nStartPos = m_RLcode[nRLcodePosTemp++]; // Start
  //      ASSERT( m_RLcode[nRLcodePosTemp] >= 0 );
  //      nLength   = m_RLcode[nRLcodePosTemp++]+1; // Length

  //      memset( &pBuffer_inout[nLineStartPos + nStartPos], WHITESPOT, nLength*sizeof(pBuffer_inout[0]));
  //    }
  //  }
  //}
}




//#endif //#if( 1 = MATLABFUNC )