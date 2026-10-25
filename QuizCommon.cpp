#include <iostream>
#include <fstream>
#include <dirent.h>
#include <strings.h>
#include <sys/stat.h>

#include "QuizCommon.h"

using namespace std;

// retrieve language variable defined in QuizRelease.cpp
extern int Programme_Language;
const char split[10] = "=";

int EmptyFile(const char *filename)
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

void PauseConsole()
{
	ShowText("按下任意鍵繼續...", "Press any key to continue...");
	getch();
	cout << endl;
}

void ProgrammeTitle()
{
	ClearScreen();		//clear the console interface
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
			return ReadSetup();
		}
	}
	else
	{
		Setup.getline(buffer, sizeof(buffer));
		Programme_Language = atoi(buffer);
		Setup.close();
	}
	return 0;
}

// compare two menu entries in alphabetical order (case-insensitive), ".." is always the first one
static int CompareMenuEntry(const void *a, const void *b)
{
	const char *str_a = (const char *)a, *str_b = (const char *)b;

	if (strcmp(str_a, "../") == 0)
		return -1;
	if (strcmp(str_b, "../") == 0)
		return 1;
	return strcasecmp(str_a, str_b);
}

int ListPathFiles(const char *current_dir, char MenuContext[][MAXLINE], int menu_start_idx, bool list_dir)
{
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char fullPath[MAX_PATH];
	int number = 0;

	dir = opendir(current_dir);
	if (dir == NULL && mkdir(current_dir, 0755) == 0) {
		// try again to open the created directory
		dir = opendir(current_dir);
	}

	if (dir == NULL) {
		ChangeColour(COLOR_LIGHT_RED);
		ShowText("錯誤：無法開啟資料夾 ", "Error: Cannot open folder ");
		cout << current_dir << endl;
		ChangeColour(COLOR_NORMAL);
		return ERROR_CODE;
	}

	while ((entry = readdir(dir)) != NULL) {
		// Skip "." and hidden files such as .DS_Store, but keep ".."
		if (entry->d_name[0] == '.' && strcmp(entry->d_name, "..") != 0)
			continue;

		// leave the space for the empty blocks and the functional options
		if (menu_start_idx + number >= MAXVALUE - MAX_USER_FUNC_COUNT - 2)
			break;

		snprintf(fullPath, sizeof(fullPath), "%s/%s", current_dir, entry->d_name);
		if (stat(fullPath, &entry_stat) != 0)
			continue;

		if (list_dir) {
			if (S_ISDIR(entry_stat.st_mode)) {
				// list ".." only when not in root directory
				if (strcmp(current_dir, PATH_DATABASE) == 0 &&
					strcmp(entry->d_name, "..") == 0) {
					continue;
				}
				snprintf(MenuContext[menu_start_idx + number], MAXLINE, "%s/", entry->d_name);
				number += 1;
			}
		} else if (S_ISREG(entry_stat.st_mode)) {
			snprintf(MenuContext[menu_start_idx + number], MAXLINE, "%s", entry->d_name);
			number += 1;
		}
	}

	closedir(dir);

	// readdir() returns the entries in no particular order
	qsort(MenuContext[menu_start_idx], number, sizeof(MenuContext[0]), CompareMenuEntry);

	return number;
}

