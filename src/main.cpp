#include <ncurses.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

#define WIDTH 21     // 가로 크기
#define HEIGHT 21    // 세로 크기

#define TICK_RATE 500 // 화면 갱신 시간 (ms)

#define LEFT 260
#define RIGHT 261
#define UP 259
#define DOWN 258
#define ESC 27

void init();
void update();
void keyControl();
void map_update();
void exit();

// 스네이크 게임에 사용할 Window들을 전역 변수로 선언
WINDOW *win1;
WINDOW *win2;
WINDOW *win3;

// 벽, 캐릭터 위치 등 글로벌 정보를 저장할 이차원 배열
int map[HEIGHT][WIDTH] = {};
int y = HEIGHT/2, x = WIDTH/2;
vector<pair<int, int>> body;

// 입력키 저장 (아래쪽-258, 위쪽-259, 왼쪽-260, 오른쪽-261)
static int key = LEFT;

int main(int argc, char *argv[])
{
  thread key_thread(keyControl);  // 입력 제어 쓰레드 생성

  init();     // Window, Map, Head, Body 등 각종 초기 설정을 위한 함수 호출
  update();   // 게임 진행 중 수정사항 반영을 위한 함수 호출
  exit();     // 프로그램 종료 전 메모리 반환 등을 위한 함수 호출

  // 입력 제어 쓰레드 종료를 먼저 기다린 뒤 프로그램 종료
  if(key_thread.joinable())
    key_thread.join();
}

void init()
{
  // 초기 설정 관련
  initscr();                              // 스크린 활성화
  resize_term(HEIGHT + 2, WIDTH * 2 + 30);    // 화면 크기 설정 (점수 표시를 위한 공간 포함)
  curs_set(0);                            // 커서(깜빡임) 비활성화
  box(stdscr, 0, 0);
  keypad(stdscr, TRUE);
  noecho();

  // Default 또는 새로운 윈도우에서 사용할 Color Set 설정
  start_color();
  init_color(COLOR_BLACK, 0, 0, 0);               // 검은색 Color를 완전히 어두운 색으로 설정
  init_color(COLOR_CYAN, 500, 500, 500);          // 기본 Color로 Gray 색상이 제공되지 않기 때문에 CYAN Color를 회색으로 변경
  init_pair(1, COLOR_WHITE, COLOR_WHITE);         // 1번 쌍에 게임화면 배경색 설정
  init_pair(2, COLOR_BLACK, COLOR_BLACK);         // 2번 쌍에 각 꼭짓점(Immune Wall) 색상 설정
  init_pair(3, COLOR_CYAN, COLOR_CYAN);           // 3번 쌍에 모서리(Wall) 색상 설정
  init_pair(5, COLOR_GREEN, COLOR_GREEN);
  init_pair(6, COLOR_RED, COLOR_RED);
  init_pair(4, COLOR_BLACK, COLOR_CYAN);          // 4번 쌍에 점수 윈도우에서 폰트로 사용할 색상 설정

  // 게임화면 및 점수 표시 관련 윈도우 생성 및 초기화
  win1 = newwin(HEIGHT, WIDTH * 2, 1, 2); // 설정한 가로, 세로 크기에 따라 게임화면 윈도우를 생성
  win2 = newwin(10, WIDTH + 3, 1, WIDTH * 2 + 3);       // 점수 표시를 위한 고정된 크기의 윈도우 생성
  win3 = newwin(10, WIDTH + 3, 12, WIDTH * 2 + 3);      // 미션 위한 고정된 크기의 윈도우 생성

  // 게임화면 배경 색상 설정
  wattron(win1, COLOR_PAIR(3));
  wbkgd(win1, COLOR_PAIR(3));
  wattroff(win1, COLOR_PAIR(3));

  // Immune Wall, Wall, Field와 Character의 Head, Body 초기 설정
  map[0][0] = map[0][WIDTH-1] = 2;
  map[HEIGHT-1][0] = map[HEIGHT-1][WIDTH-1] = 2;
  for(int i = 1; i < WIDTH-1; i++)
    map[0][i] = map[HEIGHT-1][i] = 1;
  for(int i = 1; i < HEIGHT-1; i++)
    map[i][0] = map[i][WIDTH-1] = 1;
  map[y][x] = 3;

  body.push_back(pair<int, int>(y, x+1));
  body.push_back(pair<int, int>(y, x+2));

  // 점수 윈도우 초기 색상 적용
  wattron(win2, COLOR_PAIR(3));
  wbkgd(win2, COLOR_PAIR(3));
  wattroff(win2, COLOR_PAIR(3));

  // 점수창 초기 텍스트 설정
  wattron(win2, COLOR_PAIR(4));
  mvwprintw(win2, 1, 1, "Score Board");
  mvwprintw(win2, 3, 1, "B: 0 / 0");
  mvwprintw(win2, 4, 1, "+: 0");
  mvwprintw(win2, 5, 1, "-: 0");
  mvwprintw(win2, 6, 1, "G: 0");
  wattroff(win2, COLOR_PAIR(4));

  // 미션 윈도우 초기 색상 적용
  wattron(win3, COLOR_PAIR(3));
  wbkgd(win3, COLOR_PAIR(3));
  wattroff(win3, COLOR_PAIR(3));

  // 미션창 초기 텍스트 설정
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
  while(true)
  {
    // 기존에 그려져 있던 Head, Body를 Map에서 삭제
    map[y][x] = 0;
    for(int i = 0; i < body.size(); i++)
      map[body[i].first][body[i].second] = 0;

    // Head를 body[0]으로 가져오고, 나머지는 앞에서 이동한 위치로 따라서 한 칸씩 이동
    for(int i = 1; i < body.size(); i++)
      body[i] = body[i-1];
    body[0].first = y;
    body[0].second = x;

    // 입력에 따른 다음 이동 처리
    if(key == LEFT)
      x--;
    if(key == RIGHT)
      x++;
    if(key == UP)
      y--;
    if(key == DOWN)
      y++;
    if(key == ESC || y < 1 || y > HEIGHT-2 || x < 1 || x > WIDTH-2) // 지도를 벗어나거나 ESC를 입력하면 종료
      break;
    map[y][x] = 3;  // 이동할 위치의 계산이 끝나면 해당 위치에 Head 표시

    // body 부분을 Map에 저장
    for(int i = 0; i < body.size(); i++)
      map[body[i].first][body[i].second] = 4;

    // 지도 업데이트 함수 호출
    map_update();

    // Window Refresh(변경사항 반영)
    refresh();
    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);

    this_thread::sleep_for(chrono::milliseconds(TICK_RATE));  // Tick Rate 마다 화면 갱신
  }
}

