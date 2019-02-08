/* Copyright (C) 2019 Matej Gomboc https://github.com/MatejGomboc

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see http://www.gnu.org/licenses/. */


// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include "plotter.h"


#define IDD_DEVICE_CBO 101


Plotter plotter;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = (HICON)LoadImage(NULL, L"mandelbrot.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"MainWindow";
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(
		NULL,
		L"MainWindow",
		L"Mandelbrot plotter",
		WS_OVERLAPPEDWINDOW | WS_MAXIMIZE | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
		return -1;

	ShowWindow(hWnd, nCmdShow | SW_MAXIMIZE | SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		{
			HWND hWndComboBox = CreateWindowEx(
				NULL,
				L"COMBOBOX",
				L"device_select",
				CBS_AUTOHSCROLL | CBS_DROPDOWNLIST | CBS_HASSTRINGS | CBS_SORT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL,
				0, 0, 240, 120,
				hWnd,
				(HMENU)IDD_DEVICE_CBO,
				(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
				NULL
			);

			if (!hWndComboBox)
				return -1;

			std::vector<std::string> device_names = plotter.get_device_names();

			for (int i = 0; i < device_names.size(); i++)
			{
				SendMessage(hWndComboBox, CB_ADDSTRING, NULL, (LPARAM)L"A");
				SendMessage(hWndComboBox, CB_ADDSTRING, NULL, (LPARAM)L"B");
				SendMessage(hWndComboBox, CB_ADDSTRING, NULL, (LPARAM)L"C");
				SendMessage(hWndComboBox, CB_ADDSTRING, NULL, (LPARAM)L"D");
			}
			
			//SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)NULL);

			return 0;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDD_DEVICE_CBO:
				{
					switch (HIWORD(wParam))
					{
						case CBN_SELCHANGE:
						{
							LRESULT selection = SendMessage((HWND)lParam, CB_GETCURSEL, (WPARAM)NULL, (LPARAM)NULL);
							return 0;
						}
					}
				}
			}
		}
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
			return 0;
		}
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
