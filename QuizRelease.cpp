//Bill Tsou, from National Ilan University
//Built the prgramme Quiz on March 5th, 2018
// Update project starts from Jan. 10th, 2026

#include <iostream>
#include <fstream>

#include "QuizCommon.h"

using namespace std;

//Set the default programme language as English
int Programme_Language = English;

int main()
{
	int total, chosen, passchoose = 0;
	char Answer[MAXVALUE][MAXLINE], Question[MAXVALUE][MAXLINE];

    // Set the console output code page to UTF-8 to fix display error on Windows 11
    SetConsoleOutputCP(65001);

	RegisterCreativeCommonsDialog();	//register dialog class only once

Start:
	MENU *menu = new MENU();
	ReadSetup();
	system("cls");					//clear everything on the console window
	ProgrammeTitle();				//show the programme title
	
	ShowText("從選單中選擇測驗或功能 (Delete 刪除選單中的檔案)：",
	 "Select the quiz or functionality from Menu (Delete to remove the file on Menu):");
	total = GetTheMenu(Answer);		//use Answer as buffer to store the menu info
	//total to calculate how much lines are in the file
	if(total == ERROR_CODE)	//the menu file has just built up, restart the programme
	{
		delete menu;
		goto Start;
	}
	if(EmptyFile("[Quiz].qrc") == 0)//if the menu file is not empty
		strcpy(Answer[total], "*BLOCK*");
	else
		total = -1;
	if(Programme_Language == Chinese)
	{
		strcpy(Answer[total + 1], " /關於");
		strcpy(Answer[total + 2], " /語言設定 Language");
		strcpy(Answer[total + 3], " /搜尋資料庫");
		strcpy(Answer[total + 4], " /開啟檔案位置");
		strcpy(Answer[total + 5], " /編輯選單中的檔案");
		strcpy(Answer[total + 6], " /新增現有檔案到選單");
		strcpy(Answer[total + 7], " /新增資料庫");
		strcpy(Answer[total + 8], " /清空歷史檔案");
		strcpy(Answer[total + 9], " /歷史錯誤練習");
	}
	else
	{
		strcpy(Answer[total + 1], " /About this programme");
		strcpy(Answer[total + 2], " /Language Setup");
		strcpy(Answer[total + 3], " /Search");
		strcpy(Answer[total + 4], " /Open Quiz directory");
		strcpy(Answer[total + 5], " /Edit the file on Menu");
		strcpy(Answer[total + 6], " /Add the old file to Menu");
		strcpy(Answer[total + 7], " /Create new library");
		strcpy(Answer[total + 8], " /Clear History file");
		strcpy(Answer[total + 9], " /History");
	}
	total = total + Functions + 1;
	menu->InitMenu(Answer, total, InitialY);
	chosen = menu->StartChoose(passchoose);
	delete menu;
	passchoose = chosen;

	if(chosen == total - 9)			//About the programme
	{
		About();
		goto Start;
	}
	else if(chosen == total - 8)	//Language Setup
	{
		EnterSetup();
		goto Start;
	}
	else if(chosen == total - 7)	//Search
	{
		if(EmptyFile("[Quiz].qrc") == 0)	//If the menu file is not empty
			SearchOnMenu();
		else						//If the menu file is not empty or does not exist
		{
			ChangeColour(COLOR_LIGHT_YELLOW);
			ShowText("\n選單是空的！沒有辦法從選單中搜尋關鍵字\n", "\nMenu is empty! Cannot search from Menu\n");
			ChangeColour(COLOR_NORMAL);
			getch();
		}
		goto Start;
	}
	else if(chosen == total - 6)	//Start file directory
	{
		system("start .");
		goto Start;
	}
	else if(chosen == total - 5)	//Edit the file on menu
	{
		if(EmptyFile("[Quiz].qrc") == 0)
			EditOnMenu(Answer, total - Functions - 1);
		else
		{
			ChangeColour(COLOR_LIGHT_YELLOW);
			ShowText("\n選單中沒有選項可供編輯...\n", "\nThere is no file on Menu to edit...\n");
			ChangeColour(COLOR_NORMAL);
			getch();
		}
		goto Start;
	}
	else if(chosen == total - 4)	//Add the old file into the menu
	{
		AddOldFile();
		goto Start;
	}
	else if(chosen == total - 3)	//Create new library
	{
		CreateNewLib();
		goto Start;
	}
	else if(chosen == total - 2)	//Clear data in history file
	{
		cout << endl;
		ChangeColour(COLOR_PURPLE);
		ShowText("Quiz 將會把歷史檔案的資料全部清除\n",
			"Quiz will clear all the data in History file\n");
		ShowText("您確定要這樣做嗎？(y/n): ",
			"Are you sure to continue?(y/n): ");
		char confirm = 'A';
		while(confirm != 'Y' && confirm != 'N')
			confirm = toupper(getch());
		if(confirm == 'Y')
		{
			fstream file;
			file.open("[History].qzc", ios::out);
			if(!file)
			{
				ChangeColour(COLOR_WHITE);
				ShowText("建立檔案時發生問題！", "Error when building History file");
				getch();
			}
			else
			{
				file.close();
			}
		}
		goto Start;
	}
	else if(chosen == total - 1)	//History Mode
	{
		total = ReadQuestion(Answer, Question, "[History].qzc");
		if(total == 0)
		{
			ChangeColour(COLOR_LIGHT_YELLOW);
			ShowText("所有的錯誤已被清除！\n\n",
			 "All of the past incorrect questions have been eliminated!\n\n");
			getch();
			goto Start;
		}
		else if(total == ERROR_CODE)
		{	//History file hasn't been built
			ShowText("歷史檔案檔案尚未建立！\n\n", "The history file has not been built!\n\n");
			getch();
			goto Start;
		}
		BreakOrder(Answer, Question, total);
		StartTesting(Answer, Question, total, true);
	}
	else if(chosen == ESCAPE)		//close the window
	{
		exit(0);
	}
	else if(chosen < 0)				//delete file from the menu
	{
		DeleteOnMenu(Answer, (chosen * (-1) - 1), total - Functions - 1);
		passchoose = chosen * (-1) - 1;
		goto Start;
	}
	else							//Open the file
	{
		strcat(Answer[chosen], ".qz");	//add the filename-ex from the menu that have chosen
		total = ReadQuestion(Answer, Question, Answer[chosen]);	//Read the file
		if(total == 0)
		{
			ChangeColour(COLOR_LIGHT_YELLOW);
			ShowText("檔案中沒有任何有效資料...", "No valid library...");
			getch();
			goto Start;
		}
		else if(total == ERROR_CODE)	//the file does not exist now
		{
			getch();
			goto Start;
		}
		BreakOrder(Answer, Question, total);					//Break the original order
		StartTesting(Answer, Question, total);
	}

	do
	{
		ShowText("您想要再重新測驗嗎 (y/n)? ", "Do you want to test again (y/n)? ");
		chosen = getch();
		if(toupper(chosen) == 'Y')
			goto Start;
		else if(toupper(chosen) == 'N')
			break;
		cout << endl << endl;
	}while(1);
	return 0;
}
