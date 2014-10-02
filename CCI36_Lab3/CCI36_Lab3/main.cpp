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
#include <algorithm>

static LRESULT CALLBACK WinProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

#define Max(x, y) ((x) > (y) ? (x) : (y))
#define Arred(x) ((int)((x) + 0.5))   // only for x>=0

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
#define LINE_SCAN 0
#define FLOOD_FILL_RECURSIVE 1
#define FLOOD_FILL 2

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

const int pattern_max = 5;
char pattern[pattern_max][6][7] = {
	{
		"rrorrr",
		"rrorrr",
		"oooooo",
		"rrorrr",
		"rrorrr",
		"oooooo"
	}, {
		"bbbbbb",
		"rrrrrr",
		"wwwwww",
		"mmmmmm",
		"oooooo",
		"wwwwww"
	}, {
		"rwwwwr",
		"rrwwrr",
		"rrwwrr",
		"rwwwwr",
		"wwwwww",
		"wwwwww"
	}, {
		"bbbbbb",
		"bbbbbb",
		"brrbrr",
		"brrrrr",
		"bbrrrb",
		"bbbrbb"
	}, {
		"wwwrww",
		"wrwwwr",
		"wwwrww",
		"wrwwwr",
		"wwwrww",
		"wrwwwr"
	}
};

int current_pattern; // chosen pattern from the list above (if pattern_max then it is no pattern)

bool xor = false;

char buffer[200] = "";					// string buffer for keyboard input
int algorithmType = LINE_SCAN;

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
void SetPatternColor(int x, int y) {
	x /= 8; // scale
	y /= 8; // scale
	int i = y % 6;
	int j = x % 6;
	switch (pattern[current_pattern][i][j]){
	case 'r':
		SetGraphicsColor(MY_RED, 1);
		break;
	case 'o':
		SetGraphicsColor(MY_LIGHTRED, 1);
		break;
	case 'b':
		SetGraphicsColor(MY_BLUE, 1);
		break;
	case 'g':
		SetGraphicsColor(MY_GREEN, 1);
		break;
	case 'w':
		SetGraphicsColor(MY_WHITE, 1);
		break;
	case 'm':
		SetGraphicsColor(MY_MAGENTA, 1);
		break;
	}
}

