#include <iostream>
#include <fstream>

#include "QuizCommon.h"

using namespace std;

// retrieve language variable defined in QuizRelease.cpp
extern int Programme_Language;
const char split[10] = "=";

int EmptyFile(char *filename)
{
	fstream File;
	int FileSize;
	File.open(filename, ios::in);
	if(!File)
	{
		ShowText("需要新增檔案 ", "The file should be built ");
		cout << filename << " ";
		return -1;
	}
	else
	{
		File.seekg(0, ios::end);
		FileSize = File.tellg();
		File.close();
		if(FileSize == 0)
			return 1;	//the file is empty
		else
			return 0;	//the file is not empty
	}
}

char *trim(char *str)
{
    if (str == NULL) return NULL;

    // trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') return str; // All spaces

    // trim trailing space
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end + 1) = '\0';

    return str;
}

void ShowText(const char *CH, const char *EN)
{
	if(Programme_Language == Chinese)
		cout << CH;
	else
		cout << EN;
}

void ProgrammeTitle()
{
	system("cls");		//clear the console interface
	ChangeColour(COLOR_LIGHT_YELLOW);
	ShowText("Quiz - 中英文單字測驗\n\n",
	 "Quiz - Test for Chinese, English Translation\n\n");
	ChangeColour(COLOR_NORMAL);
}

int ReadSetup()
{
	fstream Setup;
	char buffer[20];
	Setup.open("[Quiz].set", ios::in);
	if(!Setup)
	{
		Setup.close();
		Setup.open("[Quiz].set", ios::out);
		if(!Setup)	//if the [Quiz].set file doesn't exist, build a new one
		{
			ChangeColour(COLOR_LIGHT_RED);
			ShowText("建立檔案時出現問題",
			 "There\'s some problems while building setup file!");
			getch();
			exit(0);
		}
		else
		{
			Setup << English << endl;	//default language is English
			Setup.close();
			ReadSetup();
		}
	}
	else
	{
		Setup.getline(buffer, sizeof(buffer));
		Programme_Language = atoi(buffer);
		Setup.close();
	}
}

int ListPathFiles(const char *current_dir, char MenuContext[][MAXLINE], int menu_start_idx, bool list_dir)
{
	WIN32_FIND_DATAA findFileData;
	HANDLE findHandle;
	char searchPath[MAX_PATH];
	int number = 0;

    // Create search pattern
	snprintf(searchPath, MAX_PATH, "%s\\*", current_dir);

	for(int i = 0; i < 2; i++) {
		findHandle = FindFirstFileA(searchPath, &findFileData);

		if (findHandle == INVALID_HANDLE_VALUE) {

			if (i == 0) {
				// try to create folder
				if (CreateDirectoryA(current_dir, NULL)) {
					FindClose(findHandle);
					continue;	// try again to open the created directory
				}
			}

			ChangeColour(COLOR_LIGHT_RED);
			ShowText("錯誤：無法開啟資料夾 ", "Error: Cannot open folder ");
			cout << current_dir << endl;
			ChangeColour(COLOR_NORMAL);
			return ERROR_CODE;
		}
		else {
			break;
		}
	}

	do {
		// Skip "."
		if (strcmp(findFileData.cFileName, ".") != 0) {

			if (list_dir) {
				if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					// list ".." only when not in root directory
					if (strcmp(current_dir, PATH_DATABASE) == 0 &&
						strcmp(findFileData.cFileName, "..") == 0) {
						continue;
					}
					snprintf(MenuContext[menu_start_idx + number], MAXLINE, "%s\\", findFileData.cFileName);
					number += 1;
				}
				else {
					// skip non-directory filenames
					continue;
				}
			} else if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
				snprintf(MenuContext[menu_start_idx + number], MAXLINE, "%s", findFileData.cFileName);
				number += 1;
			}
		}
	} while (FindNextFileA(findHandle, &findFileData));

	FindClose(findHandle);

	return number;
}

