#include <math.h>
#include <alloc.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "fft.h"

HWND hWndMain;
HINSTANCE hInst;
PAINTSTRUCT ps;
DLGPROC lpfnMsgProc;
char eq[512]="",buf[512],exp1[512]="",exp2[512]="";
unsigned numsamples=128;
double start=-2*M_PI,end=2*M_PI,step=(end-start)/numsamples;
int work=0,viewas=0,eqf=0;
extern double _X_;
LOGBRUSH lb;
HBRUSH hbackbrush;
int animon=0;

int get_exp(char *exp,double *result);

class signal
		{
		 public:

		  float *samples;
		  int nsamples,flag;

			signal(void) { nsamples=0; samples=0; flag=0; };
		  ~signal(void) { if (samples) free(samples); };
		  int  AddSample(float r,float i);
		  int  FreeSignal()
				 { if (samples)
						free(samples);
					nsamples=0; samples=0; flag=0;
				 };
		  float SampleLenght(int a)
				  {
					 a<<=1;
					 if (flag)
						 {
						  if (a>=nsamples)
								a-=nsamples;
						  else a=nsamples-a-1;
						  return sqrt(samples[a]*samples[a]+samples[a+1]*samples[a+1]);
						 }
					 else return samples[a];
				  };
		  float SamplePhase(int a)
				  {
					 a<<=1;
					 if (samples[a+1]>-1e-10 && samples[a+1]<1e-10)
						 return 0.0;
					 return atan(samples[a]/samples[a+1])*180.0/M_PI;
				  };
		} sig1,sig2;

int signal::AddSample(float r,float i)
{
	 float *temp;

	 if (samples)
       temp=(float *)realloc(samples,sizeof(float)*(nsamples+1)*2);
	 else temp=(float *)malloc(sizeof(float)*2);

    if (!temp)
		 return 0;

    samples=temp;

    samples[nsamples*2]=r;
    samples[nsamples*2+1]=i;

    nsamples++;

    return 1;
}

int build_signals()
{
	double f;
   int a;
   sig1.FreeSignal();
   sig2.FreeSignal();

   SetDlgItemText(hWndMain,550,"  Building function ...");

   for( a=0;a<numsamples;a++ )
	  {
	   _X_=start+a*step;
			if (get_exp(eq,&f))
		{
		 sig1.AddSample((float)f,0.0);
		 sig2.AddSample((float)f,0.0);
            }
		  }
   SetDlgItemText(hWndMain,550,"  Executing FFT ...");
   four1(sig2.samples-1,sig2.nsamples,1);
   sig2.flag=1;
	SetDlgItemText(hWndMain,550,"");
	return 1;
}

void draw_line(int a,HWND hWnd,int id)
{
     HDC hdc;
     HWND hWndCtrl;
     RECT r;
     float dx;
     int n=numsamples/2;

	  hWndCtrl=GetDlgItem(hWnd,id);
     GetClientRect(hWndCtrl,&r);
     dx=(float)r.right/(numsamples+1);
     hdc=GetDC(hWndCtrl);
     SetROP2(hdc,R2_NOT);
     MoveTo(hdc,(n-a)*dx,0);
     LineTo(hdc,(n-a)*dx,r.bottom);
     MoveTo(hdc,(n+a)*dx,0);
     LineTo(hdc,(n+a)*dx,r.bottom);
	  ReleaseDC(hWndCtrl,hdc);
}

