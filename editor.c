/* My own vi-like editor for windows*/
/* Date: 07 Oct 2007 */
/* Author: Srinivas Nayak, Pradipta Kumar Satapathy */
/* Compile using Turbo C++ */

/* This code is distributed under the GPL License. */
/* For more info check: */
/* http://www.gnu.org/copyleft/gpl.html */


#include<stdio.h>
#include<conio.h>
#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include<dos.h>

#define TOTAL_DISPLAY_LINE_NUMBER 24
#define TOPMOST_DISPLAY_LINE_NUMBER 0
#define BOTTOMMOST_DISPLAY_LINE_NUMBER 23


#define TOTAL_DISPLAY_COLUMN_NUMBER 80
#define LEFTMOST_DISPLAY_COLUMN_NUMBER 0
#define RIGHTMOST_DISPLAY_COLUMN_NUMBER 79

#define NEWLINE 0xFF
#define TAB 0xEE
#define PADDING 0xDD
#define SPACE 0x20
/******************************************************/

struct line_node
{
	int  display_line_num;
	int  text_length;
	char text[TOTAL_DISPLAY_COLUMN_NUMBER];//one extra for '\n' character.
	struct line_node * prev;
	struct line_node * next;
};
typedef struct line_node node;

enum
{
	ESCAPE_MODE,
	INSERT_MODE,
	COLON_MODE
};

struct position
{
	int row;
	int col;
};
/********** GLOBAL VARIABLE DECLARATIONS **************/

FILE *sourcefile;//file pointers to source files.
char sourcefilename[20]; //array to hold source file name.
char statusbuffer[1][20]; //this holds the things
						  //that are displayed in the bottommost status line of editor

node * start_display_node;
node * current_node;
node *head= NULL, *tail = NULL;
int editor_mode = ESCAPE_MODE;

//***************END OF GLOBAL DECLARATIONS************************

//******************FUNCTION DECLARATIONS********************************************

// when the editor screen either first loaded or pressing escape in all the modes, the user will
// do all the operations in escape mode. e.g: deleting characters, deleting the current line..etc
void edit_in_escape_mode();

// After pressing the character 'i' in the escape mode will take into the insert mode. In this mode
// user can edit as many chacters into the file.
void edit_in_insert_mode();

// When pressing ESC followed by character ':' in the editor will take the editor into colon mode.
// In colon mode, user can save ('w') or quit (q) the file.
void edit_in_colon_mode();

// This function will be useful only in the escape mode. This function will be responsible for delete
// the current line. This functionality can be done by pressing 'dd'
void delete_current_line();

// This function will be useful only in the escape mode. This function will be responsible for delete
// the current line. This functionality can be done by pressing 'x'
void delete_current_character();

node * create_node();
void parse_from_file();
void editor_initialize();
void flush_on_screen();
void show_editor();
void show_cursor();
void reverse_parse_into_file();
void parse_from_file();
void save_file();
void insert(int,char *);
void delete_node();
void move_displayed_data_one_line_down();
void move_cursor_up();
void move_displayed_data_one_line_up();
void move_cursor_down();
void move_cursor_left();
void move_cursor_right();
void quit_file();
void stringcopy(char *, char *);
void create_empty_node();
// Each line is terminated with a CARRIAGE RETURN character,
// so we need to calculate how many characters in a node.
int get_string_length();
//****************END OF FUNCTION DECLARATIONS ******************************


