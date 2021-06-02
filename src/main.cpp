#include <ncurses.h>

using namespace std;

#define WIDTH 49
#define HEIGHT 24

int main(int argc, char *argv[])
{
  initscr();
  resize_term(HEIGHT, WIDTH);

  start_color();

  WINDOW *win1 = newwin(HEIGHT-2, WIDTH-3, 1, 2);
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  init_pair(2, COLOR_BLACK, COLOR_BLACK);

  wattron(win1, COLOR_PAIR(1));
  wbkgd(win1, COLOR_PAIR(1));
  wattroff(win1, COLOR_PAIR(1));

  wattron(win1, COLOR_PAIR(2));
  mvwprintw(win1, 0, 0, "00");
  mvwprintw(win1, HEIGHT-3, 0, "00");
  mvwprintw(win1, 0, WIDTH-5, "00");
  mvwprintw(win1, HEIGHT-3, WIDTH-5, "00");
  wattroff(win1, COLOR_PAIR(2));

  refresh();
  wrefresh(win1);

  getch();

  refresh();

  delwin(win1);

  endwin();

  return 0;
}