int ListQzFilesRecursive(const char *dir_path, char FilePaths[][MAX_PATH], int count)
{
	DIR *dir;
	struct dirent *entry;
	struct stat entry_stat;
	char fullPath[MAX_PATH];
	int name_len;

	dir = opendir(dir_path);
	if (dir == NULL)
		return count;	// cannot open the directory, just skip it

	while ((entry = readdir(dir)) != NULL) {
		// Skip ".", ".." and hidden files
		if (entry->d_name[0] == '.')
			continue;

		snprintf(fullPath, MAX_PATH, "%s/%s", dir_path, entry->d_name);
		if (stat(fullPath, &entry_stat) != 0)
			continue;

		if (S_ISDIR(entry_stat.st_mode)) {
			count = ListQzFilesRecursive(fullPath, FilePaths, count);	// recurse into sub-directory
		}
		else if (S_ISREG(entry_stat.st_mode)) {
			name_len = strlen(entry->d_name);
			if (name_len > 3 && strcmp(entry->d_name + name_len - 3, ".qz") == 0
				&& count < MAXVALUE) {
				strncpy(FilePaths[count], fullPath, MAX_PATH);
				count += 1;
			}
		}
	}

	closedir(dir);

	return count;
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

int ReadQuestion(char Answer[][MAXLINE], char Question[][MAXLINE], const char *filename)
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
			buffer[strcspn(buffer, "\r")] = '\0';	//remove CR of Windows line ending
			
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
	 "Programme builder: Bill Tsou from Department of Electrical Engineering, "
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
	char FilePaths[MAXVALUE][MAX_PATH], linebuffer[MAXLINE], Entemp[MAXLINE], Chtemp[MAXLINE],
	 search[30], *ptr, EnResult[MAXVALUE][MAXLINE], ChResult[MAXVALUE][MAXLINE];
	int number = 0, counter = 0;

	// walk the whole database directory tree to collect every library file
	number = ListQzFilesRecursive(PATH_DATABASE, FilePaths, 0);
	if(number == 0)
	{
		ChangeColour(COLOR_LIGHT_YELLOW);
		ShowText("資料庫是空的！沒有辦法搜尋關鍵字\n\n", "Database is empty! Cannot search for keywords\n\n");
		ChangeColour(COLOR_NORMAL);
		getch();
		return;
	}

	ShowText("請輸入要從資料庫中搜尋的字串：",
	 "Please Enter the string you want to search from the database: ");
	ReadLine(search, sizeof(search));

	ShowText("\n\n\t答案(英文)\t\t\t\t\t\t題目(中文)\n\n",
	 "\n\nAnswer\t(English)\t\t\t\t\t\tQuestion(Chinese)\n\n");

	for(int i = 0; i < number; i++)
	{
		OpenFile.open(FilePaths[i], ios::in);
		if(!OpenFile)
		{
			cout << FilePaths[i] << ": ";
			ShowText("檔案不存在！\n\n", "The file doesn\'t exist!\n\n");
			continue;
		}

		bool file_header_shown = false;	// only show the filename once it has a match
		while(!OpenFile.eof())
		{
			OpenFile.getline(linebuffer, sizeof(linebuffer));
			linebuffer[strcspn(linebuffer, "\r")] = '\0';	//remove CR of Windows line ending
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
				if(file_header_shown == false)
				{
					cout << FilePaths[i] << ":\n\n";
					file_header_shown = true;
				}

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
		if(file_header_shown == true)
			cout << endl;
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
	else if(counter == 0)
	{
		ShowText("按任意鍵返回...", "Press any keys to go back...");
		getch();
		return;
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
	ReadLine(linebuffer, sizeof(linebuffer));

	char save_path[MAX_PATH];
	snprintf(save_path, sizeof(save_path), "%s/%s.qz", PATH_DATABASE, linebuffer);
	OpenFile.open(save_path, ios::out);
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
}

void CreateNewLib(const char *current_dir)
{
	ProgrammeTitle();
	
	char filename[MAX_PATH] = {0}, new_filename[120] = {0};
	fstream NewQuizFile;
	
	ShowText("請輸入想創建檔案的檔名：", "Please enter the filename of a new file: ");
	ReadLine(new_filename, sizeof(new_filename));
	//check if there is any file that does exist in the same directory
	snprintf(filename, sizeof(filename), "%s/%s.qz", current_dir, new_filename);
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
}

void EditOnMenu(const char *current_dir, char Menu[][MAXLINE], int total)
{
	ProgrammeTitle();
	ShowText("從選單中選擇要編輯的檔案：", "Select which you want to edit on Menu:");

	int menu_edit_total = 0;
	char Menu_Edit[MAXVALUE][MAXLINE] = {{0}};

	// filter out all directories and blocks, only keep the filenames for edit
	for (int i = 0; i < total; i++)
	{
		if ((Menu[i][strlen(Menu[i]) - 1] == '/') ||
			(strcmp(Menu[i], MENU_EMPTY_BLOCK_OPT) == 0))
		{
			continue;
		}

		strcpy(Menu_Edit[menu_edit_total++], Menu[i]);
	}
	
	MENU *menu = new MENU();
	int choose;
	int tmp_strlen = 0;
	char selected_filename[MAX_PATH] = {0}, text_filename[MAX_PATH] = {0};
	menu->InitMenu(Menu_Edit, 0, menu_edit_total, InitialY, 7, 164);
	do
	{
		choose = menu->StartChoose(0);
	}while(choose < 0);
	delete menu;
	if(choose == ESCAPE)
		return;

	snprintf(selected_filename, sizeof(selected_filename), "%s/%s", current_dir, Menu_Edit[choose]);
	strcpy(text_filename, selected_filename);

	tmp_strlen = strlen(text_filename);
	if ((tmp_strlen > 3) &&
		(text_filename[tmp_strlen - 1] == 'z') &&
		(text_filename[tmp_strlen - 2] == 'q') &&
		(text_filename[tmp_strlen - 3] == '.')
	)
	{
		strcpy(text_filename + tmp_strlen - 2, "txt");
	}

	if (rename(selected_filename, text_filename) != 0)	//change qz file to txt file
	{
		ChangeColour(COLOR_LIGHT_RED);
		ShowText("\n無法重新命名檔案 ", "\nCannot rename the file ");
		cout << selected_filename << endl;
		ChangeColour(COLOR_NORMAL);
		getch();
		return;
	}
	
	cout << endl;
	txtToqz(text_filename);			//change txt file to qz file
}

void txtToqz(char *filename)	//transfer the ex-filename from txt to qz
{
	char cmd_open[MAX_PATH + 100] = {0};	//command to open the editor
	char qz_filename[MAX_PATH] = {0};
	int len_fn = 0;

	cout << endl;
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
#ifdef __APPLE__
	ShowText("完成題目後請儲存並結束文字編輯 (⌘Q)，程式將會繼續執行...",
	 "After completed to build the library, please save and quit TextEdit (Cmd+Q).\n"
	 "The programme would run automatically...");
	ChangeColour(COLOR_NORMAL);

	//open the txt file with TextEdit and wait until TextEdit quits
	snprintf(cmd_open, sizeof(cmd_open), "open -W -e \"%s\"", filename);
	system(cmd_open);
#else
	ShowText("完成題目後請儲存並關閉文字編輯器，再按下任意鍵繼續...",
	 "After completed to build the library, please save and close the text editor,\n"
	 "then press any key to continue...");
	ChangeColour(COLOR_NORMAL);

	//open the txt file with the default text editor, xdg-open returns immediately
	snprintf(cmd_open, sizeof(cmd_open), CMD_OPEN " \"%s\" >/dev/null 2>&1", filename);
	system(cmd_open);
	getch();
#endif
	cout << endl;

	//change the extension from .txt to .qz
	strcpy(qz_filename, filename);
	len_fn = strlen(qz_filename);
	if (len_fn > 4 && strcmp(qz_filename + len_fn - 4, ".txt") == 0)
		strcpy(qz_filename + len_fn - 3, "qz");

	if (rename(filename, qz_filename) != 0)
	{
		ChangeColour(COLOR_LIGHT_RED);
		ShowText("無法重新命名檔案 ", "Cannot rename the file ");
		cout << filename << endl;
		ChangeColour(COLOR_NORMAL);
		getch();
	}
}

void DeleteOnMenu(const char *current_dir, char Menu[][MAXLINE], int chosen, int total)
{
	char file_path[MAX_PATH] = {0}, user_input = '\0';
	snprintf(file_path, sizeof(file_path), "%s/%s", current_dir, Menu[chosen]);

	ProgrammeTitle();

	// check if the selected option is folder, the folder cannot be deleted
	if (file_path[strlen(file_path) - 1] == '/') {
		ChangeColour(COLOR_LIGHT_GREEN);
		cout << " " << file_path;
		ShowText(" 是資料夾，無法被刪除！", " is a folder, cannot be deleted!");
		ChangeColour(COLOR_NORMAL);

		cout << endl << endl;
		ShowText("按下任意鍵繼續...", "Press any keys to continue...");
		getch();
		return;
	}

	ChangeColour(COLOR_LIGHT_BLUE);

	ShowText("檔案 ", "File ");
	cout << file_path;
	ShowText(" 將要被刪除，刪除後將無法復原，確定要繼續嗎 (Y/n) ? ",
		" is about to be deleted, file could not be recovered once deleted.\n"
		"Are you sure to continue (Y/n) ? ");

	ChangeColour(COLOR_NORMAL);

	do {
		char tmp_input = getch();
		// default option is Y
		if ((unsigned int)tmp_input == 13)
			tmp_input = 'Y';
		user_input = toupper(tmp_input);
	} while((user_input != 'Y') && (user_input != 'N'));

	cout << endl << endl;

	if (user_input == 'N')	// cancel file deletion by user
		return;

	if (remove(file_path) != 0) {
		ChangeColour(COLOR_LIGHT_RED);
		ShowText("檔案刪除失敗! 按任意鍵繼續...", "Failed to delete the file! Press any keys to continue...");
		ChangeColour(COLOR_NORMAL);
		getch();
		return;
	}

	ShowText("檔案已經刪除! 按任意鍵繼續...", "The file has been deleted! Press any keys to continue...");
	getch();
}

bool SearchString(char *Source, char *Search)
{//Search String Search from Source string, if they have the part in common, return true
	int source_len = strlen(Source), search_len = strlen(Search);
	if(source_len < search_len)
		return false;
	for(int i = 0; i <= source_len - search_len; i++)
	{
		for(int j = 0; j < search_len; j++)
		{
			if(Source[i + j] != Search[j])
				break;
			else if(j == search_len - 1)
				return true;
		}
	}
	return false;
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
	int ret_answer = 0;					// return value of EnteringAnswer()
	int real_ques_num = 0;				// the real question number that user answered
	short int WrongRecord[MAXVALUE];	//record which question answers incorrectly
	float result;

ReStart:
	ShowText("請輸入您想要測驗的題數 (輸入 * 或留白代表全部) ",
	 "Please enter how much questions you want to answer (enter * or leave blank represents ALL) ");
	cout << "(<= " << total << "): ";
	ReadLine(buffer, sizeof(buffer));
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

	// show the question number for quiz and hit messages
	ChangeColour(COLOR_LIGHT_BLUE);
	ShowText(" === 測驗共 ", "There are ");
	cout << choosenum;
	ShowText(" 題，按下 Ctrl+C 放棄答題", " questions in quiz. Press Ctrl+C to quit the quiz.");
	cout << " ===" << endl << endl;
	ChangeColour(COLOR_NORMAL);

	// initialize variables
	ret_answer = 0;
	real_ques_num = 0;

	for(int i = 0; i < choosenum; i++)
	{
		cout << (i + 1) << ". " << Question[i] << " , " << strlen(Answer[i]);
		ShowText(" 個字母: ", " characters: ");
		ret_answer = EnteringAnswer(strlen(Answer[i]), buffer);
		if(ret_answer == -1)		// user quits the question
		{
			ChangeColour(COLOR_LIGHT_YELLOW);
			cout << endl << endl;
			ShowText("放棄本次測驗！", "The quiz was quit!");
			cout << endl << endl;
			ChangeColour(COLOR_NORMAL);
			break;
		}
		else
			real_ques_num += 1;

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

	if (real_ques_num > 0)
	{
		ShowText("本次測驗的正確率為：", "The correctness ratio of this quiz is: ");
		result = (float)(real_ques_num - wrong) / (float)real_ques_num * 100.0f;
		cout << result << "%" << endl << endl;
	}
	else
	{
		ShowText("放棄測驗所有出題，無法計算正確率！", "All questions in the test were quit, the correctness cannot be calculated!");
		cout << endl << endl;
	}

	if(wrong > 0 && histmode == false)	//if something wrong has been made
	{
		AddToHistory(Answer, Question, WrongRecord, wrong);
		RefreshHistory();
	}
	else if(histmode == true)
	{
		// add the non-selected questions to WrongRecord for keeping them in history file
		if (choosenum < total)
		{
			for (int i = choosenum; i < total; i++)
				WrongRecord[wrong++] = i;
		}
		RebuildHistory(Answer, Question, WrongRecord, wrong);
	}
}

int EnteringAnswer(int wordlength, char *rewords)
{
	char answer[MAXLINE];
	int wordcursor = 0, storage = 0;
	unsigned char keybuffer;
	EAInit(wordlength);
	
	while((keybuffer = getch()))
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
		else if((int)keybuffer == 3)// if Ctrl+C was pressed
			return -1;
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
		buffer[num][strcspn(buffer[num], "\r")] = '\0';	//remove CR of Windows line ending
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
