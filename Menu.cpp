#include <iostream>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include "Menu.h"

using namespace std;

MENU::MENU()
{	//nothing to do in the constructor because all the data should be initialize in
	//the function InitMenu
}

int MENU::InitMenu(char context[][MAXLINE], int total, int inity, int unchoose, int choose)
{	//Error code -1
	if(total > 0 && total < MAXVALUE)
	{
		total_ContextNumber = total;
		for(int i = 0; i < total_ContextNumber; i++)
		{
			strcpy(MenuContext[i], context[i]);
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
	GotoXY(0, Inity + pass);
	ChangeColour(choose_colour);
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

				if(strcmp(MenuContext[choosenum], "*BLOCK*") == 0)
					choosenum -= 1;		//crossover the BLOCK
			}
			else if(key == 80)			//key down
			{
				choosenum += 1;
				if(choosenum > total_ContextNumber - 1)
					choosenum = 0;

				if(strcmp(MenuContext[choosenum], "*BLOCK*") == 0)
					choosenum += 1;		//crossover the BLOCK
			}
			else if(key == 75)			//key left
				choosenum = 0;
			else if(key == 77)			//key right
				choosenum = total_ContextNumber - 1;
			else if(key == 83 && choosenum < total_ContextNumber - Functions)
			{	//delete file on the programme
				//remember the functional like //history cannot be deleted!
				return (choosenum * (-1) - 1);	//return the negative chosen number - 1
			}	//in order to prevent number 0 should be opened or deleted

			GotoXY(0, Inity + choosenum);
			ChangeColour(choose_colour);
			cout << MenuContext[choosenum];
		}
	}
	
	//let the cursor move to the bottom of the menu
	GotoXY(0, Inity + total_ContextNumber);
	//Change the colour to the unchoose colour
	ChangeColour(unchoose_colour);
	return choosenum;
}

void MENU::ChangeColour(int colour)
{	//standard function to change the colour which will show the following time
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, colour);
}

void MENU::GotoXY(int x, int y)
{	//standard function use to let the cursor goto the coordinate (x,y)
	COORD pos = {x, y};
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hConsole, pos);
}

void MENU::ShowContext()
{	//show the all data line by line
	for(int i = 0; i < total_ContextNumber; i++)
	{
		if(strcmp(MenuContext[i], "*BLOCK*") == 0)
		{	//skip the data if that is *BLOCK*
			cout << endl;
			continue;
		}
		cout << MenuContext[i] << endl;
	}
}
