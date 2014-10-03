/*
* CCI-36
* Lab 03: Janelamento, Segmentação e Seleção interativa
*
* Alunos:
*     Felipe Vincent Yannik Romero Pereira
*     Luiz Filipe Martins Ramos
*
* Data: 03/10/14
*/

#include "stdafx.h"
#include "grafbase.h"
#include "PolyFill.h"
#include <windows.h>
#include <string.h>
#include <math.h>
#include <list>
#include <vector>
#include <algorithm>

static LRESULT CALLBACK WinProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

#define Max(x, y) ((x) > (y) ? (x) : (y))
#define Arred(x) ((int)((x) + 0.5))   // only for x>=0
#define sqr(x) ((x)*(x))

#define PI 3.1415926535897932384626433832795
#define ENTER 13
#define BACKSPACE 8
#define ESC 27
#define NO_ACTION -1
#define L_MOUSE_MOVE_DOWN 0
#define L_MOUSE_DOWN 1
#define R_MOUSE_DOWN 2
#define L_DOUBLE_CLICK 3
#define L_MOUSE_UP 4

#define CONSOLE_SIZE_X 640				// initial input line size in pixel
#define START_TEXT_X 5					// input line box initial position X
#define END_TEXT_X (CONSOLE_SIZE_X-2)	// size of input line box in X
static SHORT numXpixels = 640;			// size of window in X
static SHORT numYpixels = 430;          // size of window in Y
int start_text_y = numYpixels - 20;
int end_text_y = 21 + start_text_y;		// size of input line in Y

int key_input = NO_ACTION, mouse_action = NO_ACTION;
int mouse_x, mouse_y;
static BOOL graphics = TRUE;            // Boolean, enable graphics?

bool xor = false;

char buffer[200] = "";					// string buffer for keyboard input

int menu_item;

static COLORREF win_draw_color = RGB(255, 255, 255);  // current draw color
static HBRUSH blackBrush;
HDC hdc;								// Presentation Space Handle
HWND WinHandle = NULL;					// Client area window handle

/* Foreground colors default to standard EGA palette.                */
/* No map is necessary unless a non-standard palette is implemented. */
static HPEN hpen;

// 16 Predefined colors
typedef enum {
	MY_BLACK, MY_BLUE, MY_GREEN, MY_CYAN, MY_RED, MY_MAGENTA,
	MY_BROWN, MY_LIGHTGRAY, MY_DARKGRAY, MY_LIGHTBLUE, MY_LIGHTGREEN,
	MY_LIGHTCYAN, MY_LIGHTRED, MY_LIGHTMAGENTA, MY_YELLOW, MY_WHITE
} my_color;

/* Define RGB color settings for MY enumerated colors */
static COLORREF color_trans_map[] =
{
	RGB(0, 0, 0),		//MY_BLACK 
	RGB(0, 0, 255),		//MY_BLUE,
	RGB(0, 127, 0),		//MY_GREEN,
	RGB(0, 233, 233),	//MY_CYAN,
	RGB(255, 0, 0),		//MY_RED,
	RGB(255, 0, 255),	//MY_MAGENTA,
	RGB(153, 51, 0),	//MY_BROWN,
	RGB(175, 175, 175),	//MY_LIGHTGRAY,
	RGB(70, 70, 70),	//MY_DARKGRAY,
	RGB(51, 51, 255),	//MY_LIGHTBLUE,
	RGB(0, 255, 0),		//MY_LIGHTGREEN,
	RGB(51, 255, 255),	//MY_LIGHTCYAN,
	RGB(255, 25, 25),	//MY_LIGHTRED,
	RGB(255, 65, 255),	//MY_LIGHTMAGENTA,
	RGB(255, 255, 0),	//MY_YELLOW,
	RGB(255, 255, 255),	//MY_WHITE,
};

int color = MY_WHITE;

/****************************************************************************
*  Set the X dimension of the current window in pixels.                 *
****************************************************************************/
void SetMaxX(int maxX)
{
	numXpixels = maxX;
}

/****************************************************************************
*  Set the X dimension of the current window in pixels.                 *
****************************************************************************/
void SetMaxY(int maxY)
{
	numYpixels = maxY;
	start_text_y = numYpixels - 20;
	end_text_y = 21 + start_text_y;
}

/****************************************************************************
*  Draws a pixel at the specified point on the screen.                      *
*  Caution!! GpiSetPel has been found to crash programs at some locations!  *
****************************************************************************/
const unsigned int Mask = 0x00FFFFFF;


/****************************************************************************
*  Given the position (x,y) of the pixel, this function sets the color to
*  be used, taking into consideration the current pattern.
****************************************************************************/

void DrawPixel(int x, int y)
{
	SetPixel(hdc, x, y, xor ? GetPixel(hdc, x, y) ^ Mask : win_draw_color);
	SetGraphicsColor(color, 1);
}