void DrawPixel(int x, int y, bool with_pattern)
{
	if (!with_pattern || pattern[current_pattern][x % 6][y % 6] != '0')
	{
		if (with_pattern) {
			SetPatternColor(x, y);
		}
		SetPixel(hdc, x, y, xor ? GetPixel(hdc, x, y) ^ Mask : win_draw_color);
		SetGraphicsColor(color, 1);
	}
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

HMENU menu, menu_draw;//, menu_color, menu_pattern, menu_algorithm;

void MenuBar()
{

	menu = CreateMenu();
	menu_draw = CreatePopupMenu();
	//menu_color = CreatePopupMenu();
	//menu_pattern = CreatePopupMenu();
	//menu_algorithm = CreatePopupMenu();


	AppendMenu(
		menu,					// handle to menu to be changed
		MF_POPUP,				// menu-item flags
		(UINT)menu_draw,		// menu-item identifier or handle to drop-down menu or submenu
		(LPCTSTR)L"&Draw"		// menu-item content
		);

	/*AppendMenu(
		menu,					// handle to menu to be changed
		MF_POPUP,				// menu-item flags
		(UINT)menu_pattern,		// menu-item identifier or handle to drop-down menu or submenu
		(LPCTSTR)L"&Pattern"	// menu-item content
		);

	AppendMenu(menu, MF_POPUP, (UINT)menu_color, (LPCTSTR)L"&Color");

	AppendMenu(
		menu,					// handle to menu to be changed
		MF_POPUP,				// menu-item flags
		(UINT)menu_algorithm,	// menu-item identifier or handle to drop-down menu or submenu
		(LPCTSTR)L"&Algorithm"	// menu-item content
		);*/

	InsertMenu(menu_draw, 0, MF_STRING, 21, (LPCTSTR)L"&Polygon");

	AppendMenu(menu_draw, MF_STRING, 22, (LPCTSTR)L"&Circle");

	/*InsertMenu(menu_color, 0, MF_STRING, 1, (LPCTSTR)L"Black");
	AppendMenu(menu_color, MF_STRING, 2, (LPCTSTR)L"Blue");
	AppendMenu(menu_color, MF_STRING, 3, (LPCTSTR)L"Green");
	AppendMenu(menu_color, MF_STRING, 4, (LPCTSTR)L"Cyan");
	AppendMenu(menu_color, MF_STRING, 5, (LPCTSTR)L"Red");

	AppendMenu(menu_color, MF_STRING, 6, (LPCTSTR)L"Magenta");
	AppendMenu(menu_color, MF_STRING, 7, (LPCTSTR)L"Brown");
	AppendMenu(menu_color, MF_STRING, 8, (LPCTSTR)L"LightGray");
	AppendMenu(menu_color, MF_STRING, 9, (LPCTSTR)L"DarkGray");

	AppendMenu(menu_color, MF_STRING, 10, (LPCTSTR)L"LightBlue");
	AppendMenu(menu_color, MF_STRING, 11, (LPCTSTR)L"LightGreen");
	AppendMenu(menu_color, MF_STRING, 12, (LPCTSTR)L"LightCyan");
	AppendMenu(menu_color, MF_STRING, 13, (LPCTSTR)L"LightRed");
	AppendMenu(menu_color, MF_STRING, 14, (LPCTSTR)L"LightMagenta");
	AppendMenu(menu_color, MF_STRING, 15, (LPCTSTR)L"Yellow");
	AppendMenu(menu_color, MF_STRING, 16, (LPCTSTR)L"White");

	InsertMenu(menu_pattern, 0, MF_STRING, 100, (LPCTSTR)L"Bricks");
	AppendMenu(menu_pattern, MF_STRING, 101, (LPCTSTR)L"Rainbow");
	AppendMenu(menu_pattern, MF_STRING, 102, (LPCTSTR)L"Crosses");
	AppendMenu(menu_pattern, MF_STRING, 103, (LPCTSTR)L"Heart");
	AppendMenu(menu_pattern, MF_STRING, 104, (LPCTSTR)L"Dotted");
	AppendMenu(menu_pattern, MF_STRING, 105, (LPCTSTR)L"No Pattern");

	InsertMenu(menu_algorithm, 0, MF_STRING, 200, (LPCTSTR)L"Scan Line");
	AppendMenu(menu_algorithm, MF_STRING, 201, (LPCTSTR)L"Recursive Flood");
	AppendMenu(menu_algorithm, MF_STRING, 202, (LPCTSTR)L"Flood");*/
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
			DrawPixel(Arred((int)x), Arred((int)y), false);
			x = x + dx;    // dx = 1 ou -1 ou m
			y = y + dy;   // yx = 1 ou -1 ou 1/m
		}
	}
}

/**********************************************
* Draws the polygon to guarantee closure.
***********************************************/
void DrawPoly(polygon_type &polygon) {
	for (int i = 0; i < polygon.n; i++){
		DrawLine(polygon.vertex[i].x, polygon.vertex[i].y, polygon.vertex[(i + 1) % polygon.n].x, polygon.vertex[(i + 1) % polygon.n].y);
	}
}

bool Empty(int x, int y)
{
	return(GetPixel(x, y) == 0);

}


void InsertVertex(polygon_type &poly, int x, int y)
{	// insert x,y as the last element
	if (poly.n < MAX_POLY)
	{
		poly.vertex[poly.n].x = x;
		poly.vertex[poly.n].y = y;
		poly.n++;
	}

}

/*void GetPoint(polygon_type polygon, int k, int &x, int &y)
{
	x = polygon.vertex[k].x;
	y = polygon.vertex[k].y;

}*/

