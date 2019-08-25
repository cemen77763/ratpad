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

#define handle_error(msg)\
 do { perror(msg); exit(EXIT_FAILURE); } while(0);

int x = 1, y = 1;
int STR = 1;

void sig_winch(int signo);

void text_editor(WINDOW *win);

int openfile(WINDOW *win, char *file_name, int ch);

void editing(char *text);

void save_file(WINDOW *win, int fd);

void menu(WINDOW *win);


int main(){
	WINDOW *win;
	char file_name[255];
	int working = 1;

	initscr();
	signal(SIGWINCH, sig_winch);
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(TRUE);
	start_color();

	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	win = newwin(20, 80, 0, 0);

	menu(win);
	text_editor(win);
	
	delwin(win);
	endwin();
	exit(EXIT_SUCCESS);
}


void sig_winch(int signo){
	struct winsize size;
	ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
	resizeterm(size.ws_row, size.ws_col);
}

void menu(WINDOW *win){
	wbkgd(win, COLOR_PAIR(1));
	box(win, '|', '-');

	move(20,0);      
    hline(ACS_CKBOARD,100); 

	mvaddstr(21,0,"F1 to open exists file");
	mvaddstr(21,50,"F2 to save file");
	mvaddstr(22,0,"Esc to quit the editor");

	refresh();
	wrefresh(win);
}

int openfile(WINDOW *win, char *file_name, int ch){
	int WFopen = 1, wrong_name = 0;
	int i = 0, fd = -1;
	if (ch != KEY_F(1)){
		while(1){
			ch = getch();
			if (ch == KEY_F(1)) break;
		}
	}

	while(WFopen){
		if ((ch  == KEY_F(1)) || (wrong_name)){
			wmove(win, 1, 1);
			wprintw(win, "Input name of file: ");
			x = 21;
			wrefresh(win);
			while (1){
				ch = getch();
				if (ch == 10){
					wmove(win, 1, 21);
					winstr(win, file_name);
					break;
				}
				if (ch == KEY_BACKSPACE){
					if (x != 21){ 
						mvwaddch(win, y, x - 1, ' ');
						wmove(win, y, x - 1);
						x--; i--;
						wrefresh(win);
					}
				} else{	
					mvwaddch(win, y, x, ch);
					x++;
					wmove(win, y, x);
					i++;
					wrefresh(win);	
				}
			}
			file_name[i] = '\0';
			fd = open(file_name, O_RDWR);
			if (fd == -1){
				wrong_name = 1;
				i = 0;
				wclear(win);
				menu(win);
				x = 1; y = 1;
				wmove(win, y, x);
			}
			else{
				x = 1, y = 1;
				WFopen = 0;
				wrong_name = 0;
			}
		}
	}
	return fd;
}

void editing(char *text){
	int i = 0,j = 0;

	while(text[i] != '\0'){
		if (text[i] == '\n')
			text[i] = ' ';
		i++;
	} 
	text[i] = '\0';
}

void save_file(WINDOW *win, int fd){
	int i;
	char buff[79];

	lseek(fd, SEEK_SET, 0);
	for (i = 1; i <= STR; i++){
		wmove(win, i, 1);
		winstr(win, buff);
		buff[78] = '\n'; buff[79] = '\0';
		write(fd, buff, 79);
	}
}

void text_editor(WINDOW *win){
	int ch, i = 0, fd = -1;
	int working = 1, WFopen = 1, wrong_name = 0;
	int maxcols = 79;
	char file_name[30], buff[80];


	fd = openfile(win, file_name, ch);

	wclear(win);
	menu(win);

	i = read(fd, buff, 78);
	buff[i] = '\0';
	editing(buff);
	x = 1; y = 1;
	while(i){
		wmove(win, y, x);
		wprintw(win, buff);
		if (i == 78){
			y++;
			x = 1;
			STR++;
		}
		else x = i;
		i = read(fd, buff, 78);
		buff[i] = '\0';
		editing(buff);
		wrefresh(win);
	}

	wmove(win, y, x);
	wrefresh(win);
	while (working){
		ch = getch();

		switch(ch){
			case KEY_BACKSPACE:{
				if (x == 1) 
					if (y != 1){
						STR--;
						mvwaddch(win, y - 1, maxcols - 1, ' ');
						y--; x = maxcols;
						wrefresh(win);
					} else break;
				mvwaddch(win, y, x - 1, ' ');
				wmove(win, y, x - 1);
				x--; 
				wrefresh(win);
				break;
			}
			case 10:{
				x = 1;
				y++; 
				if (y > STR) STR++;
				wmove(win, y, x);
				wrefresh(win);
				break;
			}
			case KEY_F(1):{
				STR = 1;
				close(fd);
				wclear(win);
				menu(win);
				x = 1; y = 1;
				wmove(win, y, x);
				fd = openfile(win, file_name, ch);
				wclear(win); 
				menu(win);

				i = read(fd, buff, 78);
				buff[i] = '\0';
				editing(buff);
				x = 1; y = 1;
				while(i){
					wmove(win, y, x);
					wprintw(win, buff);
					if (i == 78){
						STR++;
						y++;
						x = 1;
					}
					else x = i;
					i = read(fd, buff, 78);
					buff[i] = '\0';
					editing(buff);
					wrefresh(win);
				}
				wmove(win, y, x);
				wrefresh(win);
				break;
			}
			case KEY_F(2):{
				save_file(win, fd);
				break;
			}
			case 27:{
				working = 0;
				close(fd);
				break;
			}
			default:{
				if (x == maxcols){
					STR++;
					mvwaddch(win, y + 1, 1, ch);
					wmove(win, y + 1, 2);
					x = 2; y++; 
					wrefresh(win);
					break;
				}
				mvwaddch(win, y, x, ch);
				x++;
				wmove(win, y, x);
				wrefresh(win);
				break;
			}
		}
	}
}