/****************************************************************************
*                                                                           *
*  Name       :  InitGraphics()                                             *
*                                                                           *
*  Description:   Initializes the process for Window services               *
*  Concepts   : - obtains anchor block handle							    *
*  - creates the main frame window which creates the                        *
*  main client window                                                       *
*                                                                           *
*                                                                           *
****************************************************************************/

wchar_t wind_class[] = L"Window Application";

HMENU menu, menu_draw, menu_action;

void MenuBar()
{

	menu = CreateMenu();
	menu_draw = CreatePopupMenu();
	menu_action = CreatePopupMenu();

	
	AppendMenu(menu, MF_POPUP, (UINT)menu_action, (LPCTSTR)L"&Action");
	
	AppendMenu(
		menu,					// handle to menu to be changed
		MF_POPUP,				// menu-item flags
		(UINT)menu_draw,		// menu-item identifier or handle to drop-down menu or submenu
		(LPCTSTR)L"&Draw"		// menu-item content
		);

	InsertMenu(menu_draw, 0, MF_STRING, 21, (LPCTSTR)L"&Polygon");

	AppendMenu(menu_draw, MF_STRING, 22, (LPCTSTR)L"&Circle");

	InsertMenu(menu_action, 0, MF_STRING, 1, (LPCTSTR)L"Draw");
	AppendMenu(menu_action, MF_STRING, 2, (LPCTSTR)L"Pick");
	AppendMenu(menu_action, MF_STRING, 3, (LPCTSTR)L"Zoom");
}

void InitGraphics()
{
	HINSTANCE hInst = NULL;
	HWND hWnd;
	WNDCLASS wc;
	LPCWSTR window_class = (LPCWSTR)wind_class;

	// Fill up window structure
	wc.lpszClassName = window_class;			// registration name
	wc.hInstance = hInst;						// application instance
	wc.lpfnWndProc = (WNDPROC)WinProc;			// event handling function
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);	// cursor type
	wc.hIcon = NULL;
	wc.lpszMenuName = NULL;						// menu, if any
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // background color
	wc.style = CS_HREDRAW | CS_VREDRAW;			// window style
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;


	MenuBar();

	/* Register window class */

	if (!RegisterClass(&wc))
	{
		printf(" Error in RegisterClass...\n");
		exit(1);
	}

	// Create window
	hWnd = CreateWindow(
		window_class,						// Desktop window class name             
		L"Lab 2 CCI-36",					// window name                 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,	// Window class style                  
		0, 0,								//window  top, left corner(origin)
		500, 500,							// window X,Y size                                    
		(HWND)NULL,							// Parent window
		(HMENU)menu,						// handle to menu
		(HINSTANCE)hInst,					// handle to application instance
		(LPVOID)NULL);						// pointer to window-creation data

	if ((hWnd == NULL))
	{
		printf("Error in CreateWindow ...\n ");
		exit(1);
	}

	// Sets the visibility state of window 
	ShowWindow(hWnd, SW_SHOW);

	// store window handle device context 
	WinHandle = hWnd;
	hdc = GetDC(WinHandle);
	// set hpen, blackbrush for clearing window, color for text and text background
	hpen = CreatePen(PS_SOLID, 1, win_draw_color);
	SelectObject(hdc, hpen);
	blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(255, 255, 255));
}

/****************************************************************************
*  Reset display to default mode.                                           *
****************************************************************************/
void CloseGraphics(void)
{
	// Delete pen and destroy window
	DeleteObject(hpen);
	ReleaseDC(WinHandle, hdc);
	DestroyWindow(WinHandle);          /* Tidy up... */
}

/****************************************************************************
*  Returns the X dimension of the current window in pixels.                 *
****************************************************************************/
int GetMaxX(void)
{
	return numXpixels;
}

/****************************************************************************
*  Returns the Y dimension of the current window in pixels.                 *
****************************************************************************/
int GetMaxY(void)
{
	return numYpixels;
}

/****************************************************************************
*  Set current graphics drawing color.                                      *
****************************************************************************/
void SetGraphicsColor(int new_color, int width)
{
	HPEN hpenOld;
	if (graphics)
	{
		// test to avoid unnecessay pen changing
		if (win_draw_color != color_trans_map[new_color])
		{
			// get COLORREF from defined palette
			win_draw_color = color_trans_map[new_color];

			// create a pen with that color 
			hpen = CreatePen(PS_SOLID, width, win_draw_color);
			hpenOld = (HPEN)SelectObject(hdc, hpen);
			// delete old pen
			DeleteObject(hpenOld);
		}
	}
}

/****************************************************************************
*  Returns the color value of the pixel at the specified point on the       *
*  screen.                                                                  *
****************************************************************************/
int GetPixel(int x, int y)
{
	return (int)GetPixel(hdc, x, y);
}

