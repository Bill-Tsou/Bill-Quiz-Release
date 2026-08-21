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
	int total = 0, chosen = 0, passchoose = 0, dir_file_cnt = 0, function_cnt = 0;
	char current_dir[MAX_PATH] = {0}; // path of database list
	char Answer[MAXVALUE][MAXLINE], Question[MAXVALUE][MAXLINE];

    // Set the console output code page to UTF-8 to fix display error on Windows 11
    SetConsoleOutputCP(65001);

	RegisterCreativeCommonsDialog();	//register dialog class only once

	// initialize current directory
	strncpy(current_dir, PATH_DATABASE, sizeof(current_dir));

	// read user selected language from config file
	ReadSetup();

Start:
	MENU *menu = new MENU();
	system("cls");					//clear everything on the console window
	ProgrammeTitle();				//show the programme title

	if(menu == NULL) {
		ChangeColour(COLOR_LIGHT_RED);
		ShowText("沒有足夠的記憶體建立選單!\n", "There is no enough memory for creating Menu\n");
		ChangeColour(COLOR_NORMAL);
		system("pause");
		return EXIT_FAILURE;
	}

	// get directory and files list
	dir_file_cnt = GetDirMenu(current_dir, Answer);
	if(dir_file_cnt == ERROR_CODE)
	{
		delete menu;
		system("pause");
		return EXIT_FAILURE;
	}

	// append functional options
	function_cnt = GetMenuFunctions(Answer, dir_file_cnt);	//use Answer as buffer to store the menu info
	if(function_cnt == ERROR_CODE)
	{
		delete menu;
		system("pause");
		return EXIT_FAILURE;
	}

	total = dir_file_cnt + function_cnt;

	if(menu->InitMenu(Answer, function_cnt, total, InitialY) == ERROR_CODE) {
		ChangeColour(COLOR_LIGHT_RED);
		ShowText("選單初始化失敗！\n", "Menu Initialization failed!\n");
		ChangeColour(COLOR_NORMAL);

		delete menu;
		system("pause");
		return EXIT_FAILURE;
	}

	ShowText("從選單中選擇測驗或功能 (Delete 刪除選單中的檔案)：",
	 "Select the quiz or functionality from Menu (Delete to remove the file on Menu):");

	chosen = menu->StartChoose(passchoose);
	delete menu;
	passchoose = chosen;

	if(chosen == ESCAPE) {
		GotoXY(0, InitialY + total);
		return EXIT_SUCCESS;
	}
	// check if user tries to delete the selected file
	else if (chosen < 0)
	{
		passchoose = chosen * (-1) - 1;
		DeleteOnMenu(current_dir, Answer, passchoose, total - MAX_USER_FUNC_COUNT - 1);
		goto Start;
	}

	switch(chosen - dir_file_cnt)
	{
	case USER_FUNC_ABOUT:
	{
		About();
		goto Start;
	}

	case USER_FUNC_LANGUAGE:
	{
		EnterLanguageSetup();
		goto Start;
	}

	case USER_FUNC_OPEN_DIRECTORY:
	{
		char path_buf[MAX_PATH];
		snprintf(path_buf, sizeof(path_buf), "explorer \"%s\"", current_dir);
		system(path_buf);
		goto Start;
	}

	case USER_FUNC_LIB_SEARCH:
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

	case USER_FUNC_LIB_EDIT:
	{
		if(EmptyFile("[Quiz].qrc") == 0)
			EditOnMenu(current_dir, Answer, total - MAX_USER_FUNC_COUNT - 1);
		else
		{
			ChangeColour(COLOR_LIGHT_YELLOW);
			ShowText("\n選單中沒有選項可供編輯...\n", "\nThere is no file on Menu to edit...\n");
			ChangeColour(COLOR_NORMAL);
			getch();
		}
		goto Start;
	}

	case USER_FUNC_LIB_CREATE:
	{
		CreateNewLib(current_dir);
		goto Start;
	}

	case USER_FUNC_HISTORY_CLEAR:
	{
		GotoXY(0, InitialY + total + 1);

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

	case USER_FUNC_HISTORY_PRACTICE:
	{
		GotoXY(0, InitialY + total + 1);

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
		break;
	}

	default:

		// to check whether the selected item is directory
		if (Answer[chosen][strlen(Answer[chosen]) - 1] == '\\') {
			// get rid the the last character
			Answer[chosen][strlen(Answer[chosen]) - 1] = '\0';

			// is directory, check if it is previous directory
			if (strcmp(Answer[chosen], "..") == 0) {
				// go to previous directory
				char *last_backslash = strrchr(current_dir, '\\');
				if (last_backslash != NULL) {
					*last_backslash = '\0'; // truncate to previous directory
				}
				else {
					// back to root directory
					strncpy(current_dir, PATH_DATABASE, sizeof(current_dir));
				}
			} else {
				// go to selected sub-directory
				snprintf(current_dir, sizeof(current_dir), "%s\\%s", current_dir, Answer[chosen]);
			}
			// enter different direcctory with initial cursor position
			passchoose = 0;
			goto Start;
		}

		// validate user selection and try to start the quiz
		//let the cursor move to the bottom of the menu
		GotoXY(0, InitialY + total);

		char filename[MAX_PATH];
		snprintf(filename, sizeof(filename), """%s\\%s", current_dir, Answer[chosen]);
		total = ReadQuestion(Answer, Question, filename);	//Read the file
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

		break;
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

	return EXIT_SUCCESS;
}
