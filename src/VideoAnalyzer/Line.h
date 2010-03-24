/*! 
*************************************************************************************
* \file 
*    Line.h
* \function：
*     操作人员选择屏幕内无需监控的闭合区域，
*     并将数据保存到OutPutDate和Data.txt中
*
* \note：
*  1、一定保证：MAX=width*height
*  2、通过调整行数(width)和列数(height)来调整划线精度。
*  3、数据输出：Data.txt
*
* \date
*    2009_07_02
* \author
*    - 宋洋                    <soany777@163.com>
*************************************************************************************
*/
// #pragma once//只要在头文件的最开始加入这条指令就能够保证头文件被编译一次

                         
#if !defined  (_LINE_H__21487776_F28C_45b5_9E53_33FC9F1F2F08)
#define _LINE_H__21487776_F28C_45b5_9E53_33FC9F1F2F08
// #undef DEBUG_NEW

class Line
{
public:
  Line(int imWidth , int imHeight);
  ~Line(void);
public:
#define MAX_RGN 100       //标定的不连续区域的最大值。
  int m_Max_Pane_Num;     //小格总数m_Max_Pane_Num=width*height;

  CPoint pt_Pic;          //本次选中的小格中点坐标。
  CPoint pos_pic;         //图像左上角坐标
  CPoint*  Pic_Center;    //记录图像上每个小格的中心坐标
  CPoint*  Pic_Sign;      //记录图像上被选中的每个小格的中心坐标
  CPoint* Pic_Sign_Temp;
  int Pic_sizex,Pic_sizey;        //图像的长宽（像素）
  int width,height;       //图像水平、垂直方向上周期数。
	int PTx,PTy;            //图像水平和垂直周期宽度和高度（像素）
  int index;              //数组元素索引(Pic_Sign中元素的索引)
  int* Cut_Index;         //重新划线时，下一个标定点的索引存放数组、
  int CutNum;             //断点个数
  int next_index;         //存放重新划线时，下一个标定点的索引

  COLORREF  clr_Rgn;
  COLORREF  clr_Point;
  COLORREF  clr_Line;

  bool flag1;//判断是否选中棋盘格区域
  bool flag2;//判断是否选中"设置(fill)"
  bool flag3;//重新划线标记
  bool flag4;//刷新标志
  bool binverse;//! 反选标志
                /*
                binverse = true  : 遮罩模式，用户选择需要遮罩的区域；
                binverse = false : 反选模式，用户选择需要监控的区域；
                */
  bool* OutPutCenterDate; //下标范围[0 ~ m_Max_Pane_Num],对选中小格OutPutCenterDate[i] = 1;未选中小格utPutCenterDate[i] = 0;
  CPoint*  OutPutPicCenterPoint;//存放选中小格的中心坐标，[0 ~ pt_Num]。 暂时没有用到
  int pt_Num; //OutPutPicCenterPoint中，坐标点的个数
  unsigned char*  OutPutDate;//保存最终数据，选中区域置0；非选中区域置1。

public:
  void init_line(int imWidth , int imHeight);
  void destruction_line();
  void ShowPic(CClientDC &dc);//显示图像
  void GetCenterPoint_Pic( );//获取图像中心坐标
  bool GetMinPoint(CPoint b);
  void Draw(CClientDC &dc);//划线
  void DrawChessLine(CClientDC &dc);//在图像画线 //不连续直线的数量：LineNum
  void show(CClientDC &dc);//显示选中区域(填充)
  void GetCutIndex();//重新划线时，获得下一个标定点(断点)的索引
  bool DeletPoint(CPoint b);//删除选中点
  void undo();//撤销，删除Pic_Sign[]最后一个坐标
  void OutPut(CClientDC &dc);//获得输出数据
  void ShowReginInfo(CClientDC &dc,int x0,int y0); //显示参数
  void SaveReginInfo( char* AviFileName); //保存参数
  bool SaveParameterToFile( CString filename); //保存参数
  bool GetParameterToFile( CString filename); //保存参数
  CString GetFilePath(); //! 返回.exe所在目录。D:\\_Dbug\\bin\\1.exe,则返回：D:\\_Dbug\\bin
//   char*  CStringToChar(CString str_src);
  bool GetTemplateParameter(unsigned char*  m_RGB_template,CString parameter_name,int const nYWidth_in, int const nYHeight_in);



//=======================================================================
// public:
//   int nIn_framenum;
//   int nOut_framenum;
//   BOOL flag5;
// CStringA pathName ;
// CStringA fileName ;
// CStringA fileType ;
// 
// 
//   CMjpegStreamFile* InputMjpegStreamFile;
//   CYuvStreamFile* InputYuvStreamFile;
//   CAviStreamFile* InputAviStreamFile;
// 
//   CFramesBuffer* pFramesBuffer;
//   CFrameContainer* pFrame_decoded;
// 
// 
//   void init_pinpic();
//   void des_pinpic();
//   void OpenFile(CClientDC &dc);
//   void DrawBmptoPicture(CClientDC &dc, CFrameContainer* pFrame_in );
//   void ShowVideo(CClientDC &dc);

//=======================================================================

public:
#define X_SizeMax 2000//水平像素最大值
#define Y_SizeMax 1000//垂直像素最大值

  unsigned char	Data[X_SizeMax*Y_SizeMax*4];// 显示专用临时数组
  byte dataR[X_SizeMax][Y_SizeMax];//临时数组
  byte dataG[X_SizeMax][Y_SizeMax];//临时数组
  byte dataB[X_SizeMax][Y_SizeMax];//临时数组
  void GetBMPfromFile(//从BMP文件读数据
    CString Filename,
    byte r[X_SizeMax][Y_SizeMax],
    byte g[X_SizeMax][Y_SizeMax],
    byte b[X_SizeMax][Y_SizeMax],
    int &sizeX,int &sizeY);
  void GetBMPfromFile(//从BMP文件读数据
    CString Filename,
    double r[X_SizeMax][Y_SizeMax],
    double g[X_SizeMax][Y_SizeMax],
    double b[X_SizeMax][Y_SizeMax],
    int &sizeX,int &sizeY);
  bool LoadBMPImage(//从BMP文件得到位图对象
    LPCTSTR sBMPFile,CBitmap& bitmap,CPalette *pPal);
  void DrawPicture(CClientDC &DC,int x0,int y0, // 图像显示
    byte r[X_SizeMax][Y_SizeMax],
    byte g[X_SizeMax][Y_SizeMax],
    byte b[X_SizeMax][Y_SizeMax],
    int SizeX,int SizeY);
  void DrawBitmap(//绘制数组data的图像
    CClientDC &dc,int x0,int y0,int SizeX,int SizeY,unsigned char *data);
  void DrawPicture1(CClientDC &DC,int x0,int y0, // 图像显示
                            unsigned char *m_bmpbfer,
                            int SizeX,int SizeY);
  void DrawPicture2(CClientDC &DC,int x0,int y0, // 图像显示
    unsigned char *m_bmpbfer,
    int SizeX,int SizeY);
  void DrawPicture3(CClientDC &DC,int x0,int y0, // 图像显示
    unsigned char *m_bmpbfer,
    int SizeX,int SizeY);

};
#endif