void CheckGraphicsMsg(void)
{
	MSG msg;
	/*Peek Message from message queue */
	if (PeekMessage(&msg, WinHandle, 0L, 0L, PM_REMOVE))
	{
		// Translate keyboard hit to virtual key code and send to message queue
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void ClearString(char *str)
{
	str[0] = 0x00;
}

void EraseMessage()
{
	RECT rec = { START_TEXT_X, start_text_y, END_TEXT_X, end_text_y };
	HBRUSH backgrdBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	// Clear input input box
	FillRect(hdc, &rec, backgrdBrush);
}

void  PrintMessage(char *buffer)
{
	// Write input text in the graphics window
	// Input line is in the upper portion of the graphics window
	if (strlen(buffer) > 0)
		TextOutA(hdc, START_TEXT_X, start_text_y, (LPCSTR)buffer, strlen(buffer));
}

/****************************************************************************
*  Mouse Handler for Win 95                                                   *
****************************************************************************/
static LRESULT CALLBACK WinProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;

	char str[3] = " ";
	switch (messg)
	{
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		PrintMessage(buffer);
		// Draw everything
		//	  RefreshScreen();

		ValidateRect(hWnd, NULL);

		EndPaint(hWnd, &ps);
		break;
	case WM_CHAR:
		// Win32 keyboard message: lParam=key code wParam= virtual key (ASCII) code 

		if (!(LOWORD(wParam) & KF_UP) &&
			!(LOWORD(wParam) & KF_ALTDOWN))
		{
			//  take keyboard input
			key_input = (char)LOWORD(wParam);

			if (key_input == ENTER) // Enter
			{
				EraseMessage();
				//buffer[strlen(buffer)]=0x00;
			}
			else if (key_input == BACKSPACE) // BackSpace
			{
				if (strlen(buffer) > 0)
				{
					int len = strlen(buffer) - 1;
					// Clear last character in buffer
					buffer[len] = ' ';
					// Clear characters in input box
					strcat_s(buffer, "   ");
					PrintMessage(buffer);
					buffer[len] = 0x00; // put end string
				}
			}
			else if (key_input > 31 && key_input < 130)
			{
				int leng = strlen(buffer);
				EraseMessage();
				str[0] = key_input;
				strcat_s(buffer, str); // add char
				// display, update input box
				PrintMessage(buffer);
			}
			else if (key_input != ESC) //ESC
				key_input = -1;
			break;
		}
	case WM_SIZE:
		// resize 
		SetMaxX(LOWORD(lParam));  // width of client area 
		SetMaxY(HIWORD(lParam));
		PostMessage(WinHandle, WM_PAINT, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		key_input = wParam;
		if (key_input == MK_LBUTTON)
		{
			EraseMessage();
			mouse_x = LOWORD(lParam);
			mouse_y = HIWORD(lParam);
			key_input = wParam;
			printf_s(buffer, " x = %d y = %d", mouse_x, mouse_y);
			PrintMessage(buffer);
			mouse_action = L_MOUSE_MOVE_DOWN;
			ClearString(buffer);
		}
		break;
	case WM_LBUTTONDOWN:
		EraseMessage();
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);
		key_input = wParam;
		printf_s(buffer, " x = %d y = %d", mouse_x, mouse_y);
		PrintMessage(buffer);
		mouse_action = L_MOUSE_DOWN;
		ClearString(buffer);
		break;
	case WM_LBUTTONUP:
		EraseMessage();
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);
		key_input = wParam;
		sprintf_s(buffer, " x = %d y = %d", mouse_x, mouse_y);
		PrintMessage(buffer);
		mouse_action = L_MOUSE_UP;
		ClearString(buffer);
		break;
	case WM_RBUTTONDOWN:
		EraseMessage();
		key_input = wParam;
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);

		sprintf_s(buffer, " x = %d y = %d", mouse_x, mouse_y);
		PrintMessage(buffer);

		mouse_action = R_MOUSE_DOWN;
		ClearString(buffer);

		break;
	case WM_LBUTTONDBLCLK:
		EraseMessage();
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);
		key_input = wParam;
		sprintf_s(buffer, " x = %d y = %d", mouse_x, mouse_y);
		PrintMessage(buffer);
		mouse_action = L_DOUBLE_CLICK;
		ClearString(buffer);
		key_input = wParam;

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		menu_item = LOWORD(wParam);
		break;

	default:
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}

	return 0;
}

void DrawXorPixel(int x, int y)
{
	unsigned int mask = 0x00FFFFFF;

	COLORREF cor = GetPixel(hdc, x, y);

	cor ^= mask; // bit-bit Xor operation mask with color
	SetPixel(hdc, x, y, cor);
}

void DrawLineXor(int x1, int y1, int x2, int y2)
{
	int i, length; float  x, y, dx, dy;

	length = Max(abs(x2 - x1), abs(y2 - y1));

	if (length > 0)
	{
		dx = ((float)(x2 - x1)) / length;
		dy = ((float)(y2 - y1)) / length;
		x = (float)x1; y = (float)y1;
		for (i = 0; i <= length; i++)
		{
			DrawXorPixel(Arred(x), Arred(y));
			x = x + dx;    // dx = 1 ou -1 ou m
			y = y + dy;   // yx = 1 ou -1 ou 1/m
		}
	}
}

