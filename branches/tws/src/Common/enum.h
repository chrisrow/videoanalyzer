/*! 
*************************************************************************************
* \file 
*    enum.h
* \brief
*    common enum for the whole project
* \date
*    2008_05_12
* \author
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*************************************************************************************
*/

#if !defined  (_enum_H__A3C0DF05_B1CE_469f_BBCA_34339608941C)
#define _enum_H__A3C0DF05_B1CE_469f_BBCA_34339608941C


//! File open method of CYuvStream
enum FILETYPE   
{
  FILETYPE_READ,   //!< use fopen_s(&fileHander, filename, "rb")   
  FILETYPE_WRITE,  //!< use fopen_s(&fileHander, filename, "wb+"), it means read and write
}; 

enum STREAMFILETYPE   
{
  STREAMFILETYPE_YUV,   
  STREAMFILETYPE_AVI,   
  STREAMFILETYPE_MJPG,
  STREAMFILETYPE_UNKNOWN,
}; 

//! Stored image type in CYuvStream
enum YUVTYPE
{
  YUVTYPE_4,    //!< only Y component, supported by yuvFrame, not by yuvStream
  YUVTYPE_420,
  YUVTYPE_422,
  YUVTYPE_444,
};

//! Size of date read or write  
enum YUVCOMP
{
  YUVCOMP_Y,    //!< fread or fwrite YUVCOMP_Y component
  YUVCOMP_U,    //!< fread or fwrite YUVCOMP_U component
  YUVCOMP_V,    //!< fread or fwrite YUVCOMP_V component
  YUVCOMP_YUV,  //!< fread or fwrite YUVCOMP_YUV component
};

enum STREAMSOURCE
{
  STREAMSOURCE_FILE,
  STREAMSOURCE_CAMERA,
};
#endif