void paint_curve(class signal& sig,HWND hWnd,int id,int eraseflag)
{
	  HDC hd;
	  HPEN hp,hlp;
	  HBRUSH hlb;
	  HRGN hrgn;
	  static HWND hWndCtrl;
	  static RECT r;
	  static float dx,dy,y,s,maxy;
	  int a;

	  if (eraseflag)
		  {
			maxy=0.0;
			if (viewas&&id==107)
			 for( a=0;a<sig.nsamples;a++ )
				  if ( (s=fabs(sig.SamplePhase(a))) >maxy)
					  maxy=s;
				  else ;
			else
			 for( a=0;a<sig.nsamples;a++ )
				  if ( (s=fabs(sig.SampleLenght(a))) >maxy)
					  maxy=s;
				  else ;
			if (maxy==0.0)
			  maxy=1.0;

			hWndCtrl=GetDlgItem(hWnd,id);
			GetClientRect(hWndCtrl,&r);
			dx=(float)r.right/(numsamples+1);
			y=r.bottom/2;
			dy=y/(maxy*1.1);

			sprintf(buf,"%.2lf",maxy);
			SetDlgItemText(hWnd,120+(id-106)*2,buf);
			sprintf(buf,"%.2lf",-maxy);
			SetDlgItemText(hWnd,121+(id-106)*2,buf);
		  }

	  hd=GetDC(hWndCtrl);

	  hrgn = CreateRectRgn(1, 1, r.right-1,r.bottom-1);
	  SelectClipRgn(hd, hrgn);
	  DeleteObject(hrgn);

	  if (eraseflag)
		  {
			hlp=SelectObject(hd,GetStockObject(BLACK_PEN));
			MoveTo(hd,0,y);
			LineTo(hd,r.right,y);
			if (dx>=2.0)
			for( a=1;a<=sig.nsamples;a++ )
			 {
			  MoveTo(hd,a*dx,y-2);
			  LineTo(hd,a*dx,y+3);
			 }
			SelectObject(hd,hlp);
		  }

	  if (eraseflag)
		  {
			hp=CreatePen(PS_SOLID,1,RGB(255,0,0));
			SetROP2(hd,R2_COPYPEN);
		  }
	  else {
			  hp=CreatePen(PS_SOLID,1,RGB(0,0,255));
			  SetROP2(hd,R2_NOT);
			 }
	  hlp=SelectObject(hd,hp);
	  if (viewas&&id==107)
		if (eraseflag)
		  MoveTo(hd,dx,y-dy*sig.SamplePhase(0));
		else MoveTo(hd,dx,y-dy*sig.SamplePhase(0)/sig.nsamples);
	  else
		if (eraseflag)
		  MoveTo(hd,dx,y-dy*sig.SampleLenght(0));
		else MoveTo(hd,dx,y-dy*sig.SampleLenght(0)/sig.nsamples);

	  if (viewas&&id==107)
		if (eraseflag)
		  for( a=1;a<sig.nsamples;a++ )
				 LineTo(hd,(a+1)*dx,y-dy*sig.SamplePhase(a));
		else for( a=1;a<sig.nsamples;a++ )
					LineTo(hd,(a+1)*dx,y-dy*sig.SamplePhase(a)/sig.nsamples);
	  else
		if (eraseflag)
		  for( a=1;a<sig.nsamples;a++ )
				 LineTo(hd,(a+1)*dx,y-dy*sig.SampleLenght(a));
		else for( a=1;a<sig.nsamples;a++ )
					LineTo(hd,(a+1)*dx,y-dy*sig.SampleLenght(a)/sig.nsamples);


     SelectObject(hd,hlp);
     ReleaseDC(hWndCtrl,hd);

     DeleteObject(hp);
}

void set_all_param(HWND hWnd)
{
	  SetDlgItemText(hWnd,101,eq);
	  SetDlgItemText(hWnd,102,exp1);
	  SetDlgItemText(hWnd,103,exp2);
	  SetDlgItemInt(hWnd,104,numsamples,FALSE);
}

int read_all_params(HWND hWnd)
{
	 double f;
	 unsigned u;
	 char str[256];
	 GetDlgItemText(hWnd,101,str,256);
	 if (!get_exp(str,&f))
		 return 0;
	 strcpy(eq,str);
	 if (eqf)
		 {
		  SendDlgItemMessage(hWnd,101,CB_INSERTSTRING,0,(LPARAM)eq);
		  eqf=0;
		 }
	 GetDlgItemText(hWnd,102,str,256);
	 if (get_exp(str,&f))
		 {
		  strcpy(exp1,str);
		  start=f;
		 }
	 else return 0;
	 GetDlgItemText(hWnd,103,str,256);
	 if (get_exp(str,&f))
		 {
		  strcpy(exp2,str);
		  end=f;
		 }
	 else return 0;
	 u=GetDlgItemInt(hWnd,104,NULL,FALSE);
	 if (u==8||u==16||u==32||u==64||u==128||u==256||u==512||u==1024)
	    {
	     numsamples=u;
	     step=(end-start)/numsamples;
	    }
	 else {
		 MessageBox(hWndMain,"Number of Samples must be a power of 2\nbetween 8 and 1024 !","Incorrect Number of Samples",MB_OK|MB_ICONEXCLAMATION);
		 return 0;
		}
	 set_all_param(hWnd);
	 return 1;
}