void DrawLine(int x1, int y1, int x2, int y2)
{
	int i, length;
	double x, y, dx, dy;

	length = Max(abs(x2 - x1), abs(y2 - y1));

	if (length > 0)
	{
		dx = ((float)(x2 - x1)) / length;
		dy = ((float)(y2 - y1)) / length;
		x = (float)x1; y = (float)y1;
		for (i = 0; i <= length; i++)
		{
			DrawPixel(Arred((int)x), Arred((int)y));
			x = x + dx;    // dx = 1 ou -1 ou m
			y = y + dy;   // yx = 1 ou -1 ou 1/m
		}
	}
}

/**********************************************
* Draws the polygon to guarantee closure.
***********************************************/

bool Empty(int x, int y)
{
	return(GetPixel(x, y) == 0);

}


void InsertVertex(float_polygon_type &poly, float x, float y)
{	// insert x,y as the last element
	if (poly.n < MAX_POLY)
	{
		poly.vertex[poly.n].x = x;
		poly.vertex[poly.n].y = y;
		poly.n++;
	}

}

void PlotCircle(int xc, int yc, int x, int y)
{
	DrawPixel(xc + x, yc + y);
	DrawPixel(xc + y, yc + x);
	DrawPixel(xc + y, yc - x);
	DrawPixel(xc + x, yc - y);
	DrawPixel(xc - x, yc + y);
	DrawPixel(xc - y, yc + x);
	DrawPixel(xc - y, yc - x);
	DrawPixel(xc - x, yc - y);
}

void PlotXorCircle(int xc, int yc, int x, int y)
{
	DrawXorPixel(xc + x, yc + y);
	DrawXorPixel(xc + y, yc + x);
	DrawXorPixel(xc + y, yc - x);
	DrawXorPixel(xc + x, yc - y);
	DrawXorPixel(xc - x, yc + y);
	DrawXorPixel(xc - y, yc + x);
	DrawXorPixel(xc - y, yc - x);
	DrawXorPixel(xc - x, yc - y);
}

void CircleBresenham(int xc, int yc, int r)
{
	int x, y, d, deltaE, deltaSE;
	x = 0;
	y = r;
	d = 1 - r;
	deltaE = 3;
	deltaSE = -2 * r + 5;

	PlotXorCircle(xc, yc, x, y);	// Plot 8 symetrical circle points

	while (y > x)				// Draw a quarter of circle in clockwise
	{
		if (d < 0)
		{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
		}
		else
		{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;
			y--;
		}
		x++;
		PlotXorCircle(xc, yc, x, y); // Plot 8 symetrical circle points
	}
}

void DrawCircle(int xc, int yc, int r)
{
	int x, y, d, deltaE, deltaSE;
	x = 0;
	y = r;
	d = 1 - r;
	deltaE = 3;
	deltaSE = -2 * r + 5;

	PlotCircle(xc, yc, x, y);	// Plot 8 symetrical circle points

	while (y > x)				// Draw a quarter of circle in clockwise
	{
		if (d < 0)
		{
			d += deltaE;
			deltaE += 2;
			deltaSE += 2;
		}
		else
		{
			d += deltaSE;
			deltaE += 2;
			deltaSE += 4;
			y--;
		}
		x++;
		PlotCircle(xc, yc, x, y); // Plot 8 symetrical circle points
	}
}

///////////////////////////////////////////////////////////////////////// Harry

float x_start, x_end, y_start, y_end, heigth, width; /* screem bitmap size */

float wxs = 0.0, wxh = 1.0, wys = 0.0, wyh = 1.0;  /* windows corners variables */

float vxs = 0.0, vxh = 1.0, vys = 0.0, vyh = 1.0; /* viewport corners variables */

float vwsx, vwsy; /* viewing transformation scale */

int inside = 0, botton = 1, top = 2, right = 4, left = 8;
float x_current, y_current;


void SetWindow(float x1, float x2, float y1, float y2)
/* window specification and scale transformation computation */
{
	wxs = x1;
	wxh = x2;
	wys = y1;
	wyh = y2;
	vwsx = (vxh - vxs) / (wxh - wxs);
	vwsy = (vyh - vys) / (wyh - wys);
}
void SetViewport(float x1, float x2, float y1, float y2)
/* Viewport specification and scale transformation computation */
{
	vxs = x1;
	vxh = x2;
	vys = y1;
	vyh = y2;
	vwsx = (vxh - vxs) / (wxh - wxs);
	vwsy = (vyh - vys) / (wyh - wys);
}
void ViewingTransformation(float *x, float *y)
/* transformation from object point to normalized point */
{
	*x = (*x - wxs)*vwsx + vxs;
	*y = (*y - wys)*vwsy + vys;
}
void NormalizedToDevice(float xn, float yn, int *xd, int *yd)
/* transformation from normalized point to device coordenate (its rounds the
float number) */
{
	*xd = (int)(x_start + width*xn);
	*yd = (int)(y_end - (y_start + heigth*yn));
}
void InverseViewingTransformation(float *x, float *y)
{
	*x = (*x - vxs) / vwsx + wxs;
	*y = (*y - vys) / vwsy + wys;
}
void DeviceToNormalized(int xd, int yd, float *xn, float *yn)
{
	*xn = ((float)(xd - x_start)) / width;
	*yn = ((float)(y_end - yd - y_start)) / heigth;
}

