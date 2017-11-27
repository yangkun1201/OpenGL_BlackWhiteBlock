/*
*		This Code Was Created By Jeff Molofee 2000
*		A HUGE Thanks To Fredric Echols For Cleaning Up
*		And Optimizing This Code, Making It More Flexible!
*		If You've Found This Code Useful, Please Let Me Know.
*		Visit My Site At nehe.gamedev.net
*/

#include <windows.h>		// Header File For Windows
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <gl\glaux.h>		// Header File For The Glaux Library
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <gl\fmod.h>
#include <list>
using namespace std;

HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application
BOOL	done = FALSE;
bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default
GLuint base;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
int DrawGLScene(GLvoid);
bool live = true;
int score = 0;
bool isBegin = false;
GLfloat mx,my;
FSOUND_STREAM	*sound[3];

int ParticelNumber = 1000;
GLfloat movex = 0.0;

typedef struct {
	bool active;
	GLfloat life;
	GLfloat fade;
	GLfloat r, g, b;
	GLfloat x, y, z;
	GLfloat xi, yi;
	GLfloat xg, yg;
}Particle;

list<Particle> particleList;

void CALLBACK TimerProc(HWND hwnd,
	UINT message,
	UINT idTimer,
	DWORD dwTime)
{
	movex += 5.0;
	if (movex == 140)
	{
		isBegin = true;
	}
}

static GLfloat colors[12][3] =				// 彩虹颜色
{
	{ 1.0f,0.5f,0.5f },{ 1.0f,0.75f,0.5f },{ 1.0f,1.0f,0.5f },{ 0.75f,1.0f,0.5f },
	{ 0.5f,1.0f,0.5f },{ 0.5f,1.0f,0.75f },{ 0.5f,1.0f,1.0f },{ 0.5f,0.75f,1.0f },
	{ 0.5f,0.5f,1.0f },{ 0.75f,0.5f,1.0f },{ 1.0f,0.5f,1.0f },{ 1.0f,0.5f,0.75f }
};

void initParticle(Particle &p)
{
	p.active = true;
	p.life = 1.0;
	p.fade = float(rand() % 100) / 30.0f;
	p.x = -60.0;
	p.y = -50.0;
	p.z = 0.0;
	p.r = colors[rand() % 12][0];
	p.g = colors[rand() % 12][1];
	p.b = colors[rand() % 12][0];
	p.xi = -1 * float((rand() % 10));
	p.xg = 0.01;
	p.yi = float((rand() % 1) - 1.0f);
	p.yg = 0.001;
}
void createParticle()
{
	particleList.clear();
	for (int i = 0;i < ParticelNumber;i++)
	{
		Particle p;
		initParticle(p);
		particleList.push_back(p);
	}
}
void drawParticle()
{
	list<Particle>::iterator it;
	glTranslatef(movex, 0.0, 0.0);
	for (it = particleList.begin();it != particleList.end();it++)
	{
		if (it->active)
		{
			glColor4f(it->r, it->g, it->b, it->life);
			//glColor4f(1.0, 1.0, 1.0, it->life);
			glBegin(GL_QUADS);
			glVertex2d(it->x, it->y);
			glVertex2d(it->x, it->y + 0.7);
			glVertex2d(it->x + 0.7, it->y + 0.7);
			glVertex2d(it->x + 0.7, it->y);
			glEnd();
			it->x += it->xi;
			it->y += it->yi;
			it->life -= it->fade;
			if (it->life < 0)
			{
				it->life = 1;
				it->x = -60.0;
				it->y = -50.0;
			}
		}
	}

}


void CreateSound(char name[80], int num)
{
	sound[num] = FSOUND_Stream_OpenFile(name, FSOUND_HW3D || FSOUND_LOOP_OFF, 0);
}
FSOUND_SAMPLE* handle;
void InitSound(void)
{
	//FMUSIC_MODULE *mod = NULL;
	
	FSOUND_Init(32000, 64, 0);
	handle = FSOUND_Sample_Load(0,"background.mp3",0,0);
	FSOUND_PlaySound(0,handle);
	//CreateSound("background.wma", 0);
	CreateSound("click.wav", 1);
	
}

