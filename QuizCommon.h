#ifndef QUIZ_RELEASE_H

#include <windows.h>
#include <string.h>
#include <conio.h>
#include <time.h>

#include "Menu.h"
#include "CreativeCommons.h"

/* ==== definitions ==== */
#define InitialY 4		//define how many lines of the title of the programme would occupy

/* ======== */

/* ==== function prototypes ==== */
//standard change console colour function
void ChangeColour(int colour);
//show the title on the top of the programme
void ProgrammeTitle();
//determine whether or not the file is empty
int EmptyFile(char*);

//Information collection
int ReadSetup();			//read which language is the programme saved
int GetTheMenu(char[][MAXLINE]);
int ReadQuestion(char[][MAXLINE], char[][MAXLINE], char*);
						//open the quiz file(.qz) and return the total questions number

//Functions on the menu
void EnterSetup();			//modify the language or menu file
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