void XYEdgeIntersection(float  *x1, float *x2, float *y1, float *y2, float wy, float *x, float *y)
{
	*x = *x1 + (*x2 - *x1)*(wy - *y1) / (*y2 - *y1);
	*y = wy;
}

void LineAbs2(float x2, float y2)
{
	float x1, y1;
	int xi1, yi1, xi2, yi2;

	x1 = x_current;
	y1 = y_current;
	x_current = x2;
	y_current = y2;

	ViewingTransformation(&x1, &y1);
	ViewingTransformation(&x2, &y2);
	NormalizedToDevice(x1, y1, &xi1, &yi1);
	NormalizedToDevice(x2, y2, &xi2, &yi2);
	DrawLine(xi1, yi1, xi2, yi2);

}
void MoveAbs2D(float x, float y)
{
	x_current = x;
	y_current = y;
}


void MoveRel2D(float dx, float dy)
{
	x_current += dx;
	y_current += dy;
}
void DrawLine2(float x1, float y1, float x2, float y2, int color)
{
	SetGraphicsColor(color, 2);
	MoveAbs2D(x1, y1);
	LineAbs2(x2, y2);

}

void SetCode2D(float *x, float *y, int *c)
/* This procedure sets the point code */
{
	*c = inside;
	if (*x < wxs)
		*c |= left;
	else if (*x > wxh)
		*c |= right;
	if (*y < wys)
		*c |= botton;
	else if (*y > wyh)
		*c |= top;
}

bool Clip2D(float *x1, float *y1, float *x2, float *y2)
{
	int c, c1, c2;
	float x, y;
	SetCode2D(x1, y1, &c1);
	SetCode2D(x2, y2, &c2);
	while (((c1 != inside) || (c2 != inside)))
	{
		if ((c1&c2) != inside)
		{

			return(false);
		}
		else
		{
			c = c1;
			if (c == inside)
				c = c2;

			if (left & c)
				XYEdgeIntersection(y1, y2, x1, x2, wxs, &y, &x);
			else if (right & c)
				XYEdgeIntersection(y1, y2, x1, x2, wxh, &y, &x);
			else if (botton & c)
				XYEdgeIntersection(x1, x2, y1, y2, wys, &x, &y);
			else if (top & c)
				XYEdgeIntersection(x1, x2, y1, y2, wyh, &x, &y);
			if (c == c1)
			{

				*x1 = x;
				*y1 = y;

				SetCode2D(x1, y1, &c1);
			}
			else
			{
				*x2 = x;
				*y2 = y;

				SetCode2D(x2, y2, &c2);
			}
		}
	}
	return(true);
}

void DrawLine2D(float x1, float y1, float x2, float y2)
{
	int xi1, yi1, xi2, yi2;
	//x1 = x_current;
	//y1 = y_current;


	//TO DO <MATRIZ>.DoTransformation(&x1, &y1);
	//TO DO  Faça Transformacao x2 y2

	if (Clip2D(&x1, &y1, &x2, &y2))
	{
		ViewingTransformation(&x1, &y1);
		ViewingTransformation(&x2, &y2);
		NormalizedToDevice(x1, y1, &xi1, &yi1);
		NormalizedToDevice(x2, y2, &xi2, &yi2);
		DrawLine(xi1, yi1, xi2, yi2);
	}
}

void LineAbs2D(float x, float y)
{
	DrawLine2D(x_current, y_current, x, y);
	x_current = x;
	y_current = y;
}

void LineRel2D(float dx, float dy)
{
	dx += x_current;
	dy += y_current;
	DrawLine2D(x_current, y_current, dx, dy);
	x_current = dx;
	y_current = dy;
}

void InitGraf()
{
	y_end = (float)GetMaxY();
	x_end = (float)GetMaxX();

	x_start = 0.0f;
	y_start = 0.0f;
	width = (float)(x_end - x_start);
	heigth = (float)(y_end - y_start);
	MoveAbs2D(0.0f, 0.0f);
	SetViewport(0.0f, 1.0f, 0.0f, 1.0f);
	SetWindow(0.0f, (float)x_end, 0.0f, (float)y_end);
}

typedef enum { LEFT, TOP, RIGHT, BOTTOM, LAST } win_edge_type;

