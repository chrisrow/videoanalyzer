/*!
************************************************************************
* \file
*    Line.cpp
* \brief
*    <<?? 
* \date
*    2008_07_02
* \author
*    - 宋洋                    <soany777@163.com>
************************************************************************
*/

#include "StdAfx.h"
// #include "myinclud.h"
#include "Line.h"
#include "Common/frames.h"


Line::Line(int imWidth , int imHeight)
{
  init_line( imWidth ,  imHeight);
//   init_pinpic();
}

Line::~Line(void)
{
  destruction_line();
//   des_pinpic();
}
void Line::init_line(int imWidth , int imHeight)
{
  Pic_sizex = imWidth;
  Pic_sizey = imHeight;
  pos_pic.x = 0;//初始化图像左上角坐标。
  pos_pic.y = 0;
  PTx = 4;//8
  PTy = 4;//8
  width = Pic_sizex / PTx;
  height = Pic_sizey / PTy;
  m_Max_Pane_Num = width * height;

  index       = 0;
  CutNum      = 0;
  next_index  = 0;
  clr_Rgn     = RGB(250,110,10);
  clr_Point   = RGB(0,255,0) ;
  clr_Line    = RGB(255,0,0);

  flag1    =  false;
  flag2    =  false;
  flag3    =  false;
  flag4    =  false;
  binverse =  true; 

  Pic_Center=new CPoint[m_Max_Pane_Num];
  Pic_Sign=new CPoint[m_Max_Pane_Num];
  Cut_Index = new int [m_Max_Pane_Num];
  OutPutCenterDate=new bool[m_Max_Pane_Num];
  Pic_Sign_Temp = new CPoint [m_Max_Pane_Num];
  OutPutDate = new unsigned char[Pic_sizex*Pic_sizey];
  OutPutPicCenterPoint = NULL;

  memset(Pic_Sign,0,m_Max_Pane_Num*sizeof(CPoint));
  memset(Pic_Center,0,m_Max_Pane_Num*sizeof(CPoint));
  memset(Cut_Index,0,m_Max_Pane_Num*sizeof(int));
  memset(OutPutCenterDate,0,m_Max_Pane_Num*sizeof(char));
  memset(OutPutDate,0,Pic_sizex*Pic_sizey*sizeof(char));
  memset(Pic_Sign_Temp,0,m_Max_Pane_Num*sizeof(CPoint));

}
void Line::destruction_line()
{
  if( Pic_Center )
  {
    delete [] Pic_Center;
    Pic_Center = NULL;
  }
  if( Pic_Sign )
  {
    delete [] Pic_Sign;
    Pic_Sign = NULL;
  }
  if( Cut_Index )
  {
    delete [] Cut_Index;
    Cut_Index = NULL;
  }
  if( OutPutCenterDate )
  {
    delete [] OutPutCenterDate;
    OutPutCenterDate = NULL;
  }
  if( OutPutDate )
  {
    delete [] OutPutDate;
    OutPutDate = NULL;
  }
  if( OutPutPicCenterPoint )
  {
    delete [] OutPutPicCenterPoint;
    OutPutPicCenterPoint = NULL;
  }
  if( Pic_Sign_Temp )
  {
    delete [] Pic_Sign_Temp;
    Pic_Sign_Temp = NULL;
  }
}

// void Line::ShowPic(CClientDC &dc)
// {
//  	GetBMPfromFile(_T("C:\\Documents and Settings\\Administrator\\桌面\\sy\\HumanDetect\\src\\TestGUI\\src\\aaa.bmp"),dataR,dataG,dataB,Pic_sizex,Pic_sizey);//PuDong
//   DrawPicture(dc,pos_pic.x,pos_pic.y,dataR,dataG,dataB,Pic_sizex,Pic_sizey);
// }