int GetDirMenu(const char *current_dir, char MenuContext[][MAXLINE])
{
	int number = 0, list_num = 0;

	// list directory first
	list_num = ListPathFiles(current_dir, MenuContext, number, true);
	if(list_num == ERROR_CODE) {
		// error for the directory failed to open
		return ERROR_CODE;
	}
	number += list_num;

	if (number > 0)
		strncpy(MenuContext[number++], MENU_EMPTY_BLOCK_OPT, MAXLINE);

	// list other files
	list_num = ListPathFiles(current_dir, MenuContext, number);
	if(list_num == ERROR_CODE) {
		// error for the directory failed to open
		return ERROR_CODE;
	}
	number += list_num;

	// add an empty block before function options
	strncpy(MenuContext[number++], MENU_EMPTY_BLOCK_OPT, MAXLINE);

	return number;		//return the number of the total menus
}

int GetMenuFunctions(char MenuContext[][MAXLINE], int menu_start_idx)
{
	const char *menu_opt = NULL;

	switch(Programme_Language)
	{
		case Chinese:
			menu_opt = &Func_Menu_Chinese[0][0];
			break;

		case English:
			menu_opt = &Func_Menu_English[0][0];
			break;
	}

	if(menu_opt == NULL) {
		ChangeColour(COLOR_LIGHT_RED);
		cout << "Error occurred while generating menu function options!" << endl;
		ChangeColour(COLOR_NORMAL);
		return ERROR_CODE;
	}

	for (int i = 0; i < MAX_USER_FUNC_COUNT; i++) {
		strncpy(MenuContext[menu_start_idx + i], (menu_opt + i * MAXLINE), MAXLINE);
	}

	return MAX_USER_FUNC_COUNT;
}

int ReadQuestion(char Answer[][MAXLINE], char Question[][MAXLINE], char *filename)
{
	fstream QuizFile;
	int number = 0;
	char buffer[2 * MAXLINE], *pch;
	
	QuizFile.open(filename, ios::in);
	if(!QuizFile)
	{
		cout << endl << endl;
		ShowText("您所要開啟的檔案現在已經不存在！\n",
		 "The file you\'ve chosen does not exist now!\n");
		return ERROR_CODE;
	}
	else
	{
		ShowText("正在讀取檔案 ", "Reading the file ");
		cout << filename << " ...";

		//read and string tokenize every line of the file
		while(!QuizFile.eof())
		{
			QuizFile.getline(buffer, sizeof(buffer));
			//read the file line by line
			
			pch = strtok(buffer, split);
			if(pch == NULL)
			{
				continue;
			}
			else
				strcpy(Answer[number], pch);
			//record the answers
			
			pch = strtok(NULL, split);
			if(pch == NULL)
			{
				continue;
			}
			else
				strcpy(Question[number], pch);
			//record the questions

			number += 1;
		}
		QuizFile.close();
		ShowText("完成\n", "Done\n");
	}
	return number;
}

//functionalities
void About()
{
	ProgrammeTitle(); 
	ChangeColour(COLOR_LIGHT_GREEN);	//change to colour green
	
	ShowText("此程式致力於加強 PVQC 第一部分\n",
	 "This programme is dedicated to improving the first part of PVQC\n");
	ShowText("希望能藉由「學習金字塔」、「遺忘曲線」理論加強學習效果\n\n",
	 "Hope to achieve the goal by the theories of \"Learning Pyramid\" and "
	 "\"Forgetting Curve\"\n\n");
	ShowText("程式建立者：鄒秉宏 國立宜蘭大學 電機工程學系\n",
	 "Programme builder: Ping-Hung, Tsou from Department of Electrical Engineering, "
	 "National Ilan University, Taiwan\n");
	cout << "March 5th, 2018\n\n";
	
	//Show Common Creatives Dialog
	ShowCreativeCommonsDialog();
	
	ChangeColour(COLOR_LIGHT_BLUE);
	ShowText("按下任意鍵返回...", "Press any key to back to Menu...");
	getch();
	ChangeColour(COLOR_NORMAL);	//change to the original colour
}