bool LineIntersectiom(float_point_type P1, float_point_type P2, win_edge_type edge)
{
	switch (edge)
	{
	case LEFT: return ((P1.x - wxs)*(P2.x - wxs) < 0);
	case RIGHT: return ((P1.x - wxh)*(P2.x - wxh) < 0);
	case TOP: return ((P1.y - wyh)*(P2.y - wyh) < 0);
	case BOTTOM: return ((P1.y - wys)*(P2.y - wys) < 0);
	}
	return false;
}
bool Visible(float_point_type P, win_edge_type edge)
{
	switch (edge)
	{

	case LEFT: return (P.x >= wxs);
	case RIGHT: return (P.x <= wxh);
	case TOP: return (P.y <= wyh);
	case BOTTOM: return (P.y >= wys);
	}
	return false;
}
float_point_type Intersection(float_point_type P1, float_point_type P2, win_edge_type edge)
{
	float_point_type p;
	switch (edge)
	{
	case LEFT:
		p.x = wxs;
		p.y = (P1.y + ((float)(P2.y - P1.y)) / (P2.x - P1.x)*(p.x - P1.x));
		break;
	case RIGHT:
		p.x = wxh;
		p.y = (P1.y + ((float)(P2.y - P1.y)) / (P2.x - P1.x)*(p.x - P1.x));
		break;
	case TOP:
		p.y = wyh;
		p.x = (P1.x + ((float)(P2.x - P1.x)) / (P2.y - P1.y)*(p.y - P1.y));
		break;
	case BOTTOM:
		p.y = wys;
		p.x = (P1.x + ((float)(P2.x - P1.x)) / (P2.y - P1.y)*(p.y - P1.y));
	}
	return p;

}
void ClipEdge(float_point_type P1, float_point_type P2, win_edge_type edge, float_polygon_type &poly_out)
{
	float_point_type Pi;
	if (Visible(P1, edge)) // P is at the same side of window
		InsertVertex(poly_out, P1.x, P1.y);
	if (LineIntersectiom(P1, P2, edge))
	{
		Pi = Intersection(P1, P2, edge);
		InsertVertex(poly_out, Pi.x, Pi.y);
	}

}
void ClipPolygon(float_polygon_type poly, float_polygon_type &poly_out)
{
	win_edge_type edge;
	for (int edg = 0; edg < 4; edg++)
	{
		poly_out.n = 0; // Reset poly_out	
		edge = (win_edge_type)edg;
		poly.vertex[poly.n] = poly.vertex[0];
		for (int i = 0; i < poly.n; i++)
			ClipEdge(poly.vertex[i], poly.vertex[i + 1], edge, poly_out);

		poly = poly_out;// Copy poly_out to poly
	}
}

void DrawPoly(polygon_type &polygon) {
	for (int i = 0; i < polygon.n; i++){
		DrawLine(polygon.vertex[i].x, polygon.vertex[i].y, polygon.vertex[(i + 1) % polygon.n].x, polygon.vertex[(i + 1) % polygon.n].y);
	}
}

void DrawPolygon(float_polygon_type poly)
{
	float_polygon_type poly_out;
	//for (int i = 0; i < poly.n; i++) // Transformação do usuário
	//<Matriz>.DoTransformatio(poly.vertex[i].x, poly.vertex[i].y)
	ClipPolygon(poly, poly_out);
	if (poly_out.n > 0) // resultou em poligono dentro
	{
		polygon_type device_poly;
		device_poly.n = poly_out.n;
		for (int i = 0; i < poly_out.n; i++) // transformacao de janelamento
		{
			ViewingTransformation(&poly_out.vertex[i].x, &poly_out.vertex[i].y);
			NormalizedToDevice(poly_out.vertex[i].x, poly_out.vertex[i].y, &device_poly.vertex[i].x, &device_poly.vertex[i].y);
		}
		DrawPoly(device_poly); // desenha
		//if (fill_polygon)
		//	FillPolygon(poly_out); // preenche poligono
	}
}

void DrawEllipse(float x0, float y0, float rx, float ry) // World coordinates
{
	int rix, riy, xi0, yi0;
	ViewingTransformation(&x0, &y0);
	ViewingTransformation(&rx, &ry);
	NormalizedToDevice(x0, y0, &xi0, &yi0);
	NormalizedToDevice(rx, ry, &rix, &riy);

	int x1, y1, x2, y2;
	if (rix > 0)
	{
		x1 = xi0 - rix;
		x2 = xi0 + rix;
	}
	else if (rix == 0) // make the ellipse 2 pixels wide (a line)
	{
		x1 = xi0;
		x2 = xi0 + 1;
	}
	else return; // wrong radius

	if (riy > 0)
	{
		y1 = yi0 - riy;
		y2 = yi0 + riy;
	}
	else if (riy == 0) // make the ellipse 2 pixels wide (a line)
	{
		y1 = yi0;
		y2 = yi0 + 1;
	}
	else return;

	HBRUSH brush = (HBRUSH)GetStockObject(NULL_BRUSH);
	SelectObject(hdc, brush);
	Ellipse(hdc, x1, y1, x2, y2);  // Draw ellipse
}