int main(int argc,char *argv[])
{
	int flag=1;
	clrscr(); //before doing anything, clear the screen.

//***************file open operations**************
	if(argc==2)// if user provides a source file name...
	{
		strncpy(sourcefilename,argv[1],19); //max sourcefilename assumed is 19.
		// try to open that file to read and update.
		sourcefile=fopen(sourcefilename,"r+");
		if(sourcefile==NULL)// if we can't open it...
		{
			// try to create a file with the given name
			sourcefile=fopen(sourcefilename,"w");
			if(sourcefile==NULL)// if we couldn't create that file,
			{
				printf("disk full?"); //report user about the failure.
				exit(0);
			}
		}
		fclose(sourcefile);

	}
	else // if user desn't provide a file name or provide more file names,
	{
		printf(" provide a file name(only one)");
		exit(0);
	}
//*************end of file open operations**************

	show_editor();
	while(flag)
		edit_in_escape_mode();

return 0;
}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void show_editor()
{
	parse_from_file();
	editor_initialize();
	flush_on_screen();
	show_cursor();
}
/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void save_into_file()
{
	reverse_parse_into_file();
}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void parse_from_file()
{
	char text[80];
	char ch;
	int i = 0,j;
	sourcefile=fopen(sourcefilename,"r");
	ch = fgetc(sourcefile);
	while(ch != EOF)
	{
		if(ch=='\t')
		{
			j = i + 7;
			for(;i<=j;i++)
			{
				text[i]=TAB;
			}
		}
		else if(ch == '\n')
		{
			text[i] = NEWLINE;

		}
		else
		{
			text[i] = ch;
		}
		i++;
		if(i == 80 || ch == (char) NEWLINE)
		{
			insert(-1,text);
			i = 0;
		}
		ch=fgetc(sourcefile);
	}
	if(i < 80)
	{
		text[i] = PADDING;
		insert(-1,text);
	}
}
/***************************************************
Function Name:
parameters:
Description:
****************************************************/
void editor_initialize()
{
	start_display_node = current_node = head;
}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/
void reverse_parse_into_file()
{

}
/***************************************************
Function Name:
parameters:
Description:
****************************************************/
void show_cursor()
{
// write this at last
}

/***************************************************
Function Name: edit_in_escape_mode
parameters:None
Description: In below cases, the editor will be in Escape mode.
			 1. When the editor will open for the first time, it will go into this mode.
			 2. Suppose editor is in the insert mode, pressing 'ESC' character will take the editing mode
				to Escape mode.
Functionality: Below functions can be done in escape mode.
			1. Pressing 'dd' will delete a line.
			2. Pressing 'x' will delete the current character. (if there is any)
			3. Pressing 'h','k','j','l' will move the cursor left,up,down and right respectively.
****************************************************/

void edit_in_escape_mode()
{
	char ch;
	editor_mode = ESCAPE_MODE;
	ch = getch();
	switch(ch)
	{
		case 'i':
			edit_in_insert_mode();
		break;
		case ':':
			edit_in_colon_mode();
		break;
		case 'd':
			 if(getch() == 'd')
			 {
				 delete_current_line();
				 flush_on_screen();
			 }
			 else
			 {
				 //invalid option..
			 }
		break;
		case 'x':
				 delete_current_character();
				 flush_on_screen();
		break;
		case 'k':
			//up arrow
			/*if(cursor_position.y == TOPMOST_DISPLAY_LINE_NUMBER)
				move_displayed_data_one_line_down();
			else
				move_cursor_up();*/
		break;
		case 'j':
			//down arrow
			/*if(cursor_position.y == BOTTOMMOST_DISPLAY_LINE_NUMBER)
				move_displayed_data_one_line_up();
			else
				move_cursor_down();*/
		break;
		case 'h':
			//left arrow
			move_cursor_left();
		break;
		case 'l':
			//right arrow
			move_cursor_right();
		break;
	}
}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void edit_in_insert_mode()
{
	char ch;
	editor_mode = INSERT_MODE;
	ch = getch();
	while(ch!=27)//ESC
	{
//insert
	}
	edit_in_escape_mode();
}

/***************************************************
Function Name: edit_in_colon_mode
parameters:None
Description: In colon mode,pressing the 'w', 'q' preceeded by a colon and followed by ENTER, will lead to
			 save and quit (without saving) the file respectively.
****************************************************/


void edit_in_colon_mode()
{
	char ch;
	editor_mode = COLON_MODE;
	ch = getch();
	switch(ch)
	{
		case 'w':
			if(getch()==13)//ENTER or carriage return
			{
			save_file();//save
			}
		break;
		case 'q':
			if(getch()==13)//ENTER or carriage return
			{
			 quit_file();//without save
			}
		break;
	}
	edit_in_escape_mode();
}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void move_cursor_up()
{

}
/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void move_cursor_down()
{
}
/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void move_cursor_left()
{
}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void move_cursor_right()
{

}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/

node * create_node(char *text)
{
	node *new_node = (node*) malloc (sizeof(node));
	if(new_node != NULL)
	{
		stringcopy(new_node->text,text);
		new_node -> next = NULL;
		new_node -> prev = NULL;
		new_node -> text_length = get_string_length(text);
	}
	return new_node;
}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void stringcopy(char* a, char* b)
{
	int i=0;
	while(i<=79)
	{
		a[i]=b[i];
		i++;
	}
}