WORD FAR PASCAL TimerFunc(HWND hWnd,WORD wMsg,int event,DWORD ttime)
{
	  static a,z;
	  static signal sig[2];
	  int c,d,n=sig2.nsamples/2,nn=sig2.nsamples*2-2;
	  float *s;

	  switch(work)
		{
		 case 0:
				sig[0].FreeSignal();
				sig[1].FreeSignal();
				for( c=0;c<sig2.nsamples;c++ )
				  {
					sig[0].AddSample(0.0,0.0);
					sig[1].AddSample(0.0,0.0);
				  }
				paint_curve(sig1,hWndMain,106,1);
				SetDlgItemText(hWndMain,550,"  Adding frequency pairs ...");
				a=0;
				z=0;
				work++;
				break;
		 case 1:
				s=sig[z].samples;
				for( c=0,d=0;c<n;d=((++c)<<1) )
					 if (c<=a)
						 {
						  s[d]=sig2.samples[d];
						  s[nn-d]=sig2.samples[nn-d];
						  s[d+1]=sig2.samples[d+1];
						  s[nn-d+1]=sig2.samples[nn-d+1];
						 }
					 else s[d]=s[d+1]=s[nn-d]=s[nn-d+1]=0.0;
				four1(s-1,sig2.nsamples,-1);
				if (a)
					paint_curve(sig[!z],hWndMain,106,0);
				paint_curve(sig[z],hWndMain,106,0);
				if (a)
					draw_line(a,hWndMain,107);
				z=!z;
				if ((++a)>=n)
					work++;
				else draw_line(a,hWndMain,107);
				break;
		 case 2:
				SetDlgItemText(hWndMain,550,"");
				KillTimer(hWndMain,1);
				SetDlgItemText(hWndMain,140,"&Animate");
				animon=0;
            work=0;
            break;
      }
}

BOOL FAR PASCAL AboutDlgProc(HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
	  switch( Message )
				 {
				  case WM_INITDIALOG:
						 SetDlgItemText(hWnd,50,"[Times,24,200]1D Fast Fourier Transform");
						 break;
              case WM_PAINT:
                   {
                    PAINTSTRUCT ps;
                    BeginPaint(hWnd,&ps);
                    FillRect(ps.hdc,&ps.rcPaint,hbackbrush);
                    EndPaint(hWnd,&ps);
						 }
                   break;
				  case WM_COMMAND:
                   if (wParam==1)
                      EndDialog(hWnd,1);
                   break;
              case WM_CLOSE:
                   EndDialog(hWnd,0);
                   break;
             }
	  return 0;
}

void clearlist(HWND hWnd,int id)
{
	  int a,nsamp=10;
	  char *samp[10]={ "------------ samples",
							 "cos(x)",
							 "cos(x)+cos(2*x)",
							 "cos(x)+cos(2*x)+cos(4*x)",
							 "rnd(2)-1",
							 "abs(x)<1 ? 1 : 0",
							 "sin(x*x)",
							 "sin(x)+cos(x)",
							 "x-(x%1)",
							 "x"
						  };
	  SendDlgItemMessage(hWnd,id,CB_RESETCONTENT,0,0L);
	  for( a=0;a<nsamp;a++ )
			 SendDlgItemMessage(hWnd,101,CB_INSERTSTRING,a,(LPARAM)samp[a]);
	  SetDlgItemText(hWnd,101,eq);
}