///////////////////////////////////////////////////////////////////////// Croata

class Entity
{
protected:
	my_color color;

public:
	Entity()
	{
		SetUnactive();
	}

	void SetActive()
	{
		color = MY_RED;
	}

	void SetUnactive()
	{
		color = MY_WHITE;
	}

	virtual bool Pick(float x, float y, float d) = 0;
	virtual void Draw() = 0;
};

class Segment : public Entity
{
protected:
	float x1, y1, x2, y2;

public:
	Segment(float x1, float y1, float x2, float y2) : x1(x1), y1(y1), x2(x2), y2(y2) { }

	virtual bool Pick(float x, float y, float d) // World coordinates
	{
		float xmin = min(x1, x2);
		float ymin = min(y1, y2);
		float xmax = max(x1, x2);
		float ymax = max(y1, y2);
		float dist2 = sqr((x - x1)*(y2 - y1) - (y - y1)*(x2 - x1)) / (sqr(x2 - x1) + sqr(y2 - y1));
		return (dist2 <= d*d) && ((xmin - d <= x) && (x <= xmax + d) && (ymin - d <= y) && (y <= ymax + d));
	}

	virtual void Draw()
	{
		DrawLine2D(x1, y1, x2, y2);
	}
};

class Polygon : public Entity
{
protected:
	std::vector<Segment> edges;
	float_polygon_type polygon;

public:
	Polygon(float_polygon_type polygon)
	{
		this->polygon = polygon;

		for (int i = 0; i < polygon.n; i++)
		{
			edges.push_back(Segment(polygon.vertex[i].x, polygon.vertex[i].y, polygon.vertex[(i + 1) % polygon.n].x, polygon.vertex[(i + 1) % polygon.n].y));
		}
	}

	virtual bool Pick(float x, float y, float d) // World coordinates
	{
		for (size_t i = 0; i < edges.size(); i++)
		{
			if (edges[i].Pick(x, y, d))
			{
				return true;
			}
		}
		return false;
	}

	virtual void Draw()
	{
		DrawPolygon(polygon);
	}
};

class Circle : public Entity
{
protected:
	float x0, y0, r;
public:
	Circle(float x0, float y0, float r) : x0(x0), y0(y0), r(r) { }

	virtual bool Pick(float x, float y, float d) // World coordinates
	{
		float dist2 = sqr(x - x0) + sqr(y - y0);
		if (r - d <= 0)
			return dist2 <= sqr(d + r);
		else
			return sqr(r - d) <= dist2 && dist2 <= sqr(d + r);
	}

	virtual void Draw()
	{
		DrawEllipse(r, r, x0, y0);
	}
};

std::list<Entity*> entities;
std::list<Entity*>::iterator selected_entity = entities.end();

void PickEntity(int x, int y)
{
	float xf, yf, dxf, dyf;
	DeviceToNormalized(x, y, &xf, &yf);
	InverseViewingTransformation(&xf, &yf);
	DeviceToNormalized(2, 2, &dxf, &dyf);
	InverseViewingTransformation(&dxf, &dyf);

	for (std::list<Entity*>::iterator it = entities.begin(); it != entities.end(); it++)
	{
		if ((*it)->Pick(xf, yf, sqrt(dxf*dyf)))
		{
			if (selected_entity != entities.end())
			{
				(*selected_entity)->SetUnactive();
			}
			selected_entity = it;
			(*selected_entity)->SetActive();
			return;
		}
	}
	selected_entity = entities.end();
}

void Delete()
{
	if (selected_entity != entities.end())
	{
		entities.erase(selected_entity);
		selected_entity = entities.end();
	}
}

/////////////////////////////////////////////////////////////////////////

enum Action { Draw, Pick, Zoom };
enum Shape { Line, Circle, Poly };

Shape shape = Line;
	
void MouseDownDraw() {
	//			// Pick first point up 
	//			if (shape == Line){
	//				if (polygon.n == 0)
	//				{
	//					p0_x = p1_x = mouse_x;
	//					p0_y = p1_y = mouse_y;
	//					InsertVertex(polygon, p0_x, p0_y);
	//				}
	//			}
	//			if (shape == Circle){
	//				p0_x = p1_x = mouse_x;
	//				p0_y = p1_y = mouse_y;
	//				r = 0;
	//			}
}

void MouseDownPick() {

}

void MouseDownZoom() {

}

void MouseMoveDraw() {
	//			// Example of elastic line
	//			if (p1_x != mouse_x || p1_y != mouse_y)
	//			{
	//				// Erase previous line. NOTE: using XOR line
	//				if (shape == Line) {
	//					DrawLineXor(p0_x, p0_y, p1_x, p1_y);
	//				}
	//				if (shape == Circle) {
	//					CircleBresenham(p0_x, p0_y, r);
	//				}
	//
	//				p1_x = mouse_x;
	//				p1_y = mouse_y;
	//
	//				// Draw new line
	//				if (shape == Line) {
	//					DrawLineXor(p0_x, p0_y, p1_x, p1_y);
	//				}
	//				if (shape == Circle) {
	//					r = (int)sqrt((p1_x - p0_x)*(p1_x - p0_x) + (p1_y - p0_y)*(p1_y - p0_y));
	//					CircleBresenham(p0_x, p0_y, r);
	//				}
	//
	//				x_1 = p0_x;
	//				y_1 = p0_y;
	//				x_2 = p1_x;
	//				y_2 = p1_y;
	//			}
}