/*void PolyInsert(edge_list_type &list, int x1, int y1, int x2, int y2)
{
	// insert line segment in edge struct, if not horizontal
	if (y1 != y2)
	{
		int YM = Max(y1, y2), J1 = list.n + 1;

		while (J1 != 1 && list.edge[J1 - 1].Ymax < YM)
		{
			list.edge[J1] = list.edge[J1 - 1];
			J1--;
		}

		list.edge[J1].Ymax = YM;
		list.edge[J1].dx = -1 * (float)(x2 - x1) / (y2 - y1);
		if (y1 > y2)
		{
			list.edge[J1].Ymin = y2;
			list.edge[J1].Xinter = (float)x1;
		}
		else {
			list.edge[J1].Ymin = y1;
			list.edge[J1].Xinter = (float)x2;
		}

		list.n++;
	}

}*/

/*void LoadPolygon(polygon_type &polygon, edge_list_type &list, int &num_Edges)
{
	int x1, x2, y1, y2, k = 0;

	list.n = 0;
	GetPoint(polygon, k, x1, y1);
	num_Edges = 0;

	for (; k <= polygon.n; k++)
	{
		GetPoint(polygon, k%polygon.n, x2, y2);
		if (y1 == y2) x1 = x2;
		else
		{
			PolyInsert(list, x1, y1, x2, y2);
			num_Edges += 1;
			x1 = x2;
			y1 = y2;
		}
	}

}


void Include(edge_list_type &list, int &end_Edge, int final_Edge, int scan)
{
	// include all edges that intersept y_scan
	while (end_Edge < final_Edge && list.edge[end_Edge + 1].Ymax >= scan)
	{
		end_Edge++;
	}
}

void XSort(edge_list_type &list, int start_Edge, int last_Edge)
{
	int L, k; bool sorted = false;
	edge_type temp;

	// Use bubble sort

	for (k = start_Edge; k < last_Edge; k++)
	{

		L = k + 1;

		while (L > start_Edge &&
			list.edge[L].Xinter < list.edge[L - 1].Xinter)
		{
			temp = list.edge[L];
			list.edge[L] = list.edge[L - 1];
			list.edge[L - 1] = temp;
			L--;

		}

	}

}

void FillIn(int x1, int x2, int y)
{
	if (x1 != x2)
	{
		for (int x = x1; x <= x2; x++)
		{
			DrawPixel(x, y, true);

		}
	}
}

void FillScan(edge_list_type &list, int end_Edge, int start_Edge, int scan)
{
	int NX, J, K;


	NX = (end_Edge - start_Edge + 1) / 2;

	J = start_Edge;


	for (K = 1; K <= NX; K++)
	{
		FillIn((int)list.edge[J].Xinter,
			(int)list.edge[J + 1].Xinter, scan);
		J += 2;
	}

}

void UpdateXValues(edge_list_type &list, int last_Edge, int &start_Edge, int scan)
{
	int K1;

	for (K1 = start_Edge; K1 <= last_Edge; K1++)
	{
		if (list.edge[K1].Ymin < scan)
		{
			list.edge[K1].Xinter += list.edge[K1].dx;
		}
		else
		{
			// remove edge
			start_Edge++;
			if (start_Edge <= K1)
				for (int i = K1; i >= start_Edge; i--)
					list.edge[i] = list.edge[i - 1];
		}
	}
}

void FillPolygon(polygon_type &polygon, edge_list_type &list)
{
	int Edges, start_Edge, end_Edge, scan;

	LoadPolygon(polygon, list, Edges);
	if (Edges == 2) return;
	scan = list.edge[1].Ymax;
	start_Edge = 1;
	end_Edge = 1;

	Include(list, end_Edge, Edges, scan);
	while (end_Edge != start_Edge - 1)
	{
		XSort(list, start_Edge, end_Edge);
		FillScan(list, end_Edge, start_Edge, scan);
		scan--;
		UpdateXValues(list, end_Edge, start_Edge, scan);
		Include(list, end_Edge, Edges, scan);
	}
}*/