void keyControl()
{
  while(true)
  {
    int input = getch();

    // 방향키 입력인 경우에만 반영
    if(input == UP || input == DOWN || input == LEFT || input == RIGHT)
      key = input;
    else if(input == ESC)
      break;

    this_thread::sleep_for(chrono::milliseconds(TICK_RATE));
  }
}

void map_update()
{
  for(int i = 0; i < HEIGHT; i++)
  {
    for(int j = 0; j < WIDTH; j++)
    {
      if(map[i][j] == 2)                        // 게임화면의 Immune Wall 색상 적용
      {
        wattron(win1, COLOR_PAIR(2));
        mvwprintw(win1, i, j*2, "2");
        mvwprintw(win1, i, j*2+1, "2");
        wattroff(win1, COLOR_PAIR(2));
      }
      else if(map[i][j] == 1)                   // 게임화면의 Wall 색상 적용
      {
        wattron(win1, COLOR_PAIR(3));
        mvwprintw(win1, i, j*2, "1");
        mvwprintw(win1, i, j*2+1, "1");
        wattroff(win1, COLOR_PAIR(3));
      }
      else if(map[i][j] == 0)                   // 게임화면의 Field 색상 적용
      {
        wattron(win1, COLOR_PAIR(1));
        mvwprintw(win1, i, j*2, "0");
        mvwprintw(win1, i, j*2+1, "0");
        wattroff(win1, COLOR_PAIR(1));
      }
      else if(map[i][j] == 3)                   // 게임화면의 Head 색상 적용
      {
        wattron(win1, COLOR_PAIR(5));
        mvwprintw(win1, i, j*2, "3");
        mvwprintw(win1, i, j*2+1, "3");
        wattroff(win1, COLOR_PAIR(5));
      }
      else if(map[i][j] == 4)                   // 게임화면의 Body 색상 적용
      {
        wattron(win1, COLOR_PAIR(6));
        mvwprintw(win1, i, j*2, "4");
        mvwprintw(win1, i, j*2+1, "4");
        wattroff(win1, COLOR_PAIR(6));
      }
    }
  }
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