void EnterLanguageSetup()
{
	ProgrammeTitle();	//show the title on the top of the programme
	MENU *menu = new MENU();
	
	ShowText("請選擇語言：", "Please select the language:");
	char items[][MAXLINE] = {"繁體中文", "English"};
	int selectnum = 0;
	
	menu->InitMenu(items, 0, 2, InitialY);
	selectnum = menu->StartChoose(0);	//start to choose from menu
	delete menu;
	
	fstream SetupOutput;
	SetupOutput.open("[Quiz].set", ios::out);
	if(!SetupOutput)
	{
		ChangeColour(COLOR_LIGHT_RED);
		ShowText("建立檔案時發生問題！\n\n", "Setup File cannot be built!\n\n");
		ChangeColour(COLOR_NORMAL);
		getch();
		return;
	}
	else
	{
		if(selectnum == 0)
			Programme_Language = Chinese;
		else
			Programme_Language = English;
		SetupOutput << Programme_Language;
		SetupOutput.close();
	}
}

void ColumnAdjustment(int Long)
{
	for(int i = 0; i < (7 - Long / 8); i++)
		cout << "\t";
}

void SearchOnMenu()
{
	ProgrammeTitle();
	
	fstream OpenFile;
	char Menu[MAXVALUE][MAXLINE], linebuffer[MAXLINE], Entemp[MAXLINE], Chtemp[MAXLINE],
	 search[30], *ptr, EnResult[MAXVALUE][MAXLINE], ChResult[MAXVALUE][MAXLINE];
	int number = 0, counter = 0;
	bool Right = false;
	OpenFile.open("[Quiz].qrc", ios::in);
	if(!OpenFile)
	{
		ShowText("MENU 檔案不存在！\n\n", "Menu file does not exist!\n\n");
		getch();
		return;
	}
	while(!OpenFile.eof())
	{
		OpenFile.getline(Menu[number], sizeof(Menu[number]));
		strcat(Menu[number], ".qz");
		number += 1;
	}
	OpenFile.close();
	
	ShowText("請輸入要從選單資料搜尋的字串：",
	 "Please Enter the string you want to search from the file on Menu: ");
	fflush(stdin);
	gets(search);
	
	ShowText("\n\n\t答案(英文)\t\t\t\t\t\t題目(中文)\n\n",
	 "\n\nAnswer\t(English)\t\t\t\t\t\tQuestion(Chinese)\n\n");

	for(int i = 0; i < number; i++)
	{
		OpenFile.open(Menu[i], ios::in);
		cout << Menu[i] << ": ";
		if(!OpenFile)
		{
			ShowText("檔案不存在！\n\n", "The file doesn\'t exist!\n\n");
			continue;
		}
		else
		{
			cout << "\n\n";			//The file does exist, show the Search result
			while(!OpenFile.eof())
			{
				OpenFile.getline(linebuffer, sizeof(linebuffer));
				ptr = strtok(linebuffer, split);
				if(ptr == NULL)
					continue;
				strcpy(Entemp, ptr);
				ptr = strtok(NULL, split);
				if(ptr == NULL)
					continue;
				strcpy(Chtemp, ptr);

				if(SearchString(Entemp, search) == true
				 || SearchString(Chtemp, search) == true)
				{
					cout << "\t" << Entemp;
					ColumnAdjustment(strlen(Entemp));
					cout << Chtemp << endl;
					
					if(counter < MAXVALUE)
					{
						strcpy(EnResult[counter], Entemp);
						strcpy(ChResult[counter], Chtemp);
					}
					counter += 1;
				}
				//if the search string fits one of the english or chinese string
				//then show on the screen
			}
			OpenFile.close();
			cout << endl;
		}
	}
	ShowText("總共搜尋到：", "Total: ");
	cout << counter;
	ShowText(" 筆資料\n\n", " counts\n\n");
	if(counter >= MAXVALUE)
	{ 
		ShowText("搜尋到的資料已經超過儲存上限，若儲存檔案不一定能儲存到所有搜尋結果\n\n",
		 "The number of results is greater than one file can be saved.\n"
		 "If you save the file, the part of the result would be missed.\n\n");
		counter = MAXVALUE - 1;
	}
	do
	{
		ShowText("是否要儲存搜尋結果為新檔案 (y/n)? ",
		 "Do you want to save the search result as a new file (y/n)? ");
		number = getch();
		cout << endl << endl;
	}while(toupper(number) != 'Y' && toupper(number) != 'N');
	if(toupper(number) == 'N')
		return;
	//if the user wants to save the result
	ShowText("請輸入新的檔案名稱：", "Please enter new file name: ");
	fflush(stdin);
	gets(linebuffer);
	strcat(linebuffer, ".qz");
	OpenFile.open(linebuffer, ios::out);
	if(!OpenFile)
	{
		ShowText("檔案無法建立！\n\n", "The file cannot be built!\n\n");
		getch();
		return;
	}
	for(int i = 0; i < counter; i++)
	{
		OpenFile << EnResult[i] << split << ChResult[i];
		if(i != counter - 1)
			OpenFile << endl;
	}
	OpenFile.close();
	linebuffer[strlen(linebuffer) - 3] = '\0';
	FileNameToMenu(linebuffer);
}

