#ifndef CREATIVECOMMONS_H
#define CREATIVECOMMONS_H

#include <windows.h>

#define English 101
#define Chinese 102

#define BITMAP_CREATIVECOMMONS		1001
#define CREATIVECOMMONS_BUTTONOK	1002

const char DialogTitleEn[] = "Quiz - Creative Commons";
const char DialogTitleCh[] = "Quiz - ³Ð¥Î CC";
const unsigned short int FontHeight = 27;

void ShowCreativeCommonsDialog();
void RegisterCreativeCommonsDialog();
LRESULT CALLBACK CreativeCommonDialogProc(HWND, UINT, WPARAM, LPARAM);

#endif