void MouseMovePick() {

}

void MouseMoveZoom() {

}

void MouseUpDraw() {
	//			if (shape == Line) {
	//				DrawLineXor(p0_x, p0_y, p1_x, p1_y);
	//				DrawLine(p0_x, p0_y, p1_x, p1_y);
	//				p0_x = p1_x = mouse_x;
	//				p0_y = p1_y = mouse_y;
	//
	//				if (polygon.n > 0 &&
	//					(polygon.vertex[polygon.n - 1].x != p0_x
	//					|| polygon.vertex[polygon.n - 1].y != p0_y))
	//					InsertVertex(polygon, p0_x, p0_y);
	//			}
	//			if (shape == Circle) {
	//				DrawCircle(p0_x, p0_y, r);
	//			}
	//			mouse_action = NO_ACTION;
}

void MouseUpPick() {

}

void MouseUpZoom() {

}

void RMouseDownDraw() {
	//			if (shape == Line){
	//				if (polygon.n != 0){
	//					DrawPoly(polygon);
	//					/*if (algorithmType == LINE_SCAN) {
	//						edge_list_type list;
	//						FillPolygon(polygon, list);
	//						}
	//						else if (algorithmType == FLOOD_FILL_RECURSIVE){
	//						FloodFillRecursive(polygon);
	//						}
	//						else {
	//						FloodFill(polygon);
	//						}*/
	//					polygon.n = 0;
	//				}
	//			}
	//			if (shape == Circle) {
	//				// FloodFillNotRecCircle(p0_x, p0_y, r);
	//			}
	//
	//			mouse_action = NO_ACTION;
}

void RMouseDownPick() {

}

void RMouseDownZoom() {

}

void main()
{
		Action action = Draw;
		SetGraphicsColor(color, 1);

		InitGraf();
	
	//	int p0_x, p0_y, p1_x, p1_y, x_1, y_1, x_2, y_2;
	//	int r = 0;
		int menu_it = 0;
	//	polygon_type polygon;
	//	polygon.n = 0;
	//
		InitGraphics();
	
		menu_item = 0;
		CheckMenuItem(menu_action, 1, MF_CHECKED);
		CheckMenuItem(menu_draw, 21, MF_CHECKED);
	
		while (key_input != ESC)						// ESC exits the program
		{
			CheckGraphicsMsg();

			if (menu_it != menu_item)
			{
						switch (menu_item){
						case 21:
							CheckMenuItem(menu_draw, 22, MF_UNCHECKED);
							CheckMenuItem(menu_draw, 21, MF_CHECKED);
							menu_it = menu_item;
							shape = Line;
							break;
						case 22:
							CheckMenuItem(menu_draw, 21, MF_UNCHECKED);
							CheckMenuItem(menu_draw, 22, MF_CHECKED);
							menu_it = menu_item;
							shape = Circle;
							break;
							default:
								int i;
								for (i = 1; i <= 16; i++)
								CheckMenuItem(menu_action, i, MF_UNCHECKED);
								CheckMenuItem(menu_action, menu_item, MF_CHECKED);
								if (menu_item >= 1 && menu_item <= 16){
									switch (menu_item) {
									case 1:
										action = Draw;
										break;
									case 2:
										action = Pick;
										break;
									case 3:
										action = Zoom;
										break;
									}
								}
								menu_it = menu_item;
					}
	
			}
	
			if (mouse_action == L_MOUSE_DOWN)
			{
				switch (action) {
				case Draw:
					MouseDownDraw();
					break;
				case Pick:
					MouseDownPick();
					break;
				case Zoom:
					MouseDownZoom();
					break;
				}
			}
			if (mouse_action == L_MOUSE_MOVE_DOWN)
			{
				switch (action) {
				case Draw:
					MouseMoveDraw();
					break;
				case Pick:
					MouseMovePick();
					break;
				case Zoom:
					MouseMoveZoom();
					break;
				}
			}
			else  if (mouse_action == L_MOUSE_UP)
			{
				switch (action) {
				case Draw:
					MouseUpDraw();
					break;
				case Pick:
					MouseUpPick();
					break;
				case Zoom:
					MouseUpZoom();
					break;
				}
			}
			else  if (mouse_action == R_MOUSE_DOWN)
			{
				switch (action) {
				case Draw:
					RMouseDownDraw();
					break;
				case Pick:
					RMouseDownPick();
					break;
				case Zoom:
					RMouseDownZoom();
					break;
				}
			}
		}
	
		CloseGraphics();
}
