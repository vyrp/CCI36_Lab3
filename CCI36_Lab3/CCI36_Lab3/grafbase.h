#include <Windows.h>

void InitGraphics();

/****************************************************************************
*  Reset display to default mode.                                           *
****************************************************************************/
void CloseGraphics(void);


/****************************************************************************
*  Clear the graphics screen.                                               *
****************************************************************************/
void ClearGraphicsScreen(void);

/****************************************************************************
*  Set the X dimension of the current window in pixels.                 *
****************************************************************************/
void SetMaxX(int maxX);

/****************************************************************************
*  Set the X dimension of the current window in pixels.                 *
****************************************************************************/
void SetMaxY(int maxY);


/****************************************************************************
*  Returns the X dimension of the current window in pixels.                 *
****************************************************************************/
int GetMaxX(void);

/****************************************************************************
*  Returns the Y dimension of the current window in pixels.                 *
****************************************************************************/
int GetMaxY(void);



/****************************************************************************
*  Set current graphics drawing color.                                      *
****************************************************************************/
void SetGraphicsColor(int new_color, int width);

/****************************************************************************
*  Draws a pixel at the specified point on the screen.                      *
*  Caution!! GpiSetPel has been found to crash programs at some locations!  *
****************************************************************************/
void DrawPixel(int x, int y);

/****************************************************************************
*  Returns the color value of the pixel at the specified point on the       *
*  screen.                                                                  *
****************************************************************************/
int GetPixel(int x, int y);

void CloseGraphics();

static LRESULT CALLBACK WinProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);