void Line::GetCenterPoint_Pic( )//获取图像每个小格的中心坐标
{
  for(int i = 0; i < height; i++)
  {
    for(int j = 0; j < width; j++)
    {
      Pic_Center[j + i * width].x = int(pos_pic.x + (0.5 + j) * PTx);
      Pic_Center[j + i * width].y = int(pos_pic.y + (0.5 + i) * PTy);
    }
  }
}
bool Line::GetMinPoint(CPoint b)
{
  float lmin=(float)(0.5 * (PTx * PTx + PTy * PTy)),l;

  if( (b.x > pos_pic.x + Pic_sizex) || (b.y > pos_pic.y + Pic_sizey) || (b.x < pos_pic.x) || (b.y < pos_pic.y) )
  {
    --index;
    return false;
  }

  for(int i = 0; i < m_Max_Pane_Num ; ++i )
  {
    l = (float)(Pic_Center[i].x - b.x) * (Pic_Center[i].x - b.x) + (Pic_Center[i].y - b.y) * (Pic_Center[i].y - b.y);
    if(l < lmin)
    {
      lmin = l;
      pt_Pic = Pic_Center[i];
    }
  }
  return TRUE;
}

void Line::Draw(CClientDC &dc)//划线
{
  CBrush brush(clr_Point);
  CBrush *pOldBrush=dc.SelectObject(&brush);
  CPoint*  a;
  CPoint*  b;
  a = new CPoint[m_Max_Pane_Num];
  b = new CPoint[m_Max_Pane_Num];
  memcpy(a, Pic_Sign, m_Max_Pane_Num * sizeof(CPoint));
  memcpy(b, Pic_Sign, m_Max_Pane_Num * sizeof(CPoint));
  for(int i = 0; i < index; ++i)
  {
    a[i].x += long(PTx * 0.5);
    a[i].y += long(PTx * 0.5);
    b[i].x -= long(PTy * 0.5);
    b[i].y -= long(PTy * 0.5);//标注矩形的右下、左上坐标
    CRect rect(a[i],b[i]);
    //		dc.Rectangle(rect);
    dc.Ellipse(rect);
  }
  if( a ){delete [] a;a = NULL;}
  if( b ){delete [] b;b = NULL;}

  CPen pen(PS_SOLID, 2, clr_Line);
  CPen *pOldPen = dc.SelectObject(&pen);
  if(index > 0 || flag3)
  {
    DrawChessLine(dc);
  }
  dc.SelectObject(pOldPen);
  pen.DeleteObject();
  dc.SelectObject(pOldBrush);
  brush.DeleteObject();
  if(flag2)
  {
    show(dc);
    flag2 = false;
  }
}

void Line::DrawChessLine(CClientDC &dc)//图像画线
{                                       //不连续直线的数量：LineNum
  if(flag3)
  {
    Pic_Sign[index] = Pic_Sign[index-1];
    ++index;
    flag3 = false;
  }

  for(int i = 0; i < index - 1; ++i)
  {
    dc.MoveTo(Pic_Sign[i]);
    dc.LineTo(Pic_Sign[i+1]);
    for(int j = 0; j < CutNum + 1; ++j)
    {
      if(i+2 == Cut_Index[j])
        ++i;
    }
  }
}

