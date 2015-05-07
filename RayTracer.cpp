// Win32Project2.cpp : Colors a pixel using setPixel().
//

#include "stdafx.h"
#include "RayTracer.h"

#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<math.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;


#define maxVertex 10000			//maximum number of vertices
#define maxFace 10000			//maximum number of faces
#define SMALL_NUM 0.00000001
#define maxGroup 100
#define La 1.0f			//Light ambient,diffused and specular coefficients.. all preset to 1
#define Ld 1.0f
#define Ls 1.0f
#define MAX_LOADSTRING 100


int xPix;
int yPix;

int **screen;			//stores ray tyhrough which pixel had intersected an object
float ***illu;			//stores illumination related delated for particular pixel on the screen
float eye[3];			//eye location
float light[3];			//light location

//Location of project related files
string folderLoc = "//vmware-host/Shared Folders/Desktop/RaySept21/RaySept21/";
string objFol = "inputs/";
string inputObj = "cube.obj";

//Extra files for storing various parameters
string inpWin = "window.txt";
string inpViw = "view.txt";
string inpPrj = "project.txt";


static HWND sHwnd;		//Windows Handle

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//Functions for Vector Operations
float* crossPr(float*,float*);
float dotPr(float*,float*);
float* addVect(float*,float*);
float* subVect(float*,float*);
float* mulVect(float,float*);

class Scene
{
	private:
		int i;
		int wd;
		int vCount;
		int fCount;
		int gCount;
		int mCount;
		float v[maxVertex][3];
		int f[maxFace][4];	//3 indecies for vertex and last one for group
		string g[maxGroup][2];		//groups
		string mtrl[maxGroup];
		float m[maxGroup][10];	//stores material constants(Ka/Ks/Kd/N) for various groups
		
    
		float vPort[3];
		
		int xWid,yWid;	//World width of screen in terms of unit Co-ordinates 
		
	public:
		void loadObj();
		void initObj();
		void objDetails();
		int interRayTri(float*,int*,float*);		//http://geomalgorithms.com/a06-_intersect-2.html
		void castRay();
		void printScreen();
		void illum(int,int,int,float*);		//illuminate face
        void initGraphics();
      //  void drawObj();
        void drawScene();   //draw the output
		void rstWin();
		void chgCol();
		void sMain();
};


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	Scene s;
	s.sMain();
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_RAYTRACER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RAYTRACER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RAYTRACER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_RAYTRACER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, xPix, yPix + (yPix/7), NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void SetWindowHandle(HWND hwnd)
{
	sHwnd = hwnd;
}

void setPixel(int x, int y, COLORREF& color)
{
	if (sHwnd == NULL)
	{		
		exit(0);
	}
	HDC hdc = GetDC(sHwnd);
	SetPixel(hdc, x, y, color);
	ReleaseDC(sHwnd, hdc);
	return;
}