void BindSound(int num)
{
	FSOUND_Stream_Play(FSOUND_FREE, sound[num]);
}

GLvoid BuildFont(GLvoid)
{
	HFONT font;
	HFONT oldfont;
	base = glGenLists(96);
	font = CreateFont(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, L"Courier New");
	oldfont = (HFONT)SelectObject(hDC, font);
	wglUseFontBitmaps(hDC, 32, 96, base);
	SelectObject(hDC, oldfont);
	DeleteObject(font);
}

GLvoid KillFont(GLvoid)
{
	glDeleteLists(base, 96);
}

GLvoid glPrint(const char *fmt, ...)
{
	char text[256];
	va_list ap;
	if (fmt == NULL)
	{
		return;
	}
	va_start(ap, fmt);
	vsprintf(text, fmt, ap);
	va_end(ap);
	glPushAttrib(GL_LIST_BIT);
	glListBase(base - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}


typedef struct {
	GLfloat x, y;
	GLfloat width, height;
	GLfloat r, g, b;
	bool clicked;
	int d;
}Block;

void arrivedTest(Block &b);
const int BlockNumber = 400;
Block blocks[BlockNumber / 4][4];

void initBlock()
{
	for (int i = 0; i < BlockNumber / 4; i++)
	{
		int n = rand() % 4;
		for (int j = 0; j < 4; j++)
		{
			blocks[i][j].width = 20.0;
			blocks[i][j].height = 20.0;
			blocks[i][j].x = -55 + j * 30;
			blocks[i][j].y = -70 + i * 30;
			blocks[i][j].clicked = false;
			blocks[i][j].d = j;
			if (j == n)
			{
				blocks[i][j].r = blocks[i][j].g = blocks[i][j].b = 0.0;
			}
			else {
				blocks[i][j].r = blocks[i][j].g = blocks[i][j].b = 1.0;
			}
		}
	}

}

void arrivedTest(Block &b)
{
	if (live == true && b.r == 0.0 && b.clicked == false)
	{
		if (b.y + b.height < -80.0)  //黑块位置 <-80，失败
		{
			if (MessageBox(hWnd, L"game over!", L"information", MB_OKCANCEL | MB_ICONERROR) == IDOK)
			{
				live = false;
			}
			else {
				exit(0);
			}
		}
		else if (b.y <= -80.0 && b.y + b.height >= -80.0) //黑块位置在-80之间，判断对应按键是否按下
		{
			if (keys['F'] && b.d == 0 || keys['G'] && b.d == 1 || keys['J'] && b.d == 2 || keys['K'] && b.d == 3)
			{

				b.clicked = true;
				score++;
				BindSound(1);
				b.r = b.g = b.b = 1.0;
				
			}
		}

	}
}

void drawBlock()
{
	glClearColor(139.0 / 255.0f, 248.0 / 255.0f, 249.0 / 255.0f, 0.5f);
	for (int i = 0; i < BlockNumber / 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			glColor3f(blocks[i][j].r, blocks[i][j].g, blocks[i][j].b);
			glBegin(GL_QUADS);
			glVertex2f(blocks[i][j].x, blocks[i][j].y);
			glVertex2f(blocks[i][j].x, blocks[i][j].y + blocks[i][j].height);
			glVertex2f(blocks[i][j].x + blocks[i][j].width, blocks[i][j].y + blocks[i][j].height);
			glVertex2f(blocks[i][j].x + blocks[i][j].width, blocks[i][j].y);
			glEnd();
			if (live)
			{
				blocks[i][j].y -= 0.03;
				
			}
			arrivedTest(blocks[i][j]);
		}
	}
	glColor3f(0.0, 0.0, 0.0);
	glRasterPos2f(-80.0, 70.0);
	glPrint("score:%d", score);
}