void Line::show(CClientDC &dc)//显示选中区域
{
  /*
  CRgn rgnA;
  rgnA.CreatePolygonRgn( Chess_Sign, index , ALTERNATE);//WINDING
  CBrush brush(clr_Rgn);
  CBrush *pOldBrush=dc.SelectObject(&brush);
  dc.FillRgn(&rgnA,&brush);
  dc.SelectObject(pOldBrush);
  brush.DeleteObject();
  */

  CRgn rgn[MAX_RGN];
  bool fg = false;
  int temp = 0, temp1 = 0;
  memcpy(Pic_Sign_Temp, Pic_Sign, m_Max_Pane_Num * sizeof(CPoint));

  CBrush brush(clr_Rgn);
  CBrush *pOldBrush = dc.SelectObject(&brush);
  for(int i = 0; i <= CutNum; ++i)
  {
    if(0 == Cut_Index[temp])
    {
      if(!fg)
      {
        rgn[i].CreatePolygonRgn(Pic_Sign_Temp,index,ALTERNATE);
        temp1=0;
      }
      else
      {
        for(int j=0;j<m_Max_Pane_Num;j++)
        {
          if(0 == Pic_Sign_Temp[j].x && 0 == Pic_Sign_Temp[j].y)
          {
            temp1=j;
            break;
          }
        }
        rgn[i].CreatePolygonRgn(Pic_Sign_Temp,temp1,ALTERNATE);
      }
    }
    else
    {
      fg = true;
      if(0 == temp)
      {
        rgn[i].CreatePolygonRgn(Pic_Sign_Temp,Cut_Index[temp],ALTERNATE);
        temp1 = Cut_Index[temp];
      }
      else
      {
        temp1 = Cut_Index[temp]-Cut_Index[temp - 1];
        rgn[i].CreatePolygonRgn(Pic_Sign_Temp,temp1,ALTERNATE);
      }
    }
    dc.FillRgn(&rgn[i],&brush);
    for(int j = 0;j < m_Max_Pane_Num;j++)//Cut_Index[temp]+1
    {
      Pic_Sign_Temp[j] = Pic_Sign_Temp[j + temp1];
      if(0 == Pic_Sign_Temp[j + 1].x && 0 == Pic_Sign_Temp[j+1].y)
        break;
    }
    if(0 != Cut_Index[temp])
    {
      temp++;
    }
  }

  dc.SelectObject(pOldBrush);
  brush.DeleteObject();
}
void Line::GetCutIndex()//重新划线时，获得下一个标定点(断点)的索引
{
  for(int i = 0; i < m_Max_Pane_Num; ++i)
  {
    if(Pic_Sign[i].x == 0 || Pic_Sign[i].y == 0)
    {
//       if (0 = index)
//       {
//         init();
//         return;
//       }
      Cut_Index[next_index] = i;
      next_index++;
      break;
    }
  }
  for(int i = 0; i < m_Max_Pane_Num; ++i)//计算断点的个数CutNum
  {
    if(0 == Cut_Index[i])
    {
      CutNum = i;
      break;
    }
  }
}
bool Line::DeletPoint(CPoint b)//删除选中点
{
  int i;
  double lmin=0.5*(PTx*PTx+PTy*PTy),l;
  int temp=m_Max_Pane_Num;
  for( i=0;i<m_Max_Pane_Num;i++)
  {
    if(b.x>pos_pic.x+Pic_sizex||b.y>pos_pic.y+Pic_sizey||b.x<pos_pic.x||b.y<pos_pic.y)
      return FALSE;
    l=(Pic_Center[i].x-b.x)*(Pic_Center[i].x-b.x)+
      (Pic_Center[i].y-b.y)*(Pic_Center[i].y-b.y);
    if(l<lmin)
    {
      lmin=l;
      pt_Pic=Pic_Center[i];
    }
  }
  for(i=0;i<m_Max_Pane_Num;i++)
  {
    if(Pic_Sign[i]==pt_Pic)
    {
      temp=i;
      break;
    }
  }
  if(temp!=m_Max_Pane_Num)
  {
    for(i=temp;i<m_Max_Pane_Num;i++)
    {
      Pic_Sign[i]=Pic_Sign[i+1];
      Pic_Sign[i] = Pic_Sign[i+1];
    }
    index--;
  }
  //for (i = 0; i < CutNum; i++)
  //    Cut_Index[i]-=1;


  return TRUE;
}

void Line::undo()//撤销，删除Pic_Sign[]最后一个坐标
{
  for (int i = 0; i < m_Max_Pane_Num; i++)
  {
    if (0 == index)
    {
      return;
    }
    if (0 == Pic_Sign[i].x && 0 == Pic_Sign[i].y)
    {
      Pic_Sign[i-1].x = 0;
      Pic_Sign[i-1].y = 0;
      index--;
      break;
    }
  }
}

