#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include "SDKDLL.h" 
#include <tchar.h>

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{ 
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		EnableLedControl(false);
		exit(EXIT_SUCCESS);
		return TRUE;
	default:
		return FALSE;
	}
}

POINT getCursorPos() {
	POINT p;
	GetCursorPos(&p);
	return p;
}

COLORREF getColor(POINT p) {
	HDC dc = GetDC(NULL);
	COLORREF color = GetPixel(dc, p.x, p.y);
	ReleaseDC(NULL, dc);
	return color;
}

void debug(POINT p, COLORREF color) {
	printf("pos(%i, %i) rgb(%i, %i, %i)\n", p.x, p.y, GetRValue(color), GetGValue(color), GetBValue(color));
}

void setRowColor(COLOR_MATRIX *colorMatrix, int x, BYTE r, BYTE g, BYTE b, int startY = 0, int maxY = 21) {
	for (int y = startY; y <= maxY; y++) {
		(*colorMatrix).KeyColor[x][y] = KEY_COLOR{ r, g, b };
	}
}

void setHealth(COLOR_MATRIX *colorMatrix) {
	POINT points[4] = { { 88, 902 },{ 88, 942 },{ 88, 982 },{ 78, 1017 } };
	for (int x = 0; x < 5; x++) {
		COLORREF color = getColor(points[x]);
		if (GetRValue(color) >= 80 && GetGValue(color) <= 40 && GetBValue(color) <= 40) {
			setRowColor(colorMatrix, x+1, 255, 0, 0, 0, 17);
		} else {
			setRowColor(colorMatrix, x+1, 0, 0, 0, 0, 17);
		}
	}
	// Always keep last row lit, unless actually dead.
	COLORREF color = getColor(POINT{ 920, 236 });
	if (GetRValue(color) == 135 && GetGValue(color) == 85 && GetBValue(color) == 22) {
		setRowColor(colorMatrix, 5, 0, 0, 0, 0, 17);
	} else {
		setRowColor(colorMatrix, 5, 255, 0, 0, 0, 17);
	}
}

void setExperience(COLOR_MATRIX *colorMatrix) {
	// Columns 5 and 10 don't exist on the function keys row.
	int multiplierPenalty = 0;
	for (int y = 0; y <= 21; y++) {
		if (y == 5 || y == 10) {
			multiplierPenalty += -1;
			continue;
		}
		COLORREF color = getColor(POINT{ 589+(40*(y+multiplierPenalty)), 1070 });
		if (y <= 18 && GetRValue(color) >= 100 && GetGValue(color) >= 90 && GetBValue(color) >= 35) {
			colorMatrix->KeyColor[0][y] = KEY_COLOR{ 228, 200, 129 };
		} else {
			colorMatrix->KeyColor[0][y] = KEY_COLOR{ 0, 0, 0 };
		}
	}
}

//1786, 880
void setMana(COLOR_MATRIX *colorMatrix) {
	POINT points[5] = { { 1786, 902 },{ 1786, 942 },{ 1786, 982 },{ 1786, 1017 }, {1786, 1063} };
	for (int x = 0; x < 5; x++) {
		COLORREF color = getColor(points[x]);
		if (GetRValue(color) <= 40 && GetBValue(color) >= 60) {
			setRowColor(colorMatrix, x+1, 0, 0, 255, 18);
		}
		else {
			setRowColor(colorMatrix, x+1, 0, 0, 0, 18);
		}
	}
}

bool isPoeActive() {
	HWND window = GetForegroundWindow();
	TCHAR title[256];
	GetWindowText(window, title, sizeof(title));
	if (_tcscmp(title, _T("Path of Exile")) != 0) {
		return false;
	}
	COLORREF color = getColor(POINT{8, 945});
	return GetRValue(color) == 196 && GetGValue(color) == 167 && GetBValue(color) == 121;
}

int main()
{
	// Disable mouse clicks
	DWORD prev_mode;
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hStdin, &prev_mode);
	SetConsoleMode(hStdin, prev_mode & ~ENABLE_QUICK_EDIT_MODE);

	// Register ctrl+c handler
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE))
	{
		printf("[ERROR] Unable to register ctrl handler.");
		system("pause");
		return 0;
	}

	// Keyboard checks
	SetControlDevice(DEV_MKeys_L);
	if (!IsDevicePlug()) {
		printf("Could not find a Coolermaster MasterKeys Pro L keyboard.\nSadly currently no other keyboards are supported.\n");
		system("pause");
		return 0;
	}
	EnableLedControl(true);

	printf("[INFO] Made by swordbeta.com\n");
	printf("[INFO] Succesfully setup everything.\n");
	printf("[INFO] Starting PoE RGB now!\n");

	while (1) {
		if (!isPoeActive()) {
			SetFullLedColor(210, 180, 140);
		} else {
			COLOR_MATRIX colorMatrix;
			setHealth(&colorMatrix);
			setExperience(&colorMatrix);
			setMana(&colorMatrix);
			SetAllLedColor(colorMatrix);
			Sleep(50);
		}
	}
	return 0;
}