void MenuTest()
{
	mx = (mx - 400)*80.0 / 800;
	my = (my - 300)*80.0 / 600;
	if (mx >= -12.0&&mx <= 12.0&&my >= -8.0&&my <= 8.0)
	{
		SetTimer(NULL, 1, 80, TimerProc);
	}
}

void drawMenu()
{
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(-12,-8);
		glVertex2f(-12, 8);
		glVertex2f(12, 8);
		glVertex2f(12, -8);
	glEnd();
	glRasterPos2f(-7, -2);
	glPrint("Begin");
	drawParticle();
	MenuTest();
}


GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}
	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

														// Calculate The Aspect Ratio Of The Window
																											//gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	gluOrtho2D(-80.0, 80.0, -80.0, 80.0);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	//BindSound(0);
	createParticle();
	BuildFont();
	InitSound();
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	//glClearColor(139.0 / 255.0f, 248.0 / 255.0f, 249.0 / 255.0f, 0.5f);				// Background
	glClearColor(0.0,0.0,0.0,0.5);
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	initBlock();
	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();
	// Reset The Current Modelview Matrix
	if (isBegin)
	{
		drawBlock();
	} else{
		drawMenu();
	}
	
	
	
	return TRUE;										// Everything Went OK
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	KillFont();
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, L"Release Of DC And RC Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, L"Release Rendering Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, L"Release Device Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, L"Could Not Release hWnd.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass(L"OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, L"Could Not Unregister Class.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(LPCWSTR title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = L"OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, L"Failed To Register The Window Class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, L"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", L"NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, L"Program Will Now Close.", L"ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

																	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		L"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Create A GL Device Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Find A Suitable PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Set The PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Create A GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Activate The GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Initialization Failed.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
	{
		if (!HIWORD(wParam))					// Check Minimization State
		{
			active = TRUE;						// Program Is Active
		}
		else
		{
			active = FALSE;						// Program Is No Longer Active
		}

		return 0;								// Return To The Message Loop
	}

	case WM_SYSCOMMAND:							// Intercept System Commands
	{
		switch (wParam)							// Check System Calls
		{
		case SC_SCREENSAVE:					// Screensaver Trying To Start?
		case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
			return 0;							// Prevent From Happening
		}
		break;									// Exit
	}

	case WM_CLOSE:								// Did We Receive A Close Message?
	{
		PostQuitMessage(0);						// Send A Quit Message
		return 0;								// Jump Back
	}

	case WM_KEYDOWN:							// Is A Key Being Held Down?
	{
		keys[wParam] = TRUE;					// If So, Mark It As TRUE
		return 0;								// Jump Back
	}

	case WM_KEYUP:								// Has A Key Been Released?
	{
		keys[wParam] = FALSE;					// If So, Mark It As FALSE
		return 0;								// Jump Back
	}
	case WM_LBUTTONDOWN:
	{
		mx = LOWORD(lParam);
		my = HIWORD(lParam);
		return 0;
		
	}
	case WM_SIZE:								// Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
		return 0;								// Jump Back
	}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure

													// Ask The User Which Screen Mode They Prefer
	/*if (MessageBox(NULL, L"Would You Like To Run In Fullscreen Mode?", L"Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		fullscreen = FALSE;							// Windowed Mode
	}*/
	fullscreen = FALSE;
	// Create Our OpenGL Window
	if (!CreateGLWindow(L"NeHe's OpenGL Framework", 800, 600, 16, fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while (!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
			{
				done = TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (active)								// Program Active?
			{
				if (keys[VK_ESCAPE])				// Was ESC Pressed?
				{
					done = TRUE;						// ESC Signalled A Quit
				}
				else								// Not Time To Quit, Update Screen
				{
					DrawGLScene();					// Draw The Scene
					SwapBuffers(hDC);				// Swap Buffers (Double Buffering)

				}

			}

			if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1] = FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
														// Recreate Our OpenGL Window
				if (!CreateGLWindow(L"NeHe's OpenGL Framework", 640, 480, 16, fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
