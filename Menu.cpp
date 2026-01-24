#include <iostream>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include "Menu.h"

using namespace std;

void ChangeColour(int colour)
{	//standard function to change the colour which will show the following time
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, colour);
}

void GotoXY(int x, int y)
{	//standard function use to let the cursor goto the coordinate (x,y)
	COORD pos = {x, y};
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hConsole, pos);
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
	unsigned char key;
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
