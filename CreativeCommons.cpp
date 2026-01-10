/*
 * this dialog was built on 28/09/18 21:07
 * Tranditional Chinese can only be seen when using specific Windows operating system
 * all of the words were using ANSI charset
*/

#include "CreativeCommons.h"

extern int Programme_Language;

LRESULT CALLBACK CreativeCommonDialogProc(HWND hwnd, UINT Message,
	WPARAM wParam, LPARAM lParam)
{
	static HBITMAP hCreativeCommons = NULL;
	static HFONT hArial;
	static HWND hButtonOK;
	switch(Message)
	{
		case WM_CREATE:
			hCreativeCommons = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BITMAP_CREATIVECOMMONS));
			if(hCreativeCommons == NULL)
				MessageBoxW(hwnd, L"The bitmap picture cannot be shown...", DialogTitleEn, MB_ICONERROR);
			
			//Change the font of the dialog
			hArial = CreateFont(FontHeight - 5, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS,
				TEXT("Arial"));
			
			//Create the button
			hButtonOK = CreateWindowA("Button", "OK", WS_VISIBLE | WS_CHILD, 10, 440, 390, 35, hwnd,
				(HMENU)CREATIVECOMMONS_BUTTONOK, NULL, NULL);
			SendMessage(hButtonOK, WM_SETFONT, (WPARAM)hArial, TRUE);
		break;
		case WM_PAINT:
		{
			BITMAP bm;
			PAINTSTRUCT ps;
			RECT rcText;
			const size_t n_ShowText = 110;
			wchar_t ShowText[n_ShowText];
			
			//get the handle to device context
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hdcMem = CreateCompatibleDC(hdc);
			SelectObject(hdcMem, hCreativeCommons);
			
			//get the client area of the window
			GetClientRect(hwnd, &rcText);
			
			//get the details of the bitmap and show it on the dialog
			GetObject(hCreativeCommons, sizeof(bm), &bm);
			BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
			
			rcText.top += bm.bmHeight + FontHeight;
			rcText.left += 10;
			rcText.right -= 10;
			
			//show the context according to the language user selected
			SelectObject(hdc, hArial);
			if(Programme_Language == English)
			{
				wcsncpy(ShowText, L"BY (Attribution): You should tag the one who designed the programme "
					"when sharing.", n_ShowText);
				DrawTextW(hdc, ShowText, wcslen(ShowText), &rcText, DT_WORDBREAK);
				rcText.top += FontHeight * 2;
				wcsncpy(ShowText, L"NC (Non-commercial): You are NOT ALLOWED to use this programme with "
					"commercial purposes.", n_ShowText);
				DrawTextW(hdc, ShowText, wcslen(ShowText), &rcText, DT_WORDBREAK);
				rcText.top += FontHeight * 3;
				wcsncpy(ShowText, L"ND (No Derivative Works): You cannot share the programme when "
					"you\'ve done some modifications.", n_ShowText);
				DrawTextW(hdc, ShowText, wcslen(ShowText), &rcText, DT_WORDBREAK);
				rcText.top += FontHeight * 4;
			}
			else	//set the language as Chinese
			{
				wcsncpy(ShowText, L"BY 姓名標示：當您分享此程式時，應當標註程式設計者姓名", n_ShowText);
				DrawTextW(hdc, ShowText, wcslen(ShowText), &rcText, DT_WORDBREAK);
				rcText.top += FontHeight * 3;
				wcsncpy(ShowText, L"NC 非商業用途：這個程式不得出售、轉賣", n_ShowText);
				DrawTextW(hdc, ShowText, wcslen(ShowText), &rcText, DT_WORDBREAK);
				rcText.top += FontHeight * 2;
				wcsncpy(ShowText, L"ND 禁止改作：您需要保持程式原本的功能，分享時不得任意修改本程式的用途", n_ShowText);
				DrawTextW(hdc, ShowText, wcslen(ShowText), &rcText, DT_WORDBREAK);
				rcText.top += FontHeight * 3;
			}
			wcsncpy(ShowText, L"Creative Commons, Taiwan", n_ShowText);
			DrawTextW(hdc, ShowText, wcslen(ShowText), &rcText, DT_CENTER);
			
			DeleteDC(hdcMem);
			EndPaint(hwnd, &ps);
		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case CREATIVECOMMONS_BUTTONOK:	//button was pressed
					DestroyWindow(hwnd);		//close the dialog
				break;
			}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
			DeleteObject(hCreativeCommons);
			DeleteObject(hArial);
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProcA(hwnd, Message, wParam, lParam);
	}
}

void RegisterCreativeCommonsDialog()
{
	//get the console handle
	HWND hwndConsole = GetConsoleWindow();
	//get the console instance
	HINSTANCE hInstConsole = GetModuleHandle(0);

	//register the dialog class
	WNDCLASSA CreativeCommonsDialog = {0};
	
	CreativeCommonsDialog.lpfnWndProc = CreativeCommonDialogProc;
	CreativeCommonsDialog.hInstance = hInstConsole;
	CreativeCommonsDialog.hCursor = LoadCursor(NULL, IDC_ARROW);
	CreativeCommonsDialog.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	CreativeCommonsDialog.lpszClassName = "DialogBoxCreativeCommons";
	
	if(!RegisterClassA(&CreativeCommonsDialog))
		MessageBoxW(NULL, L"DialogBox cannot be registered!", DialogTitleEn, MB_ICONERROR);
}

void ShowCreativeCommonsDialog()
{
	//get the console handle
	HWND hwndConsole = GetConsoleWindow();

	CreateWindowW(L"DialogBoxCreativeCommons", (Programme_Language == Chinese ? DialogTitleCh : DialogTitleEn),
		WS_VISIBLE | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 410, 510, hwndConsole, NULL, NULL, NULL);
	
	//create message loop to process the messages
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