void Line::OutPut(CClientDC &dc)//获得输出数据
{
  int i, j, m,n;
  GetCenterPoint_Pic();
  pt_Num = 0;
  CPoint* temp_pt = new CPoint[m_Max_Pane_Num];
  memset(temp_pt,0,m_Max_Pane_Num*sizeof(CPoint));

  if (binverse)
  {
    for(i=0;i<m_Max_Pane_Num;i++)
    {
      if(clr_Rgn != dc.GetPixel(Pic_Center[i]) && RGB(255,0,0) != dc.GetPixel(Pic_Center[i]))
      {
        OutPutCenterDate[i] = 1;
        temp_pt[pt_Num] = Pic_Center[i];
        pt_Num++;
      }
      else
        OutPutCenterDate[i] = 0;
    }
//
//     for (i = 0; i < pt_Num; i++ )
//     {
//       temp_pt[i] = temp_pt[i] - pos_pic;
//     }
//     if (OutPutPicCenterPoint){delete OutPutPicCenterPoint; OutPutPicCenterPoint = NULL;}
//     OutPutPicCenterPoint = new CPoint[pt_Num];
//     memcpy( OutPutPicCenterPoint, temp_pt, pt_Num*sizeof(CPoint) );
  }
  else
  {
    for(i=0;i<m_Max_Pane_Num;i++)
    {
      if(clr_Rgn != dc.GetPixel(Pic_Center[i]) && RGB(255,0,0) != dc.GetPixel(Pic_Center[i]))
      {
        OutPutCenterDate[i] = 0;
        temp_pt[pt_Num] = Pic_Center[i];
        pt_Num++;
      }
      else
        OutPutCenterDate[i] = 1;
    }
//
//     for (i = 0; i < pt_Num; i++ )
//     {
//       temp_pt[i] = temp_pt[i] - pos_pic;
//     }
//     if (OutPutPicCenterPoint){delete OutPutPicCenterPoint; OutPutPicCenterPoint = NULL;}
//     OutPutPicCenterPoint = new CPoint[pt_Num];
//     memcpy( OutPutPicCenterPoint, temp_pt, pt_Num*sizeof(CPoint) );
  }

    memset(OutPutDate,0,Pic_sizex*Pic_sizey*sizeof(char));
    for (i=0;i<height;i++)
    {
      for (j=0;j<width;j++)
      {
        if (OutPutCenterDate [i*width+j] == 1)
        {
          for (m=0;m<PTy;m++)
          {
            for (n=0;n<PTx;n++)
            {
              OutPutDate[(i*PTy+m)*width*PTx+PTx*j+n] = 200;//调试值：200；实际使用时，将此值置 1.
            }
          }
        }
      }
    }

  if( temp_pt )
  {
    delete [] temp_pt;
    temp_pt = NULL;
  }
}

void Line::ShowReginInfo(CClientDC &dc,int x0,int y0) //显示参数
{
  OutPut(dc);
  DrawPicture3(dc,x0,y0,OutPutDate,Pic_sizex,Pic_sizey);

//   FILE *pFile=fopen("D:\\_Debug\\a","rb");
//   unsigned char* datafromline=new unsigned char[352*288];
//   fread(datafromline,1,352*288,pFile);
//   fclose(pFile);
//   DrawPicture2(dc,x0,y0,datafromline,Pic_sizex,Pic_sizey);

}

void Line::SaveReginInfo( char* AviFileName) /*显示参数*/
{
	FILE *pFile=fopen(AviFileName,"wb+");
	fwrite(OutPutDate,1,Pic_sizex*Pic_sizey,pFile);
	fclose(pFile);
}


