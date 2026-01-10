/*Created on 5th, March, 2018*/
#include <iostream>

#ifndef _MENU_H
#define _MENU_H

#define MAXVALUE 100
#define MAXLINE 1024		//the max characters of one line can contain
#define ESCAPE 1001			//key esc
#define ERROR_CODE -1001	//error return code
#define Functions 9			//there are 9 functions on menu which connot be deleted
class MENU
{
	public:
		MENU();
		int InitMenu(char[][MAXLINE], int, int = 0, int = 7, int = 31);
		//menu context, total context num, init y coord, unchoose colour, choose colour
		//it is very useful to use *BLOCK* to represent nothing in one line
		virtual ~MENU();
		int StartChoose(int);//return the number of which user has choosen
		//and give a parameter that is the inital chosen line
	
	private:
		char MenuContext[MAXVALUE][MAXLINE];
		int Inity;
		int total_ContextNumber;
		short int initcoordy;
		unsigned char unchoose_colour;
		unsigned char choose_colour;
		void ChangeColour(int);
		void GotoXY(int, int);
		void ShowContext();
};

#endif 
