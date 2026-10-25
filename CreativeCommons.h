#ifndef CREATIVECOMMONS_H
#define CREATIVECOMMONS_H

#define English 101
#define Chinese 102

#define PATH_CREATIVECOMMONS_IMAGE "CreativeCommons.png"

const char DialogTitleEn[] = "Quiz - Creative Commons";
const char DialogTitleCh[] = "Quiz - 創用 CC";
const unsigned short int FontHeight = 27;

// show the Creative Commons dialog (GTK3), block until the dialog is closed
// and fall back to the terminal when no display is available
void ShowCreativeCommonsDialog();

#endif