/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void insert(int insert_pos,char * text )
{
	node *new_node,*ptr;
	new_node = create_node(text);

	if(head == NULL)
	{
		head  = new_node;
		tail = new_node;
	}
	else
	{
		if(insert_pos == -1)
		{
			new_node -> prev = tail;
			tail -> next = new_node;
			tail = new_node;
		}
		else
		{
			if(current_node == tail)
			{
				new_node -> prev = tail;
				tail -> next = new_node;
				tail = new_node;
			}
			else
			{
				new_node -> next = current_node -> next;
				(current_node -> next) -> prev = new_node;
				new_node -> prev = current_node;
				current_node -> next = new_node;
			}
		}
	}
}
/***************************************************
Function Name: delete_current_line
parameters:None
Description: It deletes the current line (provided the the line contains some data).e.g: suppose the cursor is @14 line number, pressing 'dd'
			 at line@14 will delete that perticular line.

****************************************************/

void delete_current_line()
{
	node * temp = current_node -> next;
	node * temp_start_display_node = start_display_node -> next;
	delete_node();
	if(current_node == head && current_node == tail)
	{
		// while deleting the last line of a file we need to create a
		// empty node and assign the current_node and start_display_node as head.
		create_empty_node();
		current_node = start_display_node = head;
	}
	else if(current_node == head)
	{
		start_display_node = current_node = head;
	}
	else if(current_node == tail)
	{
		// start display line will not be changed..
		// because we are not touching this when we are deleting the last line.
		current_node = tail;
	}
	else
	{
		start_display_node = temp_start_display_node;
		current_node = temp;
	}
}

/***************************************************
Function Name: delete_current_character
parameters:None
Description: It deletes the current character which is under the cursor presently.
			 (provided that character is a valid data)
			 e.g: suppose the cursor is at 14th line and it is in 3rd character, pressing 'x'
			 will delte the 3rd character.

****************************************************/
void delete_current_character()
{

	/*int i=cursor_pos.x;
	while(i<=79 && current_node->text[i+1]!=NEWLINE)
	{
		current_node->text[i]=current_node->text[i+1];
	}*/
}

/***************************************************
Function Name: delete_node
parameters:
Description:
****************************************************/


void delete_node()
{
	node *temp;
	if(current_node == head && current_node == tail)
	{
		head = tail = NULL;
		free(current_node);
	}
	else if(current_node == head)
	{
		head = current_node -> next;
		head -> prev = NULL;
		free (current_node);

	}
	else if(current_node == tail)
	{
		tail = current_node -> prev;
		tail -> next = NULL;
		free(current_node);
	}
	else
	{
		(current_node -> prev) -> next = current_node -> next;
		(current_node -> next) -> prev = current_node -> prev;
		free (current_node);
	}
}
/*************************************************
Function Name:
parameters:
Description:
*************************************************/
void create_empty_node()
{
	char text[80];
	text[0] = PADDING;
	insert(-1,text);
}
/***************************************************
Function Name:
parameters:
Description:
****************************************************/

void flush_on_screen()
{
	int i,j,k,p;
	node *temp = start_display_node;
	char far *videomemory = (char far*)0xB8000000;
	clrscr();
	for(j=TOPMOST_DISPLAY_LINE_NUMBER;
		(j<=BOTTOMMOST_DISPLAY_LINE_NUMBER && temp != NULL);
		j++,temp = temp -> next)
	{
	for(i = LEFTMOST_DISPLAY_COLUMN_NUMBER,p = 0;
		(i <= RIGHTMOST_DISPLAY_COLUMN_NUMBER);
		i++,p = p+2)
	{
		// Start FLUSHING the node text in to the video buffer here.
		if(temp->text[i]==(char)TAB)
		{
			k = i + 7;
			for(;p<=k;p+=2)
			{
				*(videomemory+(j*160+p))= SPACE;
			}
		}
		else if(temp->text[i] == (char)NEWLINE
			   || temp->text[i] == (char)PADDING)
		{
			for(;p<=79;p = p+2)
			*(videomemory+(j*160+p))= SPACE;
			break;
		}
		else
		{
			*(videomemory+(j*160+p))= temp->text[i];
		}
	}
	}
}

/***************************************************
Function Name:
parameters:
Description:
***************************************************/
void quit_file()
{
	exit(0);
}

/***************************************************
Function Name:
parameters:
Description:
***************************************************/
void save_file()
{
}
int get_string_length(char *string)
{
	int i=0;
	while(*string != (char)NEW_LINE || *string != (char)PADDING)
	{
		i++;
		string++;
		if(i == 80)
		break;
	}
	return i;
}
