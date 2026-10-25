#include <iostream>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "Menu.h"

using namespace std;

// Windows console colour index -> ANSI colour index
// (Windows: 0 black, 1 blue, 2 green, 3 cyan, 4 red, 5 magenta, 6 yellow, 7 white)
static const int win_to_ansi[8] = {0, 4, 2, 6, 1, 5, 3, 7};

void ChangeColour(int colour)
{	//standard function to change the colour which will show the following time
	int fg = colour & 0x0F, bg = (colour >> 4) & 0x0F;
	int fg_code, bg_code;

	if(fg == COLOR_NORMAL && bg == 0)
	{	//use the default colour of the terminal
		cout << "\033[0m";
		return;
	}

	if(fg == COLOR_NORMAL)
		fg_code = 39;				//default foreground colour
	else
		fg_code = (fg & 0x08 ? 90 : 30) + win_to_ansi[fg & 0x07];

	if(bg == 0)
		bg_code = 49;				//default background colour
	else
		bg_code = (bg & 0x08 ? 100 : 40) + win_to_ansi[bg & 0x07];

	cout << "\033[0;" << fg_code << ";" << bg_code << "m";
}

void GotoXY(int x, int y)
{	//standard function use to let the cursor goto the coordinate (x,y)
	cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
}

void ClearScreen()
{	//clear the screen and the scrollback, then move the cursor to the top-left corner
	cout << "\033[2J\033[3J\033[H" << flush;
}

void ResetConsole()
{
	cout << "\033[0m" << flush;
}

// read one byte from stdin, return -1 when nothing could be read
static int ReadByte()
{
	unsigned char c;
	if(read(STDIN_FILENO, &c, 1) != 1)
		return -1;
	return c;
}

// translate the rest of an escape sequence (after ESC) to the conio key code,
// return -2 when ESC was pressed alone, -1 for an unsupported sequence
static int TranslateEscape()
{
	int c = ReadByte();
	if(c == -1)
		return -2;
	if(c != '[' && c != 'O')
		return -1;

	c = ReadByte();
	switch(c)
	{
		case 'A': return 72;		//key up
		case 'B': return 80;		//key down
		case 'C': return 77;		//key right
		case 'D': return 75;		//key left
		case 'H': return 71;		//key home
		case 'F': return 79;		//key end
	}

	if(c >= '0' && c <= '9')
	{	//sequences like ESC [ 3 ~
		int num = c - '0';
		while((c = ReadByte()) >= '0' && c <= '9')
			num = num * 10 + (c - '0');
		if(c != '~')
			return -1;
		switch(num)
		{
			case 1: case 7: return 71;	//key home
			case 4: case 8: return 79;	//key end
			case 3: return 83;			//key delete
		}
	}
	return -1;
}

int getch()
{
	static int pending = -1;	//second code of a translated key
	struct termios old_attr, raw_attr;
	int key;

	if(pending >= 0)
	{
		key = pending;
		pending = -1;
		return key;
	}

	cout << flush;		//show everything before waiting for the input

	if(tcgetattr(STDIN_FILENO, &old_attr) != 0)
	{	//stdin is not a terminal
		key = ReadByte();
		return (key == '\n') ? 13 : key;
	}

	raw_attr = old_attr;
	raw_attr.c_lflag &= ~(ICANON | ECHO | ISIG);
	raw_attr.c_iflag &= ~(ICRNL | IXON);
	raw_attr.c_cc[VMIN] = 1;
	raw_attr.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &raw_attr);

	key = ReadByte();
	if(key == 27)
	{	//wait 100ms for the rest of the escape sequence
		raw_attr.c_cc[VMIN] = 0;
		raw_attr.c_cc[VTIME] = 1;
		tcsetattr(STDIN_FILENO, TCSANOW, &raw_attr);

		int code = TranslateEscape();
		if(code >= 0)
		{
			pending = code;
			key = 224;
		}
		else if(code == -1)
		{	//ignore unsupported keys such as Page Up, drop the rest of the sequence
			while(ReadByte() != -1);
			key = 0;
		}
	}
	else if(key == 127)		//backspace
		key = 8;
	else if(key == '\n')	//enter
		key = 13;

	tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);
	return key;
}

char *ReadLine(char *buf, int size)
{
	cout << flush;
	if(fgets(buf, size, stdin) == NULL)
	{
		buf[0] = '\0';
		return NULL;
	}
	buf[strcspn(buf, "\r\n")] = '\0';
	return buf;
}

MENU::MENU()
{	//nothing to do in the constructor because all the data should be initialize in
	//the function InitMenu
}

