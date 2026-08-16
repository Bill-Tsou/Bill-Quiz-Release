#ifndef QUIZ_RELEASE_H

#include <windows.h>
#include <string.h>
#include <conio.h>
#include <time.h>

#include "Menu.h"
#include "CreativeCommons.h"

/* ==== definitions ==== */
#define InitialY 4		//define how many lines of the title of the programme would occupy
#define PATH_DATABASE "database"	// init database path

/* ======== */

/* ==== functions menu list ==== */
enum UserFunctions {
	USER_FUNC_HISTORY_PRACTICE = 0,
	USER_FUNC_OPEN_DIRECTORY,
	USER_FUNC_LIB_CREATE,
	USER_FUNC_LIB_EDIT,
	USER_FUNC_LIB_SEARCH,
	USER_FUNC_HISTORY_CLEAR,
	USER_FUNC_LANGUAGE,
	USER_FUNC_ABOUT,

	MAX_USER_FUNC_COUNT
};

const char Func_Menu_Chinese[][MAXLINE] = {
	" /歷史錯誤練習",
	" /開啟目前資料庫位置",
	" /新增資料庫",
	" /編輯資料庫",
	" /搜尋資料庫",
	" /清空歷史檔案",
	" /語言設定 Language",
	" /關於"
};

const char Func_Menu_English[][MAXLINE] = {
	" /History Practice",
	" /Open Directory of Current Library",
	" /Create New Library",
	" /Edit Library",
	" /Search in Library",
	" /Clear History File",
	" /Language Setup",
	" /About this Programme"
};

/* ======== */

/* ==== function prototypes ==== */
//show the title on the top of the programme
void ProgrammeTitle();
//determine whether or not the file is empty
int EmptyFile(char*);
// custom function for trimming the string
char *trim(char *str);

//Information collection
int ReadSetup();			//read which language is the programme saved
int ListPathFiles(const char *current_dir, char MenuContext[][MAXLINE], int menu_start_idx, bool list_dir = false);
int GetDirMenu(const char *current_dir, char MenuContext[][MAXLINE]);
int GetMenuFunctions(char MenuContext[][MAXLINE], int menu_start_idx);
int ReadQuestion(char[][MAXLINE], char[][MAXLINE], char*);
						//open the quiz file(.qz) and return the total questions number

//Functions on the menu
void EnterLanguageSetup();			//modify the language or menu file
void SearchOnMenu();		//show the result of any words that contains the key words
void CreateNewLib();		//create the new quiz file(.qz)
void AddOldFile();			//add the old file into the menu
void About();				//show something about this programme
void DeleteOnMenu(char[][MAXLINE], int, int);	//delete file on menu
void EditOnMenu(char[][MAXLINE], int);			//edit file on menu

void txtToqz(char*);
void FileNameToMenu(char*);	//add the filename to the menu and check if there is any repeats
bool SearchString(char*, char*);	//search string from the source string

//Quiz
void BreakOrder(char[][MAXLINE], char[][MAXLINE], int);
void StartTesting(char[][MAXLINE], char[][MAXLINE], int, bool = false);
int EnteringWords(char*, int);
int EnteringAnswer(int, char*);
void EAInit(int);
//with what have been typed and the total number of the questions

//After the Quiz
void AddToHistory(char[][MAXLINE], char[][MAXLINE], short int[], int);
	//add the wrong question to the history file
void RefreshHistory();
	//check the history file if there is any questions that repeated
void RebuildHistory(char[][MAXLINE], char[][MAXLINE], short int[], int);
	//eliminate what in the history file when user answers correctly

//Show the text according to programme language
void ShowText(const char*, const char*);

/* ======== */

#endif // QUIZ_RELEASE_H