BOOL FAR PASCAL  MsgProc( HWND hWnd, UINT Message,
										 WPARAM wParam, LPARAM lParam)
{
 switch(Message)
	{
	 case WM_INITDIALOG:
		  {
			int a,t;
			hWndMain=hWnd;
			lb.lbStyle = BS_PATTERN;
			lb.lbColor = RGB(128,128,128);
			lb.lbHatch = LoadBitmap(hInst,"BackGround");
			hbackbrush = CreateBrushIndirect(&lb);
			DeleteObject(lb.lbHatch);
			viewas=0;
			clearlist(hWnd,101);
			SendDlgItemMessage(hWnd,597,CB_ADDSTRING,0,(LPARAM)(LPCSTR)"View Aplitude");
			SendDlgItemMessage(hWnd,597,CB_ADDSTRING,0,(LPARAM) (LPCSTR)"View Phase");
			SendDlgItemMessage(hWnd,597,CB_SETCURSEL,0,0L);
			GetPrivateProfileString("fft1d","numeq","0",eq,511,"wfft1d.ini");
			sscanf(eq,"%d",&t);
			GetPrivateProfileString("fft1d","samples","128",eq,511,"wfft1d.ini");
			sscanf(eq,"%d",&numsamples);
			GetPrivateProfileString("fft1d","ini","-2*pi",exp1,511,"wfft1d.ini");
			GetPrivateProfileString("fft1d","end","2*pi",exp2,511,"wfft1d.ini");
			for( a=0;a<t;a++ )
				 {
				  sprintf(buf,"%i",a+1);
				  if (GetPrivateProfileString("equations",buf,"",eq,511,"wfft1d.ini"))
					  SendDlgItemMessage(hWnd,101,CB_INSERTSTRING,a,(LPARAM)eq);
				 }
			GetPrivateProfileString("equations","0","cos(x)",eq,511,"wfft1d.ini");
			set_all_param(hWnd);
			build_signals();
		  }
			break;
	 case WM_DESTROY:
			{
			 FILE *fp;
			 int n,a;
			 n=SendDlgItemMessage(hWnd,101,CB_GETCOUNT,0,0L);
			 GetWindowsDirectory(&buf[256],255);
			 sprintf(buf,"%s\\wfft1d.ini",&buf[256]);
			 fp=fopen(buf,"wt");
			 fprintf(fp,"[fft1d]\nnumeq=%i\nini=%s\nend=%s\nsamples=%i\n\n[equations]\n0=%s\n",
						n,exp1,exp2,numsamples,eq);
			 for( a=0;a<n;a++ )
				 {
				  SendDlgItemMessage(hWnd,101,CB_GETLBTEXT,a,(LPARAM)eq);
				  if (!strcmp(eq,"------------ samples"))
                 break;
				  fprintf(fp,"%i=%s\n",a+1,eq);
				 }
			 fclose(fp);
			}
			break;
	 case WM_PAINT:
			BeginPaint(hWnd,&ps);
			EndPaint(hWnd,&ps);
			if (sig1.samples)
				paint_curve(sig1,hWnd,106,1);
			if (sig2.samples)
			   paint_curve(sig2,hWnd,107,1);
			break;
	 case WM_CLOSE:
			PostMessage(hWnd,WM_COMMAND,2,0);
			break;
	 case WM_COMMAND:
			if (animon)
				work=2;
			switch(wParam)
			  {
				case 2: EndDialog(hWnd,0);
						  return TRUE;
				case 140: if (animon)
								  {
									work=2;
								  }
							 else {
									 SetDlgItemText(hWnd,140,"&Stop");
									 SetTimer(hWnd,1,25,(TIMERPROC)TimerFunc);
									 animon=1;
									}
							 break;
				case 108: if (IDYES==MessageBox(hWnd,"Clear all equations in combo box ?","Clear List",MB_YESNO|MB_ICONQUESTION))
								  clearlist(hWnd,101);
							 break;
				case 3:   SetDlgItemText(hWnd,101,eq);
							 break;
				case 101: if (HIWORD(lParam)==CBN_SELCHANGE)
								 {
								  int a;
								  char b[512];
								  a=SendDlgItemMessage(hWnd,101,CB_GETCURSEL,0,0L);
								  SendDlgItemMessage(hWnd,101,CB_GETLBTEXT,a,(LPARAM)b);
								  if (!strcmp(b,"------------ samples"))
									  {

										PostMessage(hWnd,WM_COMMAND,3,0L);
										break;
									  }
								  if (eqf)
									  read_all_params(hWnd);
								  SetDlgItemText(hWnd,101,b);
								  read_all_params(hWnd);
								  build_signals();
								  InvalidateRect(GetDlgItem(hWnd,106),0,0);
								  InvalidateRect(GetDlgItem(hWnd,107),0,0);
								 }
							 else if (HIWORD(lParam)==CBN_EDITCHANGE)
										eqf=1;
							 break;
				case 106: if (sig1.samples)
								 paint_curve(sig1,hWnd,106,1);
							 break;
				case 107: if (sig2.samples)
								 paint_curve(sig2,hWnd,107,1);
							 break;
				case 156: {
							  DLGPROC Dlg;
							  Dlg = (DLGPROC) MakeProcInstance((FARPROC)AboutDlgProc, hInst);
							  DialogBox(hInst, MAKEINTRESOURCE(300), hWndMain, Dlg);
							  FreeProcInstance((FARPROC) Dlg);
							 }
							 break;
				case 597: if (HIWORD(lParam)==CBN_SELCHANGE)
								{
								  viewas=SendDlgItemMessage(hWnd,597,CB_GETCURSEL,0,0L);
								  InvalidateRect(GetDlgItem(hWnd,107),0,1);
								}
							 break;
				case 105:
							 if (read_all_params(hWnd))
								 {
								  SetCursor(LoadCursor(NULL,IDC_WAIT));
								  build_signals();
								  InvalidateRect(GetDlgItem(hWnd,106),0,0);
								  InvalidateRect(GetDlgItem(hWnd,107),0,0);
								  SetCursor(LoadCursor(NULL,IDC_ARROW));
								 }

							 break;
			  }
	   break;
	  case WM_ERASEBKGND:
	    {
			  RECT rect;
	     GetClientRect(hWnd,&rect);
	     FillRect((HDC)wParam,&rect,hbackbrush);
		 }
			return 1;
  }
	return FALSE;
}

int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
 HINSTANCE  hLib;
 hInst=hInstance;

 hLib=LoadLibrary("fpobtn.dll");
 if (!hLib)
   {
    MessageBox(NULL,"Unable to open 'FPOBTN.DLL' !\nThis file must be in the SYSTEM directory.","Library Error",MB_OK|MB_ICONSTOP);
	 return 0;
	}

 lpfnMsgProc = (DLGPROC) MakeProcInstance((FARPROC)MsgProc, hInst);
 DialogBox(hInst, MAKEINTRESOURCE(500), NULL, lpfnMsgProc);
 FreeProcInstance((FARPROC) lpfnMsgProc);

 FreeLibrary(hLib);

 return 1;
}