void drawColor()
{
	COLORREF cl;
	for (int y = 0; y < yPix; y++)
	{
		for (int x = 0; x < xPix; x++)
		{
			if (screen[x][y] > 0)
			{
				cl = RGB(illu[x][y][0]*255.0,illu[x][y][1]*255.0,illu[x][y][2]*255.0);
				setPixel(x, y, cl);
			}
			else
			{
				cl=RGB(22,22,22);
				setPixel(x, y, cl);
			}
		}
	}
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	char *szHello = "SetPixel";
	RECT rt;
	int x = 0, y = 0, n = 0;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		//get the dimensions of the window
		GetClientRect(hWnd, &rt);

		SetWindowHandle(hWnd);
		drawColor();
		
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//Vector Operation definitions
float dotPr(float *vec1,float *vec2)
{
	return ( (vec1[0]*vec2[0]) + (vec1[1]*vec2[1]) + (vec1[2]*vec2[2]) );
}

float* crossPr(float *vec1,float *vec2)
{
	float *ret;
	ret=new float[3];
	ret[0]=(vec1[1]*vec2[2])-(vec1[2]*vec2[1]);
	ret[1]=(vec1[2]*vec2[0])-(vec1[0]*vec2[2]);
	ret[2]=(vec1[0]*vec2[1])-(vec1[1]*vec2[0]);
	return ret;
}

float* addVect(float *vec1,float *vec2)
{
	float *ret;
	ret=new float[3];
	ret[0]=vec1[0]+vec2[0];
	ret[1]=vec1[1]+vec2[1];
	ret[2]=vec1[2]+vec2[2];
	return ret; 
}	

float* subVect(float *vec1,float *vec2)
{
	float *ret;
	ret=new float[3];
	ret[0]=vec1[0]-vec2[0];
	ret[1]=vec1[1]-vec2[1];
	ret[2]=vec1[2]-vec2[2];
	return ret; 
}

float* mulVect(float m,float *vec)
{
	float *ret;
	ret=new float[3];
	ret[0]=vec[0]*m;
	ret[1]=vec[1]*m;
	ret[2]=vec[2]*m;
	return ret;
}


void Scene::sMain()
{
	rstWin();
	initObj();
	
	loadObj();
	castRay();
}

void Scene::initObj()
{
	vCount=0;
	fCount=0;
	gCount=0;
	mCount=0;
	
	screen = new int* [xPix];
	for ( int i=0;i<xPix;i++)
	{
		screen[i] = new int[yPix];
	}
	illu = new float** [xPix];
	for ( int i=0;i<xPix;i++)
	{
		illu[i] = new float* [yPix];

		for ( int j=0;j<yPix;j++)
		{
			illu[i][j] = new float[6];
		}
	}


	int i,j;
	for(i=0;i<xPix;i++)
	{
		for(j=0;j<yPix;j++)
		{
			screen[i][j]=0;
            int k;
            for(k=0;k<10;k++)
            {
                illu[i][j][k]=0;
            }
            
		}
	}

	
}

void Scene::rstWin()
{
	string delimiter = " ";
	string line;

	ifstream winFile(folderLoc + inpWin);

	if ( winFile.is_open() )
	{
		while ( winFile.good() )
		{
			getline(winFile,line);
			string token = line.substr(0, line.find(delimiter));
			if (token[0]=='#')
			{
				//Comments Line
			}
			else if (token == "xDimension")
			{
				xPix = atoi(line.substr(11,line.length()).c_str());
				
			}
			else if (token == "yDimension")
			{
				yPix = atoi(line.substr(11,line.length()).c_str());
			}
		}
	}
	else
	{
		exit (13);
	}
	winFile.close();
	
	ifstream viwFile(folderLoc + inpViw);

	if ( viwFile.is_open() )
	{
		while ( viwFile.good() )
		{
			getline(viwFile,line);
			string token = line.substr(0, line.find(delimiter));
			if (token[0]=='#')
			{
				//Comments Line
			}
			else if (token == "Eye")
			{
				int i,n;
				line=line.substr(4,line.length());
				i=0;
				while(!line.empty())
				{
					n=line.find(delimiter);
					string s;
					s=line.substr(0,n);
					line=line.substr(n+1,line.length());
					eye[i]=atoi(s.c_str());
					i++;
					if( n < 0 )
					{
						break;
					}
				}
			}
			else if (token == "Light")
			{
				int i,n;
				line=line.substr(4,line.length());
				i=0;
				while(!line.empty())
				{
					n=line.find(delimiter);
					string s;
					s=line.substr(0,n);
					line=line.substr(n+1,line.length());
					light[i]=atoi(s.c_str());
					i++;
					if( n < 0 )
					{
						break;
					}
				}
			}
		}
	}
	else
	{
		exit(14);
	}
	viwFile.close();

	ifstream prjFile(folderLoc + inpPrj);

	if ( prjFile.is_open() )
	{
		float z,top,left,bottom,right;
		while ( prjFile.good() )
		{
			getline(prjFile,line);
			string token = line.substr(0, line.find(delimiter));
			if (token[0]=='#')
			{
				//Comments Line
			}
			else if (token == "Z")
			{
				z = atof(line.substr(2,line.length()).c_str());
			}
			else if (token == "Top")
			{
				top = atof(line.substr(4,line.length()).c_str());
			}
			else if (token == "Left")
			{
				left = atof(line.substr(5,line.length()).c_str());
			}
			else if (token == "Bottom")
			{
				bottom = atof(line.substr(7,line.length()).c_str());
			}
			else if (token == "Right")
			{
				right = atof(line.substr(6,line.length()).c_str());
			}
		}
		vPort[0] = left;
		vPort[1] = bottom;
		vPort[2] = z;
		xWid = right - left;
		yWid = top - bottom;
	}
	else
	{
		exit (15);
	}
	prjFile.close();
}

void Scene::loadObj()
{
	string line;
	
    string matFile;
    
    string objFile = folderLoc + objFol + inputObj;
	ifstream myfile (objFile);
	string delimiter = " ";
	if (myfile.is_open())
	{
		string grp;
		while ( myfile.good() )
	    {
			getline(myfile,line);
			string token = line.substr(0, line.find(delimiter));
			if (token[0] == '#' )
			{
				//cout<<"Comments"<<endl;
			}
			else if( token == "mtllib" )
			{
				matFile=folderLoc + objFol + line.substr(7,line.length());
			}
			else if( token == "v" )
			{
				int i,n;
			//	cout<<"Orignal line:"<<line<<endl;
				line=line.substr(2,line.length());
				i=0;
				while(!line.empty())
				{
					
					n=line.find(delimiter);
					string s;
					s=line.substr(0,n);
					//cout<<"New substring :"<<s<<":"<<endl;
					line=line.substr(n+1,line.length());
					v[vCount][i]=atoi(s.c_str());
					//cout<<"\tNew SubStr is:"<<s<<";"<<endl;
						//cout<<"\tNew Line is:"<<line<<";"<<endl;
					i++;
					if( n < 0 )
					{
						//cout<<"Line Ends"<<endl;
						break;
					}
				}
			//	cout<<"Vertex : "<< line << endl;
				vCount++;
			}
			else if( token == "g" )
			{
				line=line.substr(2,line.length());
				g[gCount][0]=line;
				gCount++;
			}
			else if( token == "group" )
			{
				grp=line.substr(6,line.length());
				//cout<<"Group set to:"<<grp<<endl;
			}
			else if( token == "usemtl" )
			{
				//cout<<"G:"<<grp<<" M:"<<line.substr(7,line.length())<<endl;
				line=line.substr(7,line.length());
				
				int i;
				for(i=0;i<gCount;i++)
				{
					if( grp == g[i][0] )
					{
						g[i][1]=line;
						//cout<<"G"<<i<<"-->"<<g[i][1]<<endl;
						break;
					}
					
				}
			}
			else if( token == "f" )
			{
				int i,n;
			//	cout<<"Orignal line:"<<line<<endl;
				line=line.substr(2,line.length());
				i=0;
				while(!line.empty())
				{
					n=line.find(delimiter);
					string s;
					s=line.substr(0,n);
					line=line.substr(n+1,line.length());
					f[fCount][i]=atoi(s.c_str());
					i++;
					if( n < 0 )
					{
						break;
					}
				}
				
		//		cout<<"Face : "<< line << endl;
				f[fCount][3]=gCount;
				fCount++;
			}
	    }
	    myfile.close();
		
		ifstream matF(matFile);
		if (matF.is_open())
		{
			while ( matF.good() )
		    {
				getline(matF,line);
				string token = line.substr(0, line.find(delimiter));
				if (token[0] == '#' )
				{
					//Comments
				}

				else if( token == "newmtl" )
				{
					mtrl[mCount]=line.substr(7,line.length());
					
					mCount++;
				}
				else if( token == "Ka" )
				{
					int i,n;
					
					line=line.substr(3,line.length());
					i=0;
					while(!line.empty())
					{
					
						n=line.find(delimiter);
						string s;
						s=line.substr(0,n);
						line=line.substr(n+1,line.length());
						m[mCount-1][i]=atof(s.c_str());
						i++;
						if( n < 0 )		//Line ends
						{
							break;
						}
					}
					//cout<<endl;
				}
				else if( token == "Kd" )
				{
					int i,n;
					line=line.substr(3,line.length());
					i=3;
					while(!line.empty())
					{
						n=line.find(delimiter);
						string s;
						s=line.substr(0,n);
						line=line.substr(n+1,line.length());
						m[mCount-1][i]=atof(s.c_str());
						i++;
						if( n < 0 )
						{
							break;
						}
					}
				}
				else if( token == "Ks" )
				{
					int i,n;
					line=line.substr(3,line.length());
					i=6;
					while(!line.empty())
					{
						n=line.find(delimiter);
						string s;
						s=line.substr(0,n);
						line=line.substr(n+1,line.length());
						m[mCount-1][i]=atof(s.c_str());
						i++;
						if( n < 0 )
						{
							break;
						}
					}
				}
				else if( token == "N" )
				{
					string s;
					s=line.substr(2,line.length());
					m[mCount-1][9]=atof(s.c_str());
				}
			}
		}
		else
		{
            exit(12);
		}
		
	}
	else
	{
        exit(11);
	}
}

void Scene::castRay()
{	
	int i,j,k;
	for(i=0;i<xPix;i++)
	{
		for(j=0;j<yPix;j++)
		{
			for(k=0;k<fCount;k++)
			{
				float *intPt;
				float *pixel;
				intPt=new float[3];
				pixel=new float[3];
				pixel[0]=vPort[0] + (float)(xWid*i)/xPix;
				pixel[1]=vPort[1] + (float)(yWid*j)/yPix;
				pixel[2]=vPort[2];
				int ret;
				ret=interRayTri(pixel,f[k],intPt);
				
				if(ret==1)	//intersection found
				{
					screen[i][j]=k+1;
					illum(i,j,k,intPt);
					break;
				}
				
			}
		}
	}
}

//    Return: -1 = triangle is degenerate (a segment or point)
//             0 =  disjoint (no intersect)
//             1 =  intersect in unique point I1
//             2 =  are in the same plane
int Scene::interRayTri(float *s,int *f,float *inx)
{

	float *v1,*v2,*v3;
	v1=v[f[0]-1];
	v2=v[f[1]-1];
	v3=v[f[2]-1];

	float *u,*v,*n;
	u=new float[3];
	v=new float[3];
	n=new float[3];
	
	u=subVect(v2,v1);	//V21
	v=subVect(v3,v1);	//V31
	n=crossPr(u,v);
	if(n[0]==0 && n[1]==0 && n[2]==0)
	{
		return -1;
	}
	float *dir,*w0,*w;
	float a,b,r;
	dir=new float[3];
	w0=new float[3];
	w=new float[3];
	
	dir=subVect(s,eye);
	w0=subVect(eye,v1);
	a=-dotPr(n,w0);
	b=dotPr(n,dir);
	if(fabs(b)<SMALL_NUM)
	{
		if(a==0)
		{
			return 2;
		}
		else
		{
			return 0;
		}
	}
	r=a/b;
	if(r<0)
	{
		return 0;
	}
	inx=addVect(eye,mulVect(r,dir));
	float uu,uv,vv,wu,wv,D;
	uu=dotPr(u,u);
	uv=dotPr(u,v);
	vv=dotPr(v,v);
	w=subVect(inx,v1);
	wu=dotPr(w,u);
	wv=dotPr(w,v);
	D=(uv*uv) - (uu*vv);
	
	float s1,t;
	s1=((uv*wv)-(vv*wu))/D;
	if(s1<0||s1>1)
	{
		return 0;
	}
	t=((uv*wu)-(uu*wv))/D;
	if(t<0||(s1+t)>1)
	{
		return 0;
	}
	return 1;
}

void Scene::illum(int x,int y,int fa,float *inxPt)
{
	
	int i;
	for (i=0;i<mCount;i++)
	{
		
		if( g[f[fa][3]][1] == mtrl[i] )
		{
			break;
		}
	}
	float Ka[3],Kd[3],Ks[3];
	float BSr;
	Ka[0]=m[i][0];
	Ka[1]=m[i][1];
	Ka[2]=m[i][2];
	Kd[0]=m[i][3];
	Kd[1]=m[i][4];
	Kd[2]=m[i][5];
	Ks[0]=m[i][6];
	Ks[1]=m[i][7];
	Ks[2]=m[i][8];
	BSr=m[i][9];//*128/1000;
	
	//float amb[3],diffu[3],spec[3];
    
    
    //DIFFUSED
    //D = Ld*Kd([surface_normal].[light_vector])
    //light vector is from surface to light
    
    //cout<<"L-"<<light[0]<<"|"<<inxPt[0]<<endl;
	
    float *lv;      //light vector
    lv = new float[3];
    lv[0]=inxPt[0]-light[0];
    lv[1]=inxPt[1]-light[1];
    lv[2]=inxPt[2]-light[2];
    
    float lvm= sqrt (lv[0]*lv[0] + lv[1]*lv[1] + lv[2]*lv[2]);
    if(lvm == 0)
    {
        lvm=1;
    }
    lv[0] = lv[0]/lvm;
    lv[1] = lv[1]/lvm;
    lv[2] = lv[2]/lvm;
    float *v1,*v2,*v3;  //vertices of faces
    v1=new float[3];
    v2=new float[3];
    v3=new float[3];
    
	v1=v[f[fa][0] - 1];
	v2=v[f[fa][1] - 1];
	v3=v[f[fa][2] - 1];

	float *u,*v,*n;
	u=new float[3];     //edge vector
	v=new float[3];     //edge vextor
	n=new float[3];     //normal vector to surface
	
	u=subVect(v2,v1);	//V21
	v=subVect(v3,v1);	//V31
	n=crossPr(u,v);
    
    float nm;
    nm = sqrt( n[0]*n[0] + n[1]*n[1] + n[2]*n[2] );
    if(nm==0)
    {
        nm=1;
    }
    n[0]=n[0]/nm;
    n[1]=n[1]/nm;
    n[2]=n[2]/nm;
    
    //AMBIENT
    illu[x][y][0]=Ka[0]*La + abs(Kd[0]*Ld*dotPr(n,lv));
    illu[x][y][1]=Ka[1]*La + abs(Kd[1]*Ld*dotPr(n,lv));
    illu[x][y][2]=Ka[2]*La + abs(Kd[2]*Ld*dotPr(n,lv));
	
	//Diffused
//    illu[x][y][3]=Kd[0]*Ld*dotPr(lv,n);
 //   illu[x][y][4]=Kd[1]*Ld*dotPr(lv,n);
 //   illu[x][y][5]=Kd[2]*Ld*dotPr(lv,n);
    
//	illu[x][y][0] = illu[x][y][0] + illu[x][y][3];
//    illu[x][y][1] = illu[x][y][1] + illu[x][y][4];
 //   illu[x][y][2] = illu[x][y][2] + illu[x][y][5];
	
	
	//Specular Reflection
	// S=Ls.Ks.(R.V)^n
	//			R=2N(N.L) - L
	//Ls = Specular Reflectivity of material
	//Ks = Specular Light from source
	//R = Angle of mirror reflection
	//V = Vector from surface to eye
	//n = breadth of sprecular reflection

	float *r;
	r = new float[3];
	r = subVect( mulVect(2*dotPr(n,lv),n) , lv);

	float *vse;
	vse = new float[3];
	vse=subVect(inxPt,eye);
	
	illu[x][y][3]=abs(Ls*Ks[0]*pow(dotPr(r,vse),BSr));
	illu[x][y][4]=abs(Ls*Ks[1]*pow(dotPr(r,vse),BSr));
	illu[x][y][5]=abs(Ls*Ks[2]*pow(dotPr(r,vse),BSr));
	
	//illu[x][y][0] = illu[x][y][0] + illu[x][y][6];
   // illu[x][y][1] = illu[x][y][1] + illu[x][y][7];
   // illu[x][y][2] = illu[x][y][2] + illu[x][y][8];

	//if ( illu[x][y][6] > 1 || illu[x][y][6] < 0)
	//{
	//	exit(illu[x][y][6] + 100);
//	}
	

    if ( illu[x][y][0] > 1 )
    {
        illu[x][y][0] = 1;
    }
    if ( illu[x][y][1] > 1 )
    {
        illu[x][y][1] = 1;
    }
    if ( illu[x][y][2] > 1 )
    {
        illu[x][y][2] = 1;
    }
	
	if ( illu[x][y][0] < 0 )
    {
	//	exit(8);
        illu[x][y][0] = 0;
    }
    if ( illu[x][y][1] < 0 )
    {
		//exit(8);
        illu[x][y][1] = 0;
    }
    if ( illu[x][y][2] < 0 )
    {
		//exit(8);
        illu[x][y][2] = 0;
    }


}