bool Line::SaveParameterToFile( CString filename) //保存参数
{
	int state;long size;CFile file;
	state=file.Open(filename,CFile::modeCreate|CFile::modeWrite,NULL);
	if(!state)
	{
		MessageBoxA( NULL, "参数保存失败!", "Line", MB_OK );
		return false;
	}

	size=sizeof(unsigned char);file.Write(OutPutDate,size*Pic_sizex*Pic_sizey);	
	size=sizeof(int);		file.Write(&m_Max_Pane_Num,size);							
	size=sizeof(CPoint);	file.Write(&pt_Pic,size);											
	size=sizeof(CPoint);	file.Write(&pos_pic,size);											
	size=sizeof(CPoint);	file.Write(Pic_Center,size*m_Max_Pane_Num);									
	size=sizeof(CPoint);	file.Write(Pic_Sign,size*m_Max_Pane_Num);										
	size=sizeof(CPoint);	file.Write(Pic_Sign_Temp,size*m_Max_Pane_Num);									
	size=sizeof(int);		file.Write(&Pic_sizex,size);											
	size=sizeof(int);		file.Write(&Pic_sizey,size);											
	size=sizeof(int);		file.Write(&width,size);												
	size=sizeof(int);		file.Write(&height,size);												
	size=sizeof(int);		file.Write(&PTx,size);													
	size=sizeof(int);		file.Write(&PTy,size);													
	size=sizeof(int);		file.Write(&index,size);												
	size=sizeof(int);		file.Write(Cut_Index,size*m_Max_Pane_Num);										
	size=sizeof(int);		file.Write(&CutNum,size);												
	size=sizeof(int);		file.Write(&next_index,size);											
	size=sizeof(COLORREF);	file.Write(&clr_Rgn,size);											
	size=sizeof(bool);		file.Write(&flag1,size);												
	size=sizeof(bool);		file.Write(&flag2,size);												
	size=sizeof(bool);		file.Write(&flag3,size);												
	size=sizeof(bool);		file.Write(&flag4,size);												
	size=sizeof(bool);		file.Write(OutPutCenterDate,size*m_Max_Pane_Num);								
	size=sizeof(int);		file.Write(&pt_Num,size);
// 	size=sizeof(CPoint);	file.Write(OutPutPicCenterPoint,size*m_Max_Pane_Num);						

	file.Close();return true;
}

bool Line::GetParameterToFile( CString filename) //保存参数
{
	int state;long size;CFile file;
	state=file.Open(filename,CFile::modeRead,NULL);
	if(!state)
	{
		MessageBoxA( NULL, "读取参数配置文件失败!", "Line", MB_OK );
//     AfxMessageBox(_T("123"));
		return false;
	}

	size=sizeof(unsigned char);file.Read(OutPutDate,size*Pic_sizex*Pic_sizey );
	size=sizeof(int);		file.Read(&m_Max_Pane_Num,size);							
	size=sizeof(CPoint);	file.Read(&pt_Pic,size);											
	size=sizeof(CPoint);	file.Read(&pos_pic,size);											
	size=sizeof(CPoint);	file.Read(Pic_Center,size*m_Max_Pane_Num);									
	size=sizeof(CPoint);	file.Read(Pic_Sign,size*m_Max_Pane_Num);										
	size=sizeof(CPoint);	file.Read(Pic_Sign_Temp,size*m_Max_Pane_Num);									
	size=sizeof(int);		file.Read(&Pic_sizex,size);											
	size=sizeof(int);		file.Read(&Pic_sizey,size);											
	size=sizeof(int);		file.Read(&width,size);												
	size=sizeof(int);		file.Read(&height,size);												
	size=sizeof(int);		file.Read(&PTx,size);													
	size=sizeof(int);		file.Read(&PTy,size);													
	size=sizeof(int);		file.Read(&index,size);												
	size=sizeof(int);		file.Read(Cut_Index,size*m_Max_Pane_Num);										
	size=sizeof(int);		file.Read(&CutNum,size);												
	size=sizeof(int);		file.Read(&next_index,size);											
	size=sizeof(COLORREF);	file.Read(&clr_Rgn,size);											
	size=sizeof(bool);		file.Read(&flag1,size);												
	size=sizeof(bool);		file.Read(&flag2,size);												
	size=sizeof(bool);		file.Read(&flag3,size);												
	size=sizeof(bool);		file.Read(&flag4,size);												
	size=sizeof(bool);		file.Read(OutPutCenterDate,size*m_Max_Pane_Num);								
	size=sizeof(int);		file.Read(&pt_Num,size);												
// 	size=sizeof(CPoint);	file.Read(OutPutPicCenterPoint,size*m_Max_Pane_Num);

	file.Close();return true;

}