void AddOldFile()
{
	ProgrammeTitle();
	
	char filename[50];
	ShowText("請輸入檔案名稱 (.qz檔案)：",
	 "Please enter the filename with file name extension \"qz\": ");
	gets(filename);
	
	if(filename[strlen(filename) - 1] != 'z' && filename[strlen(filename - 2)] != 'q'
		&& filename[strlen(filename) - 3] != '.')
			strcat(filename, ".qz");	//if the filename does not contain the filename-ex
	
	fstream newfile;
	newfile.open(filename, ios::in);
	if(!newfile)
	{
		ShowText("檔案不存在！\n\n", "The file does not exist!\n\n");
		getch();
	}
	else
	{
		filename[strlen(filename) - 3] = '\0';	//get away the filename-ex
		FileNameToMenu(filename);
		newfile.close();
	}
}

void CreateNewLib()
{
	ProgrammeTitle();
	
	char filename[50];
	fstream NewQuizFile;
	
	ShowText("請輸入想創建檔案的檔名：", "Please enter the filename of a new file: ");
	gets(filename);
	//check if there is any file that does exist in the same directory
	strcat(filename, ".qz");
	NewQuizFile.open(filename, ios::in);
	if(NewQuizFile.is_open())		//the file does exist
	{
		NewQuizFile.close();
		ShowText("您想創建的檔案已經存在，請輸入新檔名或將舊檔刪除！\n",
		 "The file you want to create has already been existed.\n"
		 "Please enter the new filename or delete the old file!\n");
		getch();
		return;
	}
	filename[strlen(filename) - 2] = '\0';
	strcat(filename, "txt");
	NewQuizFile.open(filename, ios::out);
	if(!NewQuizFile)
	{
		ShowText("建立檔案時發生問題！\n\n", "Setup File cannot be built!\n\n");
		getch();
		return;
	}
	NewQuizFile.close();
	txtToqz(filename);

	filename[strlen(filename) - 3] = '\0';
	FileNameToMenu(filename);	//Add to the menu
}