/*
// stack of points to be used during the DFS
struct point_stack {
	static const int KMAX = 500000;
	point_type v[KMAX];
	int n = 0;

	void push(point_type p) { // inserts a point
		v[n++] = p;
	}

	bool empty() { // checks weather it's empty
		return n == 0;
	}
	point_type pop() { // removes top point
		return v[--n];
	}
};

void FloodFillRec(int x, int y)
{
	if (Empty(x, y))
	{
		DrawPixel(x, y, true);

		if (Empty(x + 1, y))
			FloodFillRec(x + 1, y);
		if (Empty(x - 1, y))
			FloodFillRec(x - 1, y);
		if (Empty(x, y + 1))
			FloodFillRec(x, y + 1);
		if (Empty(x, y - 1))
			FloodFillRec(x, y - 1);
	}
}*/

/*void FloodFillRecursive(polygon_type poly)
{
	int x_seed = 0, y_seed = 0;
	for (int i = 0; i < poly.n; i++)
	{
		x_seed += poly.vertex[i].x;
		y_seed += poly.vertex[i].y;
	}
	x_seed /= poly.n;
	y_seed /= poly.n;
	FloodFillRec(x_seed, y_seed);
}*/

/*bool visited[1000][1000]; // matrix telling witch points have been visited (it could be a bit array)
point_stack s;

void FloodFillNotRec(int x, int y)
{
	memset(visited, 0, sizeof(visited)); // initializes the visited matrix
	point_type p;
	int dx[] = { 0, 1, -1, 0 };
	int dy[] = { 1, 0, 0, -1 };
	p.x = x;
	p.y = y;
	visited[x][y] = true;
	s.push(p); // inserts seed in the stack
	while (!s.empty()) {
		p = s.pop(); // remove the first point from the stack
		DrawPixel(p.x, p.y, true);
		for (int i = 0; i < 4; i++) { // iterates through the neighboors
			point_type p1;
			p1.x = p.x + dx[i];
			p1.y = p.y + dy[i];
			if (p1.x < 0 || p1.x>1000 || p1.y < 0 || p1.y>1000) continue; // check bound cases
			if (!visited[p1.x][p1.y] && Empty(p1.x, p1.y)){ // if not visited and empty visit
				visited[p1.x][p1.y] = true;
				s.push(p1); // insert neighboor in the stack
			}
		}

	}
}*/

/*void FloodFillNotRecCircle(int x, int y, int r)
{
	memset(visited, 0, sizeof(visited)); // initialize visited matrix
	point_type p;
	int dx[] = { 0, 1, -1, 0 };
	int dy[] = { 1, 0, 0, -1 };
	p.x = x;
	p.y = y;
	visited[x][y] = true;
	s.push(p); // push seed into the stack
	while (!s.empty()) {
		p = s.pop(); // remove point from stack
		DrawPixel(p.x, p.y, true);
		for (int i = 0; i < 4; i++) { // iterate over neighboors
			point_type p1;
			p1.x = p.x + dx[i];
			p1.y = p.y + dy[i];
			if (p1.x < 0 || p1.x>1000 || p1.y < 0 || p1.y>1000) continue; // bound cases
			if (!visited[p1.x][p1.y] && (p1.x - x)*(p1.x - x) + (p1.y - y)*(p1.y - y) < r*r){ // if not visited and inside circle, visit
				visited[p1.x][p1.y] = true;
				s.push(p1); // insert neighboor into stack
			}
		}

	}
}*/