CString Line::GetFilePath() 
{   
  CString m_FilePath;
  GetModuleFileName(NULL,m_FilePath.GetBuffer(MAX_PATH ),MAX_PATH);
  m_FilePath.ReleaseBuffer();
  int m_iPosIndex;
  m_iPosIndex = m_FilePath.ReverseFind('\\'); 
  m_FilePath = m_FilePath.Left(m_iPosIndex);
  return m_FilePath;
}

// char* Line::CStringToChar( CString str_src)
// {
//   wchar_t* wCharString = str_src.GetBuffer(str_src.GetLength()+1); 
//   DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,wCharString,-1,NULL,0,NULL,FALSE);
//   char *char_dest = new char[dwNum];
//   if(!char_dest)
//   {
//     MessageBoxA( NULL, "字符转换失败!", "Line", MB_OK );
//     delete [] char_dest;
//     return NULL;
//   }
//   WideCharToMultiByte (CP_OEMCP,NULL,wCharString,-1,char_dest,dwNum,NULL,FALSE);
//   return char_dest;
// }

bool Line::GetTemplateParameter(unsigned char*  m_RGB_template,CString parameter_name,int const nYWidth_in, int const nYHeight_in)
{
  CString m_FilePath = GetFilePath();//取得应用程序所在文件路径
  m_FilePath += parameter_name;
//   char* filename = CStringToChar(m_FilePath);
  const char* filename = (LPCTSTR)m_FilePath;
  CFileFind finder;//! 查找指定路径下是否有配置文件，若有则读取，若无则根据需要初始化m_RGB_template
  BOOL bWorking = finder.FindFile(m_FilePath);
//   AfxMessageBox(m_FilePath);
  if(bWorking)
  {
    FILE *pFile=fopen(filename,"rb");
    fread(m_RGB_template,1, nYWidth_in * nYHeight_in ,pFile);
    fclose(pFile);
//     SAFEDELETEARRAY(filename);
    return true;
  }
  else
  {
    memset(m_RGB_template,1,nYWidth_in * nYHeight_in);
//     SAFEDELETEARRAY(filename);
    return false;
  }
}