void EditOnMenu(char Menu[][MAXLINE], int total)
{
	ProgrammeTitle();
	ShowText("從選單中選擇要編輯的檔案：", "Select which you want to edit on Menu:");
	
	MENU *menu = new MENU();
	int choose;
	char comrename[80] = "rename \"";
	menu->InitMenu(Menu, 0, total, InitialY, 7, 164);
	do
	{
		choose = menu->StartChoose(0);
	}while(choose < 0);
	delete menu;
	if(choose == ESCAPE)
		return;
	
	strcat(comrename, Menu[choose]);
	strcat(comrename, ".qz");
	strcat(comrename, "\" \"");
	strcat(comrename, Menu[choose]);
	strcat(comrename, ".txt\"");
	system(comrename);					//change qz file to txt file
	
	cout << endl;
	strcpy(comrename, Menu[choose]);
	strcat(comrename, ".txt");
	txtToqz(comrename);					//change txt file to qz file
}

void txtToqz(char *filename)	//transfer the ex-filename from txt to qz
{
	char comrename[80] = "\"";	//command rename prompt
	
	ChangeColour(COLOR_NORMAL);
	ShowText("在新增資料庫時，請依照規則：\n\n",
	 "\n\nWhile adding the new Quiz Library, please follow the rule:\n\n");
	ChangeColour(COLOR_WHITE);
	ShowText("英文單字(答案)=中文單字(題目)\n\n",
	 "English Words(Answer)=Chinese(Question)\n\n");
	ShowText("請使用字元：= 將兩者分割\n",
	 "Please use the character = to seperate the answer and the question\n");
	ShowText("舉例：\n\n", "For Example:\n\n");
	cout << "happy=n.快樂的\n\n";
	ShowText("題目將會長這樣：\n\n", "The Quiz would look like:\n\n");
	cout << "1. n.快樂的, 5 characters: _____\n\n";
	ChangeColour(COLOR_PURPLE);
	ShowText("完成題目後請儲存並關閉記事本，程式將會繼續執行...",
	 "After completed to build the library, please save and close Notepad.\n"
	 "The programme would run automatically...");
	ChangeColour(COLOR_NORMAL);
	
	strcat(comrename, filename);
	strcat(comrename, "\"");
	system(comrename);	//open the txt file to modify the quiz file
	
	strcpy(comrename, "rename \"");
	strcat(comrename, filename);
	strcat(comrename, "\" \"");
	filename[strlen(filename) - 3] = 'q';
	filename[strlen(filename) - 2] = 'z';
	filename[strlen(filename) - 1] = '\0';
	strcat(comrename, filename);
	strcat(comrename, "\"");
	
	system(comrename);
}

void DeleteOnMenu(char Menu[][MAXLINE], int chosen, int total)
{
	fstream MenuFile;
	MenuFile.open("[Quiz].qrc", ios::out);
	if(!MenuFile)
	{
		ShowText("檔案無法建立！", "The file cannot be built!");
		getch();
		return;
	}
	else
	{
		if(chosen == total - 1)			//delete the file in the bottom of the menu
		{
			for(int i = 0; i < total - 1; i++)
			{
				MenuFile << Menu[i];
				if(i != total - 2)
					MenuFile << endl;
			}
		}
		else
		{
			for(int i = 0; i < total; i++)
			{
				if(i == chosen)
					continue;			//skip the file should be deleted
				MenuFile << Menu[i];	//else write into the menu file
				if(i != total - 1)
					MenuFile << endl;	//after the last file, should not have new line
			}
		}
		MenuFile.close();
	}
}

bool SearchString(char *Source, char *Search)
{//Search String Search from Source string, if they have the part in common, return true
	if(strlen(Source) < strlen(Search))
		return false;
	for(int i = 0; i <= strlen(Source) - strlen(Search); i++)
	{
		for(int j = 0; j < strlen(Search); j++)
		{
			if(Source[i + j] != Search[j])
				break;
			else if(j == strlen(Search) - 1)
				return true;
		}
	}
	return false;
}

