#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <curses.h>
#include <string.h>

#define SIZE 5000
int x = 0, y = 0;

void sig_winch(int signo);
void text_editor(char *file_name, char *buff);
int openfile(char *file_name, int ch);
void menu();

int main(){
	char *buff = malloc(SIZE*sizeof(char));
	char file_name[255];
	int working = 1;

	initscr();
	signal(SIGWINCH, sig_winch);
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	//cbreak();
	curs_set(TRUE);

	menu();
	text_editor(file_name, buff);
	
	clear();
	endwin();
	free(buff);
	exit(EXIT_SUCCESS);
}


void sig_winch(int signo){
	struct winsize size;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
	resizeterm(size.ws_row, size.ws_col);
}

void menu(){
	move(20,0);      
    hline(ACS_CKBOARD,100); 

	mvaddstr(21,0,"F1 to open exists file");
	mvaddstr(21,50,"F2 to save file");
	mvaddstr(22,0,"F3 to quit the editor");

	move(y, x);
}

int openfile(char *file_name, int ch){
	int WFopen = 1, wrong_name = 0;
	int i = 0, fd = -1;

	while(WFopen){
		if ((ch  == KEY_F(1)) || (wrong_name)){
			printw("Input name of file: ");
			x = 21;
			while (1){
				ch = getch();
				if (ch == 10) break;
				if (ch == KEY_BACKSPACE){
					if (x != 21){ 
						mvaddch(y, x - 1, ' ');
						file_name[i] = '\0';
						move(y, x - 1);
						x--; i--;
					}
				} else{	
					file_name[i] = ch;
					mvaddch(y, x, ch);
					x++;
					move(y, x);
					i++;	
				}
			}
			file_name[i] = '\0';
			fd = open(file_name, O_RDWR);
			if (fd == -1){
				wrong_name = 1;
				i = 0;
				clear();
				menu();
				x = 0; y = 0;
				move(y, x);
			}
			else{
				x = 0, y = 0;
				WFopen = 0;
				wrong_name = 0;
			}
		}
		ch = getch();
	}
	return fd;
}

void text_editor(char *file_name, char *buff){
	int ch, i = 0, fd = -1;
	int working = 1, WFopen = 1, wrong_name = 0;
	int maxcols = 80;

	move(x, y);

	fd = openfile(file_name, ch);

	clear(); menu();
	i = read(fd, buff, SIZE);
	if (i != 0){
		printw(buff);
		i = i - 2;
		if (i > maxcols){
			y = i / (maxcols + 1);
			x = (i - i%maxcols) % (maxcols + 1) + 1;
		} else x = (i - i%maxcols) + 1;
		move(y, x);
	}


	//i = 0;
	while((working) && (i < SIZE - 1)){
		ch = getch();

		switch(ch){
			case KEY_BACKSPACE:{
				if (x == 0) 
					if (y != 0){
						mvaddch(y - 1, maxcols, ' ');
						buff[i] = ' ';
						y--; x = maxcols; i--;
					} else break;
				mvaddch(y, x - 1, ' ');
				buff[i] = ' ';
				move(y, x - 1);
				x--; i--;
				break;
			}
			case KEY_F(1):{
				close(fd);
				clear(); menu();
				x = 0; y = 0;
				move(y, x);
				fd = openfile(file_name, ch);
				clear(); menu();
				if (i != 0){
					i = read(fd, buff, SIZE);
					printw(buff);
					i = i - 2;
					if (i > maxcols){
						y = i / (maxcols + 1);
						x = (i - i%maxcols) % (maxcols + 1) + 1;
					}
					else x = (i - i%maxcols) + 1;
					i = i - i%maxcols;
					move(y, x);
				}
				break;
			}
			case KEY_F(2):{
				lseek(fd, SEEK_SET, 0);
				write(fd, buff, i);
				fsync(fd);
				break;
			}
			case KEY_F(3):{
				working = 0;
				close(fd);
				break;
			}
			default:{
				if (x == maxcols){
					buff[i] = '\n';
					i++;
					buff[i] = ch;
					mvaddch(y, x, ch);
					move(y + 1, 0);
					x = 0; y++; i++;
					break;
				}
				buff[i] = ch;
				mvaddch(y, x, ch);
				x++;
				move(y, x);
				i++;
				break;
			}
		}
	}
}