/*
void FloodFill(polygon_type poly) {
	int x_center = 10000, y_center = 0; // y_center will be the center for the y's of the vertexes
	// x_center is initially the minimum
	for (int i = 0; i < poly.n; i++)
	{
		x_center = min(poly.vertex[i].x, x_center);
		y_center += poly.vertex[i].y;
	}
	y_center /= poly.n;

	int cont = 0;
	int xs[MAX_POLY];
	int at = 0;
	for (int i = 1; i <= poly.n; i++) {
		// check if the scan line crosses the edge from point i-1 to point i
		if ((poly.vertex[i - 1].y < y_center && poly.vertex[i%poly.n].y > y_center) || (poly.vertex[i - 1].y > y_center && poly.vertex[i%poly.n].y < y_center)) {
			xs[at++] = poly.vertex[i - 1].x + (1.0*(poly.vertex[i%poly.n].x - poly.vertex[i - 1].x)) / (poly.vertex[i%poly.n].y - poly.vertex[i - 1].y) * (y_center - poly.vertex[i - 1].y);
		}
	}
	while (cont % 2 == 0){
		cont = 0; // count how many interceptions with the edge from x_center, y_center
		for (int i = 0; i < at; i++) {
			 if(xs[i] < x_center) {
				cont++;
			}
		}
		x_center+= 1;
	}
	// x_center is inside polygon because it intercepts an odd number of edges from it to infinty
	int x_seed = x_center; 
	int y_seed = y_center;

	FloodFillNotRec(x_seed, y_seed);
}
*/