void FileNameToMenu(char *filename)
{
	fstream MenuInput;
	char Menu[MAXVALUE][MAXLINE];
	int totalnum = 0, EmptyMenu;
	bool AddThisFile = true;
	
	EmptyMenu = EmptyFile("[Quiz].qrc");
	MenuInput.open("[Quiz].qrc", ios::in);
	if(!MenuInput)
	{
		ShowText("檔案不存在！", "The file is not exist");
		getch();
		return;
	}
	while(!MenuInput.eof())
	{
		MenuInput.getline(Menu[totalnum], sizeof(Menu[totalnum]));
		totalnum += 1;
	}
	for(int i = 0; i < totalnum; i++)	//to see if there is identical file
	{
		if(strcmp(Menu[i], filename) == 0)
		{
			AddThisFile = false;
			break;
		}
	}
	if(AddThisFile == true)
	{
		MenuInput.close();
		fstream OutputMenu;
		OutputMenu.open("[Quiz].qrc", ios::out);
		if(!OutputMenu)
		{
			ShowText("檔案無法建立！", "Menu Cannot be established!");
		}
		else
		{
			OutputMenu << filename;
			if(EmptyMenu == 0)	//if the menu is not empty
			{
				OutputMenu << endl;
				for(int i = 0; i < totalnum; i++)
				{
					OutputMenu << Menu[i];
					if(i < totalnum - 1)
						OutputMenu << endl;
				}
			}
			OutputMenu.close();
		}
	}
}

void BreakOrder(char EN[][MAXLINE], char CH[][MAXLINE], int total)
{	//Break the original order
	ShowText("\n正在打亂順序...", "\nBreaking the original order...");

	unsigned int seed = (unsigned)time(NULL);
	char linebuffer[MAXLINE];
	int number;
	srand(seed);	//random seed
	
	for(int i = 0; i < total; i++)
	{
		number = rand() % total;
		if(number == i)
			continue;
		else
		{
			strcpy(linebuffer, EN[i]);
			strcpy(EN[i], EN[number]);
			strcpy(EN[number], linebuffer);
	
			strcpy(linebuffer, CH[i]);
			strcpy(CH[i], CH[number]);
			strcpy(CH[number], linebuffer);
			//exchange two questions' order
		}
	}
	ShowText("完成\n\n", "Done\n\n");
}

void StartTesting(char Answer[][MAXLINE], char Question[][MAXLINE],
	int total, bool histmode)
{
	int choosenum;						//how many questions do the user want to answer
	char buffer[MAXLINE];
	int wrong = 0;						//how many questions answers incorrectly
	short int WrongRecord[MAXVALUE];	//record which question answers incorrectly
	float result;

ReStart:
	ShowText("請輸入您想要測驗的題數 (輸入 * 或留白代表全部) ",
	 "Please enter how much questions you want to answer (enter * or leave blank represents ALL) ");
	cout << "(<= " << total << "): ";
	fflush(stdin);
	gets(buffer);
	cout << endl;

	// trim the buffer
	if(trim(buffer) == NULL)
	{
		ShowText("輸入錯誤，請重新輸入！\n", "Input error, please re-enter!\n");
		goto ReStart;
	}

	if(strcmp(buffer, "*") == 0 || strlen(buffer) == 0)	//all questions
		choosenum = total;
	else
	{
		choosenum = atoi(buffer);	//convert the input to the number
		if(choosenum > total || choosenum <= 0)
			goto ReStart;
	}

	for(int i = 0; i < choosenum; i++)
	{
		cout << (i + 1) << ". " << Question[i] << " , " << strlen(Answer[i]);
		ShowText(" 個字母: ", " characters: ");
		EnteringAnswer(strlen(Answer[i]), buffer);
		if(strcmp(buffer, Answer[i]) == 0)	//compare the input and the answer
		{							//if the two are identical
			ChangeColour(COLOR_LIGHT_GREEN);
			ShowText("\n恭喜您 答對了！\n\n", "\nCongratulation! Correct answer!\n\n");
		}
		else						//Wrong
		{
			ChangeColour(COLOR_LIGHT_RED);
			ShowText("\n答錯了！正確答案是：", "\nIncorrect! Correct answer is: ");
			ChangeColour(COLOR_LIGHT_GREEN);
			cout << Answer[i] << endl << endl;

			WrongRecord[wrong++] = i;	//record the question number
		}
		ChangeColour(COLOR_NORMAL);
	}

	ShowText("本次測驗的正確率為：", "The correctness ratio of this quiz is: ");
	result = ((float)choosenum - (float)wrong) / (float)choosenum * 100;
	cout << result << "%" << endl << endl;

	if(wrong > 0 && histmode == false)	//if something wrong has been made
	{
		AddToHistory(Answer, Question, WrongRecord, wrong);
		RefreshHistory();
	}
	else if(histmode == true)
		RebuildHistory(Answer, Question, WrongRecord, wrong);
}

