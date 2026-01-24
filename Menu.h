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
// general Windows API
void ChangeColour(int colour);
void GotoXY(int x, int y);

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
	
	private:
		char MenuContext[MAXVALUE][MAXLINE];
		int Inity;
		int func_num;
		int total_ContextNumber;
		short int initcoordy;
		unsigned char unchoose_colour;
		unsigned char choose_colour;
		void ShowContext();
		void CheckBlockLimit(blockLimDir dir, int *cur_choosenum);
};

#endif 
