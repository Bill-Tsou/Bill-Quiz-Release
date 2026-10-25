/*Created on 5th, March, 2018*/
#include <iostream>

#ifndef _MENU_H
#define _MENU_H

// Colours for ChangeColour()
enum color{
	COLOR_DARK_BLUE = 1,
	COLOR_DARK_GREEN,
	COLOR_FADED_BLUE,
	COLOR_DARK_RED,
	COLOR_DARK_PURPLE,
	COLOR_BROWN,
	COLOR_NORMAL,		// default console text color
	COLOR_GRAY,
	COLOR_SKY_BLUE,
	COLOR_LIGHT_GREEN,
	COLOR_LIGHT_BLUE,
	COLOR_LIGHT_RED,	// error message
	COLOR_PURPLE,
	COLOR_LIGHT_YELLOW,
	COLOR_WHITE
};

enum blockLimDir{
	BLOCK_DIR_LIMIT_UP,
	BLOCK_DIR_LIMIT_DOWN
};

#define MAXVALUE 100
#define MAXLINE 1024		//the max characters of one line can contain
#define ESCAPE 1001			//key esc
#define ERROR_CODE -1001	//error return code

// empty gap display on menu
#define MENU_EMPTY_BLOCK_OPT "*BLOCK*"

/* ==== function prototypes ==== */
// general console API (termios + ANSI escape codes)
void ChangeColour(int colour);		// colour uses Windows console colour codes (bg << 4 | fg)
void GotoXY(int x, int y);
void ClearScreen();
void ResetConsole();				// restore the default terminal colour
// read one key without echo, arrow/Home/End/Delete/PageUp/PageDown keys are translated to
// the Windows conio codes: 224 followed by 72/80/75/77/71/79/83/73/81
int getch();
// read one line from stdin into buf without the trailing newline
char *ReadLine(char *buf, int size);

/* ======== */

class MENU
{
	public:
		MENU();
		int InitMenu(char[][MAXLINE], int func_num, int total, int inity = 0, int unchoose = 7, int choose = 31);
		//menu context, total context num, init y coord, unchoose colour, choose colour
		//it is very useful to use *BLOCK* to represent nothing in one line
		virtual ~MENU();
		int StartChoose(int);//return the number of which user has choosen
		//and give a parameter that is the inital chosen line
		int GetBottomY();	//the y coord of the line below the menu
	
	private:
		char MenuContext[MAXVALUE][MAXLINE];
		int Inity;
		int func_num;
		int total_ContextNumber;
		unsigned char unchoose_colour;
		unsigned char choose_colour;
		//viewport of the menu when it is taller than the terminal
		bool scrolling;		//whether only a part of the menu is shown
		int top;			//index of the first shown item
		int visible;		//number of the shown items
		void UpdateViewport();
		bool EnsureVisible(int choosenum);	//return true when the viewport is moved
		void ShowContext();
		void DrawItem(int index, unsigned char colour);
		void DrawIndicator(const char *arrow, int from, int to);
		void CheckBlockLimit(blockLimDir dir, int *cur_choosenum);
};

#endif 