int EnteringAnswer(int wordlength, char *rewords)
{
	char answer[MAXLINE];
	int wordcursor = 0, storage = 0;
	unsigned char keybuffer;
	EAInit(wordlength);
	
	while(keybuffer = getch())
	{
		if((int)keybuffer == 27)
		{	//clear the entering line and buffer
			for(int i = 0; i < wordcursor; i++)	//let the cursor be the head of the line
				cout << '\b';
			for(int i = 0; i < wordcursor; i++)
				cout << ' ';
			for(int i = 0; i < wordcursor; i++)
				cout << '\b';
			wordcursor = 0;
			EAInit(wordlength);					//empty the line
			strcpy(answer, "\0");
			storage = 0;
		}
		else if((int)keybuffer == 8)
		{	//if the key backspace is pressed
			if(wordcursor > 0 && wordcursor <= wordlength)
				cout << "\b_\b";
			//the answer letters are greater than correct ones
			else if(wordcursor > wordlength)
				cout << "\b \b";
			if(wordcursor > 0)
			{
				if(wordcursor < storage)
				{	//the cursor is in the middle of the word
					memmove(answer + wordcursor - 1, answer + wordcursor,
					 storage - wordcursor);
					for(int i = wordcursor; i < storage; i++)
						cout << answer[i - 1];
					
					if(storage <= wordlength)
						cout << '_';
					else
						cout << ' ';
					
					for(int i = wordcursor; i < storage + 1; i++)
						cout << '\b';
				}
				wordcursor -= 1;
				storage -= 1;
			}
		}
		else if(keybuffer == 224)//if any directional key is pressed
		{
			keybuffer = getch();
			if((int)keybuffer == 75 && wordcursor > 0)//if the left key is pressed
			{
				wordcursor -= 1;
				cout << '\b';
			}
			else if((int)keybuffer == 77 && wordcursor < storage)
			{	//if the right key is pressed
				cout << answer[wordcursor];
				wordcursor += 1;
			}
			else if((int)keybuffer == 83 && storage > 0 && wordcursor < storage - 1)
			{	//if the key delete is pressed
				storage -= 1;
				if(storage - 1 == wordcursor)
				{
					cout << answer[wordcursor];
					if(wordcursor < wordlength)
						cout << '_';
					else
						cout << ' ';
					cout << "\b\b";
				}
				else
				{
					memmove((answer + wordcursor + 1), (answer + wordcursor + 2),
					 (storage - wordcursor - 1));
					
					for(int i = wordcursor; i < storage; i++)
						cout << answer[i];
					
					if(storage < wordlength)
						cout << '_';
					else
						cout << ' ';
					
					for(int i = wordcursor; i <= storage; i++)
						cout << '\b';
				}
			}
			else if((int)keybuffer == 71)//if the key home is pressed
			{
				for(int i = 0; i < wordcursor; i++)
					cout << '\b';
				wordcursor = 0;
			}
			else if((int)keybuffer == 79)//if the key end is pressed
			{
				for(int i = wordcursor; i < storage; i++)
					cout << answer[i];
				wordcursor = storage;
			}
		}
		else if((int)keybuffer == 13)//if the key Enter is pressed
			break;
		else//key in other English letter
		{
			cout << keybuffer;
			if(storage > wordcursor)
			{
				memmove(answer + wordcursor + 1, answer + wordcursor, storage - wordcursor);
				answer[wordcursor] = keybuffer;
				storage += 1;
				wordcursor += 1;
				for(int i = wordcursor; i < storage; i++)
					cout << answer[i];
				for(int i = wordcursor; i < storage; i++)
					cout << '\b';
			}
			else
			{
				answer[wordcursor] = keybuffer;
				wordcursor += 1;
				storage += 1;
			}
		}
	}
	answer[storage] = '\0';
	strcpy(rewords, answer);	//copy the answer to the rewords and return
	return 1;
}