//..........................BMP文件.........................................................
void Line::GetBMPfromFile(//从BMP文件读数据
                             CString Filename,
                             byte r[X_SizeMax][Y_SizeMax],
                             byte g[X_SizeMax][Y_SizeMax],
                             byte b[X_SizeMax][Y_SizeMax],
                             int &sizeX,int &sizeY)
{
  int s;int i,j;
  CBitmap DisplayBitmap;
  s=LoadBMPImage(Filename,DisplayBitmap,NULL);if(s==false)return;
  BITMAP BM;DisplayBitmap.GetObject(sizeof(BM),&BM);//获取Bitmap文件的参数
  sizeX=BM.bmWidth;sizeY=BM.bmHeight;
  DisplayBitmap.GetBitmapBits(sizeX*sizeY*4,Data);
  DisplayBitmap.DeleteObject();
  for(j=0;j<sizeY;j++){for(i=0;i<sizeX;i++){
    b[i][j]=Data[j*4*sizeX+4*i+0];
    g[i][j]=Data[j*4*sizeX+4*i+1];
    r[i][j]=Data[j*4*sizeX+4*i+2];
  }}
}
void Line::GetBMPfromFile(//从BMP文件读数据
                             CString Filename,
                             double r[X_SizeMax][Y_SizeMax],
                             double g[X_SizeMax][Y_SizeMax],
                             double b[X_SizeMax][Y_SizeMax],
                             int &sizeX,int &sizeY)
{
  int s;int i,j;
  CBitmap DisplayBitmap;
  s=LoadBMPImage(Filename,DisplayBitmap,NULL);if(s==false)return;
  BITMAP BM;DisplayBitmap.GetObject(sizeof(BM),&BM);//获取Bitmap文件的参数
  sizeX=BM.bmWidth;sizeY=BM.bmHeight;
  DisplayBitmap.GetBitmapBits(sizeX*sizeY*4,Data);
  DisplayBitmap.DeleteObject();
  for(j=0;j<sizeY;j++){for(i=0;i<sizeX;i++){
    b[i][j]=Data[j*4*sizeX+4*i+0];
    g[i][j]=Data[j*4*sizeX+4*i+1];
    r[i][j]=Data[j*4*sizeX+4*i+2];
  }}
}
//函数：装载位图文件，生成位图GDI对象，用DDB方法显示。
// LoadBMPImage	- 装载BMP位图，创建位图GDI对象同时创建位图调色板
// Returns	- 返回TRUE成功
// sBMPFile	- 全路经BMP文件
// bitmap	- 将被初始化的位图对象
// pPal         - 将被初始化的位图调色板，可为NULL
bool Line::LoadBMPImage(//从BMP文件得到位图对象
                           LPCTSTR sBMPFile,CBitmap& bitmap,CPalette *pPal)
{
  CFile file;if(!file.Open(sBMPFile,CFile::modeRead))return FALSE;
  //读文件头
  BITMAPFILEHEADER bmfHeader;
  if (file.Read((LPSTR)&bmfHeader,sizeof(bmfHeader))!=sizeof(bmfHeader))return FALSE;
  // 文件类型标志是否为‘BM’
  if (bmfHeader.bfType != ((WORD) ('M' << 8) | 'B'))return FALSE;
  //获得文件将需的内存数
  DWORD nPackedDIBLen=(DWORD)file.GetLength() - sizeof(BITMAPFILEHEADER) ;
  HGLOBAL hDIB = ::GlobalAlloc(GMEM_FIXED, nPackedDIBLen);//分配内存
  if (hDIB == 0)return FALSE;
  // 读余下的位图文件
  if (file.Read((LPSTR)hDIB, nPackedDIBLen)!=nPackedDIBLen){  //use Read instead for ReadHuge
    ::GlobalFree(hDIB);return FALSE;}
  file.Close();
  BITMAPINFOHEADER &bmiHeader = *(LPBITMAPINFOHEADER)hDIB ;
  BITMAPINFO &bmInfo = *(LPBITMAPINFO)hDIB ;
  // 位图颜色数
  int nColors=bmiHeader.biClrUsed?bmiHeader.biClrUsed:1<<bmiHeader.biBitCount;
  LPVOID lpDIBBits;
  if( bmInfo.bmiHeader.biBitCount>8){
    lpDIBBits = (LPVOID)((LPDWORD)(bmInfo.bmiColors + bmInfo.bmiHeader.biClrUsed) +
      ((bmInfo.bmiHeader.biCompression == BI_BITFIELDS) ? 3 : 0));}
  else{lpDIBBits = (LPVOID)(bmInfo.bmiColors + nColors);}
  // 创建逻辑调色板
  if(pPal!=NULL){
    if( nColors <= 256 ){
      UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
      LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];
      pLP->palVersion=0x300;
      pLP->palNumEntries=nColors;
      for(int i=0;i<nColors;i++){
        pLP->palPalEntry[i].peRed=bmInfo.bmiColors[i].rgbRed;
        pLP->palPalEntry[i].peGreen=bmInfo.bmiColors[i].rgbGreen;
        pLP->palPalEntry[i].peBlue=bmInfo.bmiColors[i].rgbBlue;
        pLP->palPalEntry[i].peFlags=0;
      }
      pPal->CreatePalette( pLP );delete[] pLP;
    }
  }
  CClientDC dc(NULL);CPalette* pOldPalette=NULL;
  if(pPal){pOldPalette=dc.SelectPalette(pPal,FALSE);dc.RealizePalette();}
  HBITMAP hBmp = CreateDIBitmap( dc.m_hDC,		// 设备句柄
    &bmiHeader,	// pointer to bitmap size and format data
    CBM_INIT,	// initialization flag
    lpDIBBits,	// pointer to initialization data
    &bmInfo,	// pointer to bitmap color-format data
    DIB_RGB_COLORS);		// color-data usage
  bitmap.Attach( hBmp );
  if( pOldPalette ){dc.SelectPalette(pOldPalette,FALSE);}
  ::GlobalFree(hDIB);
  return TRUE;
}
void Line::DrawPicture(CClientDC &DC,int x0,int y0, // 图像显示
                       byte r[X_SizeMax][Y_SizeMax],
                       byte g[X_SizeMax][Y_SizeMax],
                       byte b[X_SizeMax][Y_SizeMax],
                       int SizeX,int SizeY)
{
  int x,y;
  for(y=0;y<SizeY;y++){for(x=0;x<SizeX;x++){
    Data[4*y*SizeX+4*x]=b[x][y];
    Data[4*y*SizeX+4*x+1]=g[x][y];
    Data[4*y*SizeX+4*x+2]=r[x][y];
  }}
  DrawBitmap(DC,x0,y0,SizeX,SizeY,Data);
}