int MENU::InitMenu(char context[][MAXLINE], int func_num, int total, int inity, int unchoose, int choose)
{	//Error code -1
	
	if(func_num >= 0)
		this->func_num = func_num;
	else
		return ERROR_CODE;

	if(total > 0 && total < MAXVALUE)
	{
		bool check_all_block = true;
		total_ContextNumber = total;
		for(int i = 0; i < total_ContextNumber; i++)
		{
			strcpy(MenuContext[i], context[i]);
			if (check_all_block && strcmp(MenuContext[i], MENU_EMPTY_BLOCK_OPT) != 0)
				check_all_block = false;
		}
		if (check_all_block) {
			ChangeColour(COLOR_LIGHT_RED);
			cout << "InitMenu(): All menu items are BLOCK, no valid menu to display." << endl;
			ChangeColour(COLOR_NORMAL);
			return ERROR_CODE;
		}
	}
	else
		return ERROR_CODE;
	
	if(inity >= 0)
		Inity = inity;
	else
		return ERROR_CODE;
	
	if(unchoose >= 0 && unchoose <= 255)
		unchoose_colour = unchoose;
	else
		return ERROR_CODE;
	
	if(choose >= 0 && choose <= 255)
		choose_colour = choose;
	else
		return ERROR_CODE;
	
	//if nothing going wrong, return positive 1
	return 1;
}

MENU::~MENU()
{	//nothing to do in the destructor because all the data types are without dynamic one
}

int MENU::StartChoose(int pass)
{
	//first, show all the data on the application
	GotoXY(0, Inity);
	ChangeColour(unchoose_colour);
	ShowContext();
	ChangeColour(choose_colour);

	// check if the passing argument is BLOCK
	if(strcmp(MenuContext[pass], MENU_EMPTY_BLOCK_OPT) == 0) {
		pass += 1;		//skip the BLOCK
	}

	GotoXY(0, Inity + pass);
	cout << MenuContext[pass];
	
	//then, begin the function to choose
	unsigned char key = 0;
	//making the input key in the range (0, 255)
	int choosenum = pass;
	while(key != 13)	//esc to escape the loop and return the number of Chosen one
	{
		key = getch();
		if(key == 27)
		{
			ChangeColour(COLOR_NORMAL);
			return ESCAPE;
		}
		else if(key == 224)				//directional keys
		{
			GotoXY(0, Inity + choosenum);
			ChangeColour(unchoose_colour);
			cout << MenuContext[choosenum];
			key = getch();
			if(key == 72)				//key up
			{
				choosenum -= 1;
				if(choosenum < 0)
					choosenum = total_ContextNumber - 1;

				CheckBlockLimit(BLOCK_DIR_LIMIT_UP, &choosenum);
			}
			else if(key == 80)			//key down
			{
				choosenum += 1;
				if(choosenum > total_ContextNumber - 1)
					choosenum = 0;

				CheckBlockLimit(BLOCK_DIR_LIMIT_DOWN, &choosenum);
			}
			else if(key == 75)			//key left
			{
				choosenum = 0;
				CheckBlockLimit(BLOCK_DIR_LIMIT_DOWN, &choosenum);
			}
			else if(key == 77)			//key right
			{
				choosenum = total_ContextNumber - func_num - 1;
				if(choosenum < 0)
					choosenum = 0;

				CheckBlockLimit(BLOCK_DIR_LIMIT_UP, &choosenum);
			}
			else if(key == 83 && choosenum < total_ContextNumber - func_num)
			{	//delete file on the programme
				//remember the functional like //history cannot be deleted!
				return (choosenum * (-1) - 1);	//return the negative chosen number - 1
			}	//in order to prevent number 0 should be opened or deleted

			GotoXY(0, Inity + choosenum);
			ChangeColour(choose_colour);
			cout << MenuContext[choosenum];
		}
	}

	//Change the colour to the unchoose colour
	ChangeColour(unchoose_colour);
	return choosenum;
}

void MENU::ShowContext()
{	//show the all data line by line
	for(int i = 0; i < total_ContextNumber; i++)
	{
		if(strcmp(MenuContext[i], MENU_EMPTY_BLOCK_OPT) == 0)
		{	//skip the data if that is *BLOCK*
			cout << endl;
			continue;
		}
		cout << MenuContext[i] << endl;
	}
}

void MENU::CheckBlockLimit(blockLimDir dir, int *cur_choosenum)
{
	switch(dir)
	{
		case BLOCK_DIR_LIMIT_UP:
			while(strcmp(MenuContext[*cur_choosenum], MENU_EMPTY_BLOCK_OPT) == 0){
				// check if content reached to the limit
				*cur_choosenum -= 1;		//crossover the BLOCK
				if (*cur_choosenum < 0) {
					*cur_choosenum = total_ContextNumber - 1;
				}
			}
			break;

		case BLOCK_DIR_LIMIT_DOWN:
			while(strcmp(MenuContext[*cur_choosenum], MENU_EMPTY_BLOCK_OPT) == 0){
				// check if content reached to the limit
				*cur_choosenum += 1;		//crossover the BLOCK
				if (*cur_choosenum > total_ContextNumber - 1) {
					*cur_choosenum = 0;
				}
			}
			break;
	}
}