void EAInit(int numbers)
{
	for(int i = 0; i < numbers; i++)
		cout << '_';
	for(int i = 0; i < numbers; i++)
		cout << '\b';
}

void AddToHistory(char En[][MAXLINE], char Ch[][MAXLINE],
	short int WrongRecord[], int total)
{
	ShowText("正在將錯誤存入歷史檔案...", "Saving the incorrect answer to History file...");
	
	fstream HisOutput;
	int HisEmpty = EmptyFile("[History].qzc");
	HisOutput.open("[History].qzc", ios::out | ios::app);
	if(!HisOutput)
	{
		ShowText("歷史檔案無法開啟！\n", "History file failed to open!\n");
		getch();
		return;
	}
	if(HisEmpty == 0)			//When the file is not empty, change to the next line
		HisOutput << endl;
	for(int i = 0; i < total; i++)
	{
		HisOutput << En[WrongRecord[i]] << split << Ch[WrongRecord[i]];
		if(i != total - 1)
			HisOutput << endl;
	}
	HisOutput.close();
	
	ShowText("完成！\n\n", "Done\n\n");
}

void RefreshHistory()
{
	ShowText("正在整理歷史檔案...", "Refreshing History file...");
	
	fstream HisFile;
	char buffer[MAXVALUE][2 * MAXLINE];
	int num = 0;
	//read the file into the memory
	HisFile.open("[History].qzc", ios::in);
	if(!HisFile)
		return;		//the file is missing
	while(!HisFile.eof())
	{
		HisFile.getline(buffer[num], sizeof(buffer[num]));
		num += 1;
	}
	HisFile.close();
	//start to compare if there is any line that is identical to the other one
	for(int i = 0; i < num; i++)
	{
		if(buffer[i][0] == '\0')
			continue;
		for(int j = i + 1; j < num; j++)
		{
			if(strcmp(buffer[i], buffer[j]) == 0)
				buffer[j][0] = '\0';
			//if there is one that identical to the other, eliminate the latter
		}
	}
	HisFile.open("[History].qzc", ios::out);
	if(!HisFile)
		return;
	for(int i = 0; i < num; i++)
	{
		if(buffer[i][0] != '\0')
			HisFile << buffer[i] << endl;
	}
	HisFile.close();
	ShowText("完成！\n\n", "Done\n\n");
}

void RebuildHistory(char En[][MAXLINE], char Ch[][MAXLINE],
	short int WrongRecord[], int total)
{
	ShowText("正在重新建立歷史檔案...", "Rebuilding History file...");
	
	fstream HistOut;
	HistOut.open("[History].qzc", ios::out);
	if(!HistOut)
		return;
	if(total != 0)
	{
		for(int i = 0; i < total; i++)
		{
			HistOut << En[WrongRecord[i]] << split << Ch[WrongRecord[i]];
			if(i != total - 1)
				HistOut << endl;
		}
	}
	HistOut.close();
	ShowText("完成！\n\n", "Done\n\n");
}
