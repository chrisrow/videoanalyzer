/*! 
*************************************************************************************
* \file 
*    Line.h
* \function��
*     ������Աѡ����Ļ�������صıպ�����
*     �������ݱ��浽OutPutDate��Data.txt��
*
* \note��
*  1��һ����֤��MAX=width*height
*  2��ͨ����������(width)������(height)���������߾��ȡ�
*  3�����������Data.txt
*
* \date
*    2009_07_02
* \author
*    - ����                    <soany777@163.com>
*************************************************************************************
*/
// #pragma once//ֻҪ��ͷ�ļ����ʼ��������ָ����ܹ���֤ͷ�ļ�������һ��

                         
#if !defined  (_LINE_H__21487776_F28C_45b5_9E53_33FC9F1F2F08)
#define _LINE_H__21487776_F28C_45b5_9E53_33FC9F1F2F08
// #undef DEBUG_NEW

class Line
{
public:
  Line(int imWidth , int imHeight);
  ~Line(void);
public:
#define MAX_RGN 100       //�궨�Ĳ�������������ֵ��
  int m_Max_Pane_Num;     //С������m_Max_Pane_Num=width*height;

  CPoint pt_Pic;          //����ѡ�е�С���е����ꡣ
  CPoint pos_pic;         //ͼ�����Ͻ�����
  CPoint*  Pic_Center;    //��¼ͼ����ÿ��С�����������
  CPoint*  Pic_Sign;      //��¼ͼ���ϱ�ѡ�е�ÿ��С�����������
  CPoint* Pic_Sign_Temp;
  int Pic_sizex,Pic_sizey;        //ͼ��ĳ������أ�
  int width,height;       //ͼ��ˮƽ����ֱ��������������
	int PTx,PTy;            //ͼ��ˮƽ�ʹ�ֱ���ڿ�Ⱥ͸߶ȣ����أ�
  int index;              //����Ԫ������(Pic_Sign��Ԫ�ص�����)
  int* Cut_Index;         //���»���ʱ����һ���궨�������������顢
  int CutNum;             //�ϵ����
  int next_index;         //������»���ʱ����һ���궨�������

  COLORREF  clr_Rgn;
  COLORREF  clr_Point;
  COLORREF  clr_Line;

  bool flag1;//�ж��Ƿ�ѡ�����̸�����
  bool flag2;//�ж��Ƿ�ѡ��"����(fill)"
  bool flag3;//���»��߱��
  bool flag4;//ˢ�±�־
  bool binverse;//! ��ѡ��־
                /*
                binverse = true  : ����ģʽ���û�ѡ����Ҫ���ֵ�����
                binverse = false : ��ѡģʽ���û�ѡ����Ҫ��ص�����
                */
  bool* OutPutCenterDate; //�±귶Χ[0 ~ m_Max_Pane_Num],��ѡ��С��OutPutCenterDate[i] = 1;δѡ��С��utPutCenterDate[i] = 0;
  CPoint*  OutPutPicCenterPoint;//���ѡ��С����������꣬[0 ~ pt_Num]�� ��ʱû���õ�
  int pt_Num; //OutPutPicCenterPoint�У������ĸ���
  unsigned char*  OutPutDate;//�����������ݣ�ѡ��������0����ѡ��������1��

public:
  void init_line(int imWidth , int imHeight);
  void destruction_line();
  void ShowPic(CClientDC &dc);//��ʾͼ��
  void GetCenterPoint_Pic( );//��ȡͼ����������
  bool GetMinPoint(CPoint b);
  void Draw(CClientDC &dc);//����
  void DrawChessLine(CClientDC &dc);//��ͼ���� //������ֱ�ߵ�������LineNum
  void show(CClientDC &dc);//��ʾѡ������(���)
  void GetCutIndex();//���»���ʱ�������һ���궨��(�ϵ�)������
  bool DeletPoint(CPoint b);//ɾ��ѡ�е�
  void undo();//������ɾ��Pic_Sign[]���һ������
  void OutPut(CClientDC &dc);//����������
  void ShowReginInfo(CClientDC &dc,int x0,int y0); //��ʾ����
  void SaveReginInfo( char* AviFileName); //�������
  bool SaveParameterToFile( CString filename); //�������
  bool GetParameterToFile( CString filename); //�������
  CString GetFilePath(); //! ����.exe����Ŀ¼��D:\\_Dbug\\bin\\1.exe,�򷵻أ�D:\\_Dbug\\bin
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
#define X_SizeMax 2000//ˮƽ�������ֵ
#define Y_SizeMax 1000//��ֱ�������ֵ

  unsigned char	Data[X_SizeMax*Y_SizeMax*4];// ��ʾר����ʱ����
  byte dataR[X_SizeMax][Y_SizeMax];//��ʱ����
  byte dataG[X_SizeMax][Y_SizeMax];//��ʱ����
  byte dataB[X_SizeMax][Y_SizeMax];//��ʱ����
  void GetBMPfromFile(//��BMP�ļ�������
    CString Filename,
    byte r[X_SizeMax][Y_SizeMax],
    byte g[X_SizeMax][Y_SizeMax],
    byte b[X_SizeMax][Y_SizeMax],
    int &sizeX,int &sizeY);
  void GetBMPfromFile(//��BMP�ļ�������
    CString Filename,
    double r[X_SizeMax][Y_SizeMax],
    double g[X_SizeMax][Y_SizeMax],
    double b[X_SizeMax][Y_SizeMax],
    int &sizeX,int &sizeY);
  bool LoadBMPImage(//��BMP�ļ��õ�λͼ����
    LPCTSTR sBMPFile,CBitmap& bitmap,CPalette *pPal);
  void DrawPicture(CClientDC &DC,int x0,int y0, // ͼ����ʾ
    byte r[X_SizeMax][Y_SizeMax],
    byte g[X_SizeMax][Y_SizeMax],
    byte b[X_SizeMax][Y_SizeMax],
    int SizeX,int SizeY);
  void DrawBitmap(//��������data��ͼ��
    CClientDC &dc,int x0,int y0,int SizeX,int SizeY,unsigned char *data);
  void DrawPicture1(CClientDC &DC,int x0,int y0, // ͼ����ʾ
                            unsigned char *m_bmpbfer,
                            int SizeX,int SizeY);
  void DrawPicture2(CClientDC &DC,int x0,int y0, // ͼ����ʾ
    unsigned char *m_bmpbfer,
    int SizeX,int SizeY);
  void DrawPicture3(CClientDC &DC,int x0,int y0, // ͼ����ʾ
    unsigned char *m_bmpbfer,
    int SizeX,int SizeY);

};
#endif