void Line::DrawPicture1(CClientDC &DC,int x0,int y0, // 图像显示
                        unsigned char *m_bmpbfer,
                        int SizeX,int SizeY)
{
  int i,j;
  for(j=0;j<SizeY;j++){for(i=0;i<SizeX;i++){
    Data[4*j*SizeX+4*i]=m_bmpbfer[3*j*SizeX+3*i+2];
    Data[4*j*SizeX+4*i+1]=m_bmpbfer[3*j*SizeX+3*i+1];
    Data[4*j*SizeX+4*i+2]=m_bmpbfer[3*j*SizeX+3*i+0];
  }}
  DrawBitmap(DC,x0,y0,SizeX,SizeY,Data);
}
void Line::DrawPicture2(CClientDC &DC,int x0,int y0, // 图像显示
                        unsigned char *m_bmpbfer,
                        int SizeX,int SizeY)
{
  int i,j;
  for(j=0;j<SizeY;j++){for(i=0;i<SizeX;i++){
    Data[4*j*SizeX+4*i]=m_bmpbfer[ 3*j*SizeX+3*i+0];
    Data[4*j*SizeX+4*i+1]=m_bmpbfer[ 3*j*SizeX+3*i+1];
    Data[4*j*SizeX+4*i+2]=m_bmpbfer[ 3*j*SizeX+3*i+2];
  }}
  DrawBitmap(DC,x0,y0,SizeX,SizeY,Data);
}
void Line::DrawPicture3(CClientDC &DC,int x0,int y0, // 图像显示
                        unsigned char *m_bmpbfer,
                        int SizeX,int SizeY)
{
  int i,j;
  for(j=0;j<SizeY;j++){for(i=0;i<SizeX;i++){
    Data[4*j*SizeX+4*i]=m_bmpbfer[ j*SizeX+i];
    Data[4*j*SizeX+4*i+1]=m_bmpbfer[ j*SizeX+i];
    Data[4*j*SizeX+4*i+2]=m_bmpbfer[ j*SizeX+i];
  }}
  DrawBitmap(DC,x0,y0,SizeX,SizeY,Data);
}

void Line::DrawBitmap(//绘制数组data的图像
                         CClientDC &dc,int x0,int y0,int SizeX,int SizeY,unsigned char *data)
{ 
  CDC dcMemory;CBitmap DisplayBitmap;
  int mBitmap=DisplayBitmap.CreateBitmap(SizeX,SizeY,1,32,data);
  if(!mBitmap){AfxMessageBox(_T("位图分配失败！"));return;};
  dcMemory.CreateCompatibleDC(NULL);
  dcMemory.SelectObject(&DisplayBitmap);
  BITMAP BM;DisplayBitmap.GetObject(sizeof(BM),&BM);
  dc.BitBlt(x0,y0,BM.bmWidth,BM.bmHeight,&dcMemory,0,0,SRCCOPY);
  dcMemory.DeleteDC();DisplayBitmap.DeleteObject();
}


