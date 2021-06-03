#include <ncurses.h>
#include <iostream>

using namespace std;

#define WIDTH 49  // 가로 크기
#define HEIGHT 24 // 세로 크기

void init();
void update();
void exit();

WINDOW *win1;
WINDOW *win2;
WINDOW *win3;

int main(int argc, char *argv[])
{
  init();
  update();
  exit();
}

void init()
{
  // 초기 설정 관련
  initscr();                          // 스크린 활성화
  resize_term(HEIGHT, WIDTH + 30);    // 화면 크기 설정 (점수 표시를 위한 공간 포함)
  curs_set(0);                        // 커서(깜빡임) 비활성화
  box(stdscr, 0, 0);

  // Default 또는 새로운 윈도우에서 사용할 Color Set 설정
  start_color();
  init_color(COLOR_BLACK, 0, 0, 0);               // 검은색 Color를 완전히 어두운 색으로 설정
  init_color(COLOR_CYAN, 500, 500, 500);          // 기본 Color로 Gray 색상이 제공되지 않기 때문에 CYAN Color를 회색으로 변경
  init_pair(1, COLOR_WHITE, COLOR_WHITE);         // 1번 쌍에 게임화면 배경색 설정
  init_pair(2, COLOR_BLACK, COLOR_BLACK);         // 2번 쌍에 각 꼭짓점(Immune Wall) 색상 설정
  init_pair(3, COLOR_CYAN, COLOR_CYAN);           // 3번 쌍에 모서리(Wall) 색상 설정
  init_pair(4, COLOR_BLACK, COLOR_CYAN);          // 4번 쌍에 점수 윈도우에서 폰트로 사용할 색상 설정

  // 게임화면 및 점수 표시 관련 윈도우 생성 및 초기화
  win1 = newwin(HEIGHT-2, WIDTH-3, 1, 2); // 설정한 가로, 세로 크기에 따라 게임화면 윈도우를 생성
  win2 = newwin(9, 24, 2, WIDTH+2);       // 점수 표시를 위한 고정된 크기의 윈도우 생성
  win3 = newwin(9, 24, 13, WIDTH+2);      // 미션 위한 고정된 크기의 윈도우 생성

  // 게임화면 배경 색상 설정
  wattron(win1, COLOR_PAIR(1));
  wbkgd(win1, COLOR_PAIR(1));
  wattroff(win1, COLOR_PAIR(1));

  // 게임화면의 Immune Wall 색상 적용
  wattron(win1, COLOR_PAIR(2));
  mvwprintw(win1, 0, 0, "00");
  mvwprintw(win1, HEIGHT-3, 0, "00");
  mvwprintw(win1, 0, WIDTH-5, "00");
  mvwprintw(win1, HEIGHT-3, WIDTH-5, "00");
  wattroff(win1, COLOR_PAIR(2));

  // 게임화면의 Wall 색상 초기 적용
  wattron(win1, COLOR_PAIR(3));
  for(int i = 2; i < WIDTH-6; i += 2)             // 상단, 하단 모서리
  {
    mvwprintw(win1, 0, i, "00");
    mvwprintw(win1, HEIGHT-3, i, "00");
  }
  for(int i = 1; i < HEIGHT-3; i++)               // 좌측, 우측 모서리
  {
    mvwprintw(win1, i, 0, "00");
    mvwprintw(win1, i, WIDTH-5, "00");
  }
  wattroff(win1, COLOR_PAIR(3));

  // 점수 윈도우 초기 색상 적용
  wattron(win2, COLOR_PAIR(3));
  wbkgd(win2, COLOR_PAIR(3));
  wattroff(win2, COLOR_PAIR(3));

  wattron(win2, COLOR_PAIR(4));
  mvwprintw(win2, 1, 1, "Score Board");
  mvwprintw(win2, 3, 1, "B: 0 / 0");
  mvwprintw(win2, 4, 1, "+: 0");
  mvwprintw(win2, 5, 1, "-: 0");
  mvwprintw(win2, 6, 1, "G: 0");
  wattroff(win2, COLOR_PAIR(4));

  // 점수 윈도우 초기 색상 적용
  wattron(win3, COLOR_PAIR(3));
  wbkgd(win3, COLOR_PAIR(3));
  wattroff(win3, COLOR_PAIR(3));

  wattron(win3, COLOR_PAIR(4));
  mvwprintw(win3, 1, 1, "Mission");
  mvwprintw(win3, 3, 1, "B: 0 ( )");
  mvwprintw(win3, 4, 1, "+: 0 ( )");
  mvwprintw(win3, 5, 1, "-: 0 ( )");
  mvwprintw(win3, 6, 1, "G: 0 ( )");
  wattroff(win3, COLOR_PAIR(4));
}

void update()
{
  // Window Refresh(변경사항 반영)
  refresh();
  wrefresh(win1);
  wrefresh(win2);
  wrefresh(win3);

  getch();
}

void exit()
{
  // 각 윈도우를 위해 할당한 메모리 반환
  delwin(win1);
  delwin(win2);
  delwin(win3);

  // 종료
  endwin();
}
