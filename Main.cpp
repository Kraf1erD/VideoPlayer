#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <dshow.h>
#include "Buttons.h"
#include "resource.h"

#pragma comment(lib, "Strmiids.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

TCHAR WinName[] = _T("VideoPlayer Beta 1.0");
TCHAR szFile[MAX_PATH];

HWND hEdit;
HBRUSH hBrush = (HBRUSH)(COLOR_WINDOW + 1);

int APIENTRY _tWinMain(HINSTANCE This, HINSTANCE Prev, LPTSTR cmd, int mode)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS wc;

	wc.hInstance = This;
	wc.lpszClassName = WinName;
	wc.lpfnWndProc = WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hIcon = LoadIcon(This, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = hBrush;

	if (!RegisterClass(&wc))
		return 0;

	hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, WinName, _T("Видеоплеер"), WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		(GetSystemMetrics(SM_CXSCREEN) - 800) / 2, (GetSystemMetrics(SM_CYSCREEN) - 240) / 2, 800, 240,
		HWND_DESKTOP, NULL, This, NULL);
	if (hWnd == NULL)
		return 0;
	ShowWindow(hWnd, mode);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), NULL, WS_CHILD | WS_VISIBLE, 10, 100, 765, 20, hWnd, NULL, NULL, NULL);
		SendMessage(hEdit, EM_SETCUEBANNER, TRUE, (LPARAM)_T("Укажите путь к файлу"));
		CreateWindow(_T("BUTTON"), _T("Открыть файл"), WS_CHILD | WS_VISIBLE, 655, 170, 120, 22, hWnd, (HMENU)OPENFILE, NULL, NULL);
		CreateWindow(_T("BUTTON"), _T("Запуск"), WS_CHILD | WS_VISIBLE, 10, 170, 80, 22, hWnd, (HMENU)STARTVID, NULL, NULL);
		CreateWindow(_T("BUTTON"), _T("О программе"), WS_CHILD | WS_VISIBLE, 193, 170, 120, 22, hWnd, (HMENU)ABOUT, NULL, NULL);
		CreateWindow(_T("BUTTON"), _T("Сменить цвет"), WS_CHILD | WS_VISIBLE, 438, 170, 120, 22, hWnd, (HMENU)COLOR, NULL, NULL);
		break;
	}

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case STARTVID:
		{
			IGraphBuilder* pGraph = NULL;
			IMediaControl* pControl = NULL;
			IMediaEvent* pEvent = NULL;

			// Initialize the COM library.
			HRESULT hr = CoInitialize(NULL);
			if (FAILED(hr))
			{
				MessageBox(NULL, L"Невозможно запустить библиотеку COM", L"Ошибка", MB_OK | MB_ICONERROR);
				return 0;
			}

			// Create the filter graph manager and query for interfaces.
			hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
				IID_IGraphBuilder, (void**)&pGraph);
			if (FAILED(hr))
			{
				MessageBox(NULL, L"Невозможно создать менеджер графов фильтра", L"Ошибка", MB_OK | MB_ICONERROR);
				return 0;
			}

			hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
			hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);

			// Build the graph. IMPORTANT: Change this string to a file on your system.
			hr = pGraph->RenderFile(szFile, NULL);
			if (SUCCEEDED(hr))
			{
				// Run the graph.

				hr = pControl->Run();
				if (SUCCEEDED(hr))
				{
					// Wait for completion.
					long evCode;
					pEvent->WaitForCompletion(INFINITE, &evCode);

					// Note: Do not use INFINITE in a real application, because it
					// can block indefinitely.
				}
			}
			pControl->Release();
			pEvent->Release();
			pGraph->Release();
			CoUninitialize();
		}
		break;

		case OPENFILE:
		{
			OPENFILENAME ofn;

			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = NULL;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = L"Файлы AVI\0*.avi\0Все файлы\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileName(&ofn) == TRUE)
			{
				SetWindowText(hEdit, szFile);
			}
		}
		break;

		case ABOUT:
			MessageBox(hWnd, _T("Copyright © 2022 StrannikCORP (Z_Kraf1er_Z)"), _T("О программе"), MB_OK | MB_ICONINFORMATION);
			break;

		case COLOR:
		{
			CHOOSECOLOR cc;                 // структура стандартного
								// диалогового окна
			static COLORREF acrCustClr[16]; // массив доп. цветов 
			//HBRUSH hbrush;                  // дескриптор кисти
			static DWORD rgbCurrent;        // начальный выбранный цвет

			// Инициализация структуры CHOOSECOLOR 
			ZeroMemory(&cc, sizeof(CHOOSECOLOR));
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hWnd;
			cc.lpCustColors = (LPDWORD)acrCustClr;
			cc.rgbResult = rgbCurrent;
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc) == TRUE)
			{
				hBrush = CreateSolidBrush(cc.rgbResult);
				InvalidateRect(hWnd, NULL, FALSE);
				rgbCurrent = cc.rgbResult;
			}
		}
		break;

		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_DROPFILES:
		DragQueryFile((HDROP)wParam, NULL, szFile, MAX_PATH);
		SetWindowText(hEdit, szFile);
		break;

	case WM_PAINT:
	{
		const TCHAR szInfo[] = _T("Напишите или откройте или перетащите файл");

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		FillRect(hdc, &ps.rcPaint, hBrush);
		TextOut(hdc, 240, 40, szInfo, ARRAYSIZE(szInfo));
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}