void PlotCircle(int xc, int yc, int x, int y)
{
	DrawPixel(xc + x, yc + y, false);
	DrawPixel(xc + y, yc + x, false);
	DrawPixel(xc + y, yc - x, false);
	DrawPixel(xc + x, yc - y, false);
	DrawPixel(xc - x, yc + y, false);
	DrawPixel(xc - y, yc + x, false);
	DrawPixel(xc - y, yc - x, false);
	DrawPixel(xc - x, yc - y, false);
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

enum Shape { Line, Circle };



///////////////////////////////////////////////////////////////////////// Croata

class Entity
{
protected:
	my_color color;
public:
	Entity()
	{
		color = MY_WHITE;
	}
	virtual bool Pick(float x, float y, float d) = 0;
	virtual bool Draw() = 0;
};

class Line : public Entity
{
protected:
	float x1, y1, x2, y2;
public:
	Line(float x1, float y1, float x2, float y2) : x1(x1), y1(y1), x2(x2), y2(y2) { }

	virtual bool Pick(float x, float y, float d)
	{
		float dist2, xmin, ymin, xmax, ymax;
		xmin = min(x1, x2);
		ymin = min(y1, y2);
		xmax = max(x1, x2);
		ymax = max(y1, y2);
		dist2 = sqrt((x - x1)*(y2 - y1) - (y - y1)*(x2 - x1)) / (sqrt(x2 - x1) + sqrt(y2 - y1));
		return (dist2 <= d*d) && ((xmin - d <= x) && (x <= xmax + d) && (ymin - d <= y) && (y <= ymax + d));
	}

	virtual bool Draw()
	{
		// TODO
	}
};

class Polygon : public Entity {

};

class Circle : public Entity {

};

std::list<Entity*> entities;

///////////////////////////////////////////////////////////////////////// Harry

/////////////////////////////////////////////////////////////////////////

void main()
{
	Shape shape = Line;
	SetGraphicsColor(color, 1);

	int p0_x, p0_y, p1_x, p1_y, x_1, y_1, x_2, y_2;
	int r = 0, menu_it = 0;
	polygon_type polygon;
	polygon.n = 0;

	InitGraphics();

	menu_item = 0;
	//CheckMenuItem(menu_color, 1, MF_CHECKED);
	CheckMenuItem(menu_draw, 21, MF_CHECKED);
	//CheckMenuItem(menu_pattern, 100 + pattern_max, MF_CHECKED);
	//CheckMenuItem(menu_algorithm, 200, MF_CHECKED);

	current_pattern = pattern_max;
	algorithmType = LINE_SCAN;

	while (key_input != ESC)						// ESC exits the program
	{
		CheckGraphicsMsg();

		if (menu_it != menu_item)

			/*if (menu_item >= 200){
				for (int i = 0; i <= 2; i++)
					CheckMenuItem(menu_algorithm, 200 + i, MF_UNCHECKED);

				CheckMenuItem(menu_algorithm, menu_item, MF_CHECKED);
				if (menu_item >= 200 && menu_item <= 202){
					algorithmType = menu_item - 200;
				}
					
				menu_it = menu_item;

			}*/
			/*else if (menu_item >= 100){
				for (int i = 0; i <= pattern_max; i++)
					CheckMenuItem(menu_pattern, 100 + i, MF_UNCHECKED);
				
				CheckMenuItem(menu_pattern, menu_item, MF_CHECKED);
				if (menu_item >= 100 && menu_item <= 100 + pattern_max)
					current_pattern = menu_item - 100;
				menu_it = menu_item;
			}*/
//			else {
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
					/*default:
						int i;
						for (i = 1; i <= 16; i++)
							CheckMenuItem(menu_color, i, MF_UNCHECKED);
						CheckMenuItem(menu_color, menu_item, MF_CHECKED);
						if (menu_item >= 1 && menu_item <= 16)
							color = menu_item - 1;

						menu_it = menu_item;*/
				}

	//		}

			if (mouse_action == L_MOUSE_DOWN)
			{
				// Pick first point up 
				if (shape == Line){
					if (polygon.n == 0)
					{
						p0_x = p1_x = mouse_x;
						p0_y = p1_y = mouse_y;
						InsertVertex(polygon, p0_x, p0_y);
					}
				}
				if (shape == Circle){
					p0_x = p1_x = mouse_x;
					p0_y = p1_y = mouse_y;
					r = 0;
				}
			}
			if (mouse_action == L_MOUSE_MOVE_DOWN)
			{
				// Example of elastic line
				if (p1_x != mouse_x || p1_y != mouse_y)
				{
					// Erase previous line. NOTE: using XOR line
					if (shape == Line) {
						DrawLineXor(p0_x, p0_y, p1_x, p1_y);
					}
					if (shape == Circle) {
						CircleBresenham(p0_x, p0_y, r);
					}

					p1_x = mouse_x;
					p1_y = mouse_y;

					// Draw new line
					if (shape == Line) {
						DrawLineXor(p0_x, p0_y, p1_x, p1_y);
					}
					if (shape == Circle) {
						r = (int)sqrt((p1_x - p0_x)*(p1_x - p0_x) + (p1_y - p0_y)*(p1_y - p0_y));
						CircleBresenham(p0_x, p0_y, r);
					}

					x_1 = p0_x;
					y_1 = p0_y;
					x_2 = p1_x;
					y_2 = p1_y;
				}
			}
			else  if (mouse_action == L_MOUSE_UP)
			{
				if (shape == Line) {
					DrawLineXor(p0_x, p0_y, p1_x, p1_y);
					DrawLine(p0_x, p0_y, p1_x, p1_y);
					p0_x = p1_x = mouse_x;
					p0_y = p1_y = mouse_y;

					if (polygon.n > 0 &&
						(polygon.vertex[polygon.n - 1].x != p0_x
						|| polygon.vertex[polygon.n - 1].y != p0_y))
						InsertVertex(polygon, p0_x, p0_y);
				}
				if (shape == Circle) {
					DrawCircle(p0_x, p0_y, r);
				}
				mouse_action = NO_ACTION;
			}
			else  if (mouse_action == R_MOUSE_DOWN)
			{
				if (shape == Line){
					if (polygon.n != 0){
						DrawPoly(polygon);
						/*if (algorithmType == LINE_SCAN) {
							edge_list_type list;
							FillPolygon(polygon, list);
						}
						else if (algorithmType == FLOOD_FILL_RECURSIVE){
							FloodFillRecursive(polygon);
						}
						else {
							FloodFill(polygon);
						}*/
						polygon.n = 0;
					}
				}
				if (shape == Circle) {
					// FloodFillNotRecCircle(p0_x, p0_y, r);
				}

				mouse_action = NO_ACTION;
			}
	}

	CloseGraphics();
}
