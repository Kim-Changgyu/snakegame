#include <ncurses.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <time.h>
#include <math.h>
#include <fstream>
#include <string>

#include "timer.h"
using namespace std;

#define WIDTH 21      // 가로 크기
#define HEIGHT 21     // 세로 크기

#define TICK_RATE 500 // 화면 갱신 시간 (ms)

#define ESC 27        // ESC

void init_reset();                //미션성공 시 게임화면 초기화를 위한 함수
void init();                      // 초기 설정을 위한 함수
void init_draw(int playtime);     // 초기 설정 및 업데이트 시 호출할 함수
void update();                    // 갱신 시간(Tick Rate)마다 호출할 함수
void keyControl();                // 비동기 입력 처리를 위한 함수
void map_update(bool itemIs);     // update 함수에서 호출할 win1(게임화면) 업데이트 함수
void exit();                      // NCurses 종료 전 메모리 반환 등을 위한 함수
void startview();                 //게임시작 시 화면을 위한 함수
void gameoverview();              //게임 오버시 화면을 위한 함수
void nextlevelview();             //미션 완료시 화면을 위한 함수
void GameClearview();             //미션을 모두 클리어 했을때 화면을 위한 함수
void RunGame();                   //게임 실행을 위한 함수
void MapLoad(int level);          // 스테이지별 맵 로드
void Gate();

// 스네이크 게임에 사용할 Window들을 전역 변수로 선언
WINDOW *win1;         // 게임화면(Wall, Head, Body 등이 움직이는 Field)
WINDOW *win2;         // 점수화면
WINDOW *win3;         // 미션화면

// 벽, 캐릭터 위치 등 글로벌 정보를 저장할 이차원 배열
int map[HEIGHT][WIDTH] = {};

int itemcount = 0;
//현재 레벨에 대한 함수
int level = 0;

// 초기 Head 위치 설정
int y = HEIGHT/2, x = WIDTH/2;

// Head를 따라갈 body (벡터의 원소는 (y, x)를 저장하도록 pair 자료형 사용)
vector<pair<int, int>> body;

//미션에 대한 2차원 벡터 missionvec[i][j]라 할 때 i는 레벨,  j는 순서대로 각 레벨의 목표bodylength, 목표growth_cnt, 목표poison_cnt 이다.
vector<vector<int> > missionvec = { {3, 1, 0}, {3, 1, 1}, {3, 2, 1} };

// 입력키 저장을 위한 변수 (NCurses에 내장된 키워드 사용)
static int key = KEY_LEFT;

// 게임 실행 제어 관련 변수
bool running = true;
// 미션을 클리어했는지 에 대한 변수
bool IsMissionClear = false;

//
bool IsGate = false;
int gate_in[2][2] = { {0, 0}, {0, 0} };
int gate_out[2][2]  = { {0, 0}, {0, 0} };

// 포지션
struct POSITION {
    int x, y;
    POSITION(int row, int col) {
      x = col;
      y = row;
    }
    POSITION() {
      x = 0;
      y = 0;
    }

    bool operator==(POSITION p) {
      return ((x == p.x) && (y == p.y));
    }
};

// item 제어 클래스
class Item {
public:
    Timer spawnTime;
    POSITION pos;
    int itemtype;

    Item() {
      if(rand() % 3 == 0)
        this->itemtype = 6;
      else
        this->itemtype = 5;

      setItemPos();
      this->spawnTime.startTimer();
    }

    //item의 위치를 램덤하게 생성하는 함수
    void setItemPos() {
      srand((unsigned int)time(0));
      int x = (rand() % (WIDTH - 1)) + 1;
      int y = (rand() % (HEIGHT - 1)) + 1;
      if (map[y][x] == 0) {
          if (rand() % 2 == 0) {
              pos = POSITION(y, x);

          } else {
              pos = POSITION(y, x);

          }
      } else {
          while (map[y][x] != 0) {
              x = (rand() % (WIDTH - 1)) + 1;
              y = (rand() % (HEIGHT - 1)) + 1;
              if (map[y][x] == 0) {
                  if (rand() % 2 == 0) {
                      pos = POSITION(y, x);

                  } else {
                      pos = POSITION(y, x);
                  }
              }
          }
      }
  }
};

//실행함수
void RunGame()
{
  //count는 레벨의 갯수만큼 반복 레벨을 모두 클리어 하거나, 게임오버가 되면 게임이 종료된다.
  int count = 0;
  while(count <= 2)
  {
    gate_in[0][0] = gate_in[0][1] = gate_in[1][0] = gate_in[1][1] = 0;
    gate_out[0][0] = gate_out[0][1] = gate_out[1][0] = gate_out[1][1] = 0;
    Gate();

    update();
    if(IsMissionClear)
    {
      if(level == 3)
      {
        GameClearview();
        break;
      }
      nextlevelview();
      IsMissionClear = false;
      IsGate = false;
    }
    else
    {
      gameoverview();
      break;
    }
    running = true;
    count++;
    refresh();
    init_reset();
  }
}

void GameClearview()
{
  clear();
  nodelay(stdscr, FALSE);
  refresh();
  refresh();
  initscr();
  start_color();
  init_pair(1, COLOR_RED,COLOR_WHITE);
  attron(COLOR_PAIR(1));
  printw("Game Clear!\n");
  printw("Press any key");
  attroff(COLOR_PAIR(1));
  refresh();
  getch();
  endwin();
}

void nextlevelview()
{
  clear();
  nodelay(stdscr, FALSE);
  refresh();
  initscr();
  start_color();
  init_pair(1, COLOR_RED,COLOR_WHITE);
  attron(COLOR_PAIR(1));
  printw("level : %d clear\n",level);
  printw("Press any key");
  attroff(COLOR_PAIR(1));
  refresh();
  getch();
  endwin();
}

void startview()
{
  initscr();
  start_color();
  init_pair(1, COLOR_RED,COLOR_WHITE);
  attron(COLOR_PAIR(1));
  printw("Snake Game\n");
  printw("Press any key");
  attroff(COLOR_PAIR(1));
  refresh();
  getch();
  endwin();
}
void gameoverview()
{
  clear();
  nodelay(stdscr, FALSE);
  refresh();
  initscr();
  start_color();
  init_pair(1, COLOR_RED,COLOR_WHITE);
  attron(COLOR_PAIR(1));
  printw("Game Over\n");
  printw("Press any key");
  attroff(COLOR_PAIR(1));
  refresh();
  getch();
  clear();
}

int main(int argc, char *argv[])
{
   startview();//시작화면
   init();     // Window, Map, Head, Body 등 각종 초기 설정을 위한 함수 호출
   RunGame(); //게임실행
   exit();     // 프로그램 종료 전 메모리 반환 등을 위한 함수 호출
   return 0;
}


void init()
{
  // 초기 설정 관련
  initscr();                                  // 스크린 활성화
  resize_term(HEIGHT + 2, WIDTH * 2 + 30);    // 화면 크기 설정 (점수 표시를 위한 공간 포함)
  box(stdscr, 0, 0);                          // Default Window 에 기본 테투리 설정
  curs_set(0);                                // 커서(깜빡임) 비활성화
  keypad(stdscr, TRUE);                       // 특수키(ESC, 방향키 등) 사용 가능하도록 설정
  noecho();
  timeout(0);

  // Default 또는 새로운 윈도우에서 사용할 Color Set 설정
  start_color();
  init_color(COLOR_BLACK, 0, 0, 0);               // 검은색 Color를 완전히 어두운 색으로 설정
  init_color(COLOR_CYAN, 500, 500, 500);          // 기본 Color로 Gray 색상이 제공되지 않기 때문에 CYAN Color를 회색으로 변경
  init_color(COLOR_MAGENTA, 500, 0, 500);          // 기본 Color로 Gray 색상이 제공되지 않기 때문에 CYAN Color를 회색으로 변경

  init_pair(1, COLOR_CYAN, COLOR_CYAN);           // 1번 쌍에 모서리(Wall), 점수, 미션 윈도우 색상 설정
  init_pair(2, COLOR_BLACK, COLOR_BLACK);         // 2번 쌍에 각 꼭짓점(Immune Wall) 색상 설정
  init_pair(3, COLOR_WHITE, COLOR_WHITE);         // 3번 쌍에 게임화면의 배경 색상(흰색) 설정
  init_pair(4, COLOR_BLACK, COLOR_CYAN);          // 4번 쌍에 점수 윈도우에서 폰트로 사용할 색상 설정
  init_pair(5, COLOR_GREEN, COLOR_GREEN);         // 5번 쌍에 Snake Head 색상 설정
  init_pair(6, COLOR_RED, COLOR_RED);             // 6번 쌍에 Snake Body 색상 설정
  init_pair(7, COLOR_YELLOW, COLOR_YELLOW);     // 7번 쌍에 Growth Potion 색상 설정
  init_pair(8, COLOR_MAGENTA, COLOR_MAGENTA);     // 8번 쌍에 Poision Potion 색상 설정

  //
  int k = 0;
  init_draw(k);

  // Immune Wall, Wall, Field와 Character의 Head, Body 초기 설정
  // map[0][0] = map[0][WIDTH-1] = 2;
  // map[HEIGHT-1][0] = map[HEIGHT-1][WIDTH-1] = 2;
  // for(int i = 1; i < WIDTH-1; i++)
  //   map[0][i] = map[HEIGHT-1][i] = 1;
  // for(int i = 1; i < HEIGHT-1; i++)
  //   map[i][0] = map[i][WIDTH-1] = 1;
  MapLoad(level);

  map[y][x] = 3;
  // Head 기준으로 오른쪽으로 Body 초기 설정
  body.push_back(pair<int, int>(y, x+1));
  body.push_back(pair<int, int>(y, x+2));
}

// data/ 디렉토리에 위치한 stage(0, 1, 2, 3).txt File을 읽어서 Map에 저장
void MapLoad(int level)
{
  ifstream load("data/stage" + to_string(level) + ".txt");

  int rows = 0, cols = 0;
  char Celement;
  while(load.get(Celement))
  {
    int Ielement = (int)(Celement - '0');
    if(Ielement >= 0 && Ielement <= 8)
    {
      map[rows][cols] = Ielement;
      cols++;
    }
    else if(Celement == '\r')
    {
      load.get(Celement);               // 텍스트 파일이기 때문에 \r을 읽은 직후 \n도 읽어서 무시
      rows++;
    }
  }
}

vector<Item> ItemContainer(0);
int growth_cnt = 0;
int poison_cnt = 0;

/* init() 함수에 내장된 코드였으나 비동기 입력 처리 함수(keyControl) 사용시
   그래픽이 깨지는 문제가 발생해 매번 업데이트 직전에 모든 Window들을 종료했다가 재시작 처리 */
void init_draw(int playtime)
{
  // 미리 활성화됐던 윈도우 삭제
  delwin(win1);
  delwin(win2);
  delwin(win3);
  endwin();

  // 재시작
  initscr();

  // 게임화면 및 점수 표시 관련 윈도우 생성 및 초기화
  win1 = newwin(HEIGHT, WIDTH * 2, 1, 2);               // 설정한 가로, 세로 크기에 따라 게임화면 윈도우를 생성
  win2 = newwin(10, WIDTH + 3, 1, WIDTH * 2 + 3);       // 점수 표시를 위한 고정된 크기의 윈도우 생성
  win3 = newwin(10, WIDTH + 3, 12, WIDTH * 2 + 3);      // 미션 위한 고정된 크기의 윈도우 생성

  // 게임화면 배경 색상 설정
  wattron(win1, COLOR_PAIR(3));
  wbkgd(win1, COLOR_PAIR(3));
  wattroff(win1, COLOR_PAIR(3));

  // 점수 윈도우 초기 색상 적용
  wattron(win2, COLOR_PAIR(1));
  wbkgd(win2, COLOR_PAIR(1));
  wattroff(win2, COLOR_PAIR(1));

  // 점수창 초기 텍스트 설정
  wattron(win2, COLOR_PAIR(4));
  mvwprintw(win2, 1, 1, "Score Board");
  mvwprintw(win2, 3, 1, "B: %d / 0", body.size()+1);
  mvwprintw(win2, 4, 1, "+: %d", growth_cnt);
  mvwprintw(win2, 5, 1, "-: %d", poison_cnt);
  mvwprintw(win2, 6, 1, "G: 0");
  mvwprintw(win2, 8, 1, "PlayTime : %d", playtime);
  wattroff(win2, COLOR_PAIR(4));

  // 미션 윈도우 초기 색상 적용
  wattron(win3, COLOR_PAIR(1));
  wbkgd(win3, COLOR_PAIR(1));
  wattroff(win3, COLOR_PAIR(1));

  // 미션창 초기 텍스트 설정 현재 목표 미션과 현재 생태를 확인할 수 있다.
  wattron(win3, COLOR_PAIR(4));
  mvwprintw(win3, 1, 1, "Mission");
  mvwprintw(win3, 3, 1, "B: %d (%d)",missionvec[level][0],body.size()+1);
  mvwprintw(win3, 4, 1, "+: %d (%d)",missionvec[level][1] , growth_cnt);
  mvwprintw(win3, 5, 1, "-: %d (%d)",missionvec[level][2] , poison_cnt);
  mvwprintw(win3, 6, 1, "G: ()");
  wattroff(win3, COLOR_PAIR(4));

  // 미션을 충족했으면 미션 클리어 변수에 true; 하고 레벨을 하나 올린다.
  if(missionvec[level][0] <= body.size()+1 && missionvec[level][1] <= growth_cnt && missionvec[level][2] <= poison_cnt)
  {
      IsMissionClear = true;
      level++;
      MapLoad(level);         // 다음 스테이지의 맵 로드
  }

  refresh();
  wrefresh(win1);
  wrefresh(win2);
  wrefresh(win3);
}
void update()
{
  // 입력 제어 쓰레드 생성
  thread key_thread(keyControl);

  Item item;
  srand((unsigned int)time(0));

  Timer levelTimer;
  Timer itemTimer;
  levelTimer.startTimer();
  itemTimer.startTimer();

  int playtime = 0;
  double itemtime;
  init_draw(playtime);

  int addY, addX;
  while(running)
  {
    init_draw(playtime);
    if(IsMissionClear)
    {
      running = false;
      break;
    }
    levelTimer.updateTime();
    itemTimer.updateTime();
    playtime = levelTimer.getPlayTime();
    itemtime = itemTimer.getTick();

    // 기존에 그려져 있던 Head, Body를 Map에서 삭제
    map[y][x] = 0;
    for(int i = 0; i < body.size(); i++)
      map[body[i].first][body[i].second] = 0;

    // body의 맨 마지막 위치 기억 (Potion 획득 시 꼬리 위치)
    addY = body[body.size()-1].first;
    addX = body[body.size()-1].second;

    // Head를 body[0]으로 가져오고, 나머지는 앞에서 이동한 위치로 따라서 한 칸씩 이동
    for(int i = body.size()-1; i > 0; i--)
      body[i] = body[i-1];
    body[0].first = y;
    body[0].second = x;

    // 아이템을 생성한다.
    if(itemtime > rand() % 3 + 4)
    {
      if(ItemContainer.size() < 3)
      {
        Item item;
        ItemContainer.push_back(item);

        itemTimer.startTimer();
      }
    }

    // 아이템 유효 시간 제어
    for(int i = 0; i < ItemContainer.size(); i++)
    {
      ItemContainer[i].spawnTime.updateTime();

      if(ItemContainer[i].spawnTime.getTick() >= 10) {
        map[ItemContainer[i].pos.x][ItemContainer[i].pos.y] = 0;
        ItemContainer.erase(ItemContainer.begin() + i);
      }
    }

    // 입력에 따른 다음 이동 처리
    if(key == KEY_LEFT)
      x--;
    else if(key == KEY_RIGHT)
      x++;
    else if(key == KEY_UP)
      y--;
    else if(key == KEY_DOWN)
      y++;
    if(map[y][x] == 1) // 지도를 벗어나거나 ESC를 입력하면 종료
    {
      running = false;
      break;
    }

    map[y][x] = 3;  // 이동할 위치의 계산이 끝나면 해당 위치에 Head 표시

    // Head가 아이템과 닿으면 ItemContainer에서 item을 삭제
    for(int i = 0; i < ItemContainer.size(); i++)
    {
      if(y == ItemContainer[i].pos.x && x == ItemContainer[i].pos.y)
      {
        // 증가 포션이면 기억해둔 꼬리 위치를 body에 추가
        if(ItemContainer[i].itemtype == 5)
        {
          growth_cnt++;
          body.push_back(pair<int, int>(addY, addX));
        }
        else if(ItemContainer[i].itemtype == 6) // 감소 포션이면 body의 길이 1 감소
        {
          poison_cnt++;
          body.pop_back();

          // body 길이가 2보다 작아지면 실패
          if(body.size() < 2)
            running = false;
        }

        ItemContainer.erase(ItemContainer.begin() + i);
        break;
      }
    }

    // Head가 body랑 충돌했을 때, 게임 종료
    for(int i = 0; i < body.size(); i++)
    {
      if(y == body[i].first && x == body[i].second)
      {
        running = false;
        break;
      }
    }

    // body 부분을 Map에 저장
    for(int i = 0; i < body.size(); i++)
      map[body[i].first][body[i].second] = 4;

    Gate();
    // 지도 업데이트 함수 호출
    map_update(false);

    // Window Refresh(변경사항 반영)
    refresh();
    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);

    this_thread::sleep_for(chrono::milliseconds(TICK_RATE));  // Tick Rate 마다 화면 갱신
  }

  // 입력 제어 쓰레드 종료를 먼저 기다린 뒤 함수 종료
  if(key_thread.joinable())
    key_thread.join();
}

// 비동기 키 입력 처리를 위한 함수
void keyControl()
{
  int input;

  while(running)
  {
    input = getch();

    if(input == KEY_UP || input == KEY_DOWN || input == KEY_LEFT || input == KEY_RIGHT)
    {
      // 입력 진행과 반대 방향의 키를 입력하면 게임 종료
      if((input == KEY_UP && key == KEY_DOWN) || (input == KEY_DOWN && key == KEY_UP)
        || (input == KEY_LEFT && key == KEY_RIGHT) || (input == KEY_RIGHT && key == KEY_LEFT))
        {
          running = false;
        }
      key = input;
    }
    else if(input == ESC)
    {
      running = false;
    }
  }
}

// 게임화면(win1) 갱신을 위한 함수
void map_update(bool itemIs)
{
  // 필드에 생성된 Item들의 위치의 색상을 변경
  for(int items = 0; items < ItemContainer.size(); items++)
  {
    if(ItemContainer[items].itemtype == 5)
      map[ItemContainer[items].pos.x][ItemContainer[items].pos.y] = 5;
    else
      map[ItemContainer[items].pos.x][ItemContainer[items].pos.y] = 6;
  }

  for(int i = 0; i < HEIGHT; i++)
  {
    for(int j = 0; j < WIDTH; j++)
    {
      if(map[i][j] == 2)                        // 게임화면의 Immune Wall 색상 적용
      {
        wattron(win1, COLOR_PAIR(2));
        mvwprintw(win1, i, j*2, "%d%d", map[i][j], map[i][j]);
        wattroff(win1, COLOR_PAIR(2));
      }
      else if(map[i][j] == 1)                   // 게임화면의 Wall 색상 적용
      {
        wattron(win1, COLOR_PAIR(1));
        mvwprintw(win1, i, j*2, "%d%d", map[i][j], map[i][j]);
        wattroff(win1, COLOR_PAIR(1));
      }
      else if(map[i][j] == 0)                   // 게임화면의 Field 색상 적용
      {
        wattron(win1, COLOR_PAIR(3));
        mvwprintw(win1, i, j*2, "%d%d", map[i][j], map[i][j]);
        wattroff(win1, COLOR_PAIR(3));
      }
      else if(map[i][j] == 3)                   // 게임화면의 Head 색상 적용
      {
        wattron(win1, COLOR_PAIR(5));
        mvwprintw(win1, i, j*2, "%d%d", map[i][j], map[i][j]);
        wattroff(win1, COLOR_PAIR(5));
      }
      else if(map[i][j] == 4)                   // 게임화면의 Body 색상 적용
      {
        wattron(win1, COLOR_PAIR(6));
        mvwprintw(win1, i, j*2, "%d%d", map[i][j], map[i][j]);
        wattroff(win1, COLOR_PAIR(6));
      }
      else if(map[i][j] == 5)                   // 게임화면의 Growth Potion 색상 적용
      {
        wattron(win1, COLOR_PAIR(7));
        mvwprintw(win1, i, j*2, "%d%d", map[i][j], map[i][j]);
        wattroff(win1, COLOR_PAIR(7));
      }
      else if(map[i][j] == 6)                   // 게임화면의 Poision Potion 색상 적용
      {
        wattron(win1, COLOR_PAIR(8));
        mvwprintw(win1, i, j*2, "%d%d", map[i][j], map[i][j]);
        wattroff(win1, COLOR_PAIR(8));
      }
      else
      {
        wattron(win1, COLOR_PAIR(8));
        mvwprintw(win1, i, j*2, "%d%d", map[i][j], map[i][j]);
        wattroff(win1, COLOR_PAIR(8));
      }
    }
  }
}

//게임화면 리셋을 위한 함수
void init_reset()
{
  key = KEY_LEFT;
  growth_cnt = 0;
  poison_cnt = 0;
  for(int i = 0; i < body.size(); i++)
    map[body[i].first][body[i].second] = 0;
  for(int items = 0; items < ItemContainer.size(); items++)
  {
    map[ItemContainer[items].pos.x][ItemContainer[items].pos.y] = 0;
  }
  ItemContainer.clear();
  body.clear();
  map[y][x] = 0;
  // 초기 설정 관련
  initscr();                                  // 스크린 활성화
  resize_term(HEIGHT + 2, WIDTH * 2 + 30);    // 화면 크기 설정 (점수 표시를 위한 공간 포함)
  box(stdscr, 0, 0);                          // Default Window 에 기본 테투리 설정
  curs_set(0);                                // 커서(깜빡임) 비활성화
  keypad(stdscr, TRUE);                       // 특수키(ESC, 방향키 등) 사용 가능하도록 설정
  noecho();
  timeout(0);

  // Default 또는 새로운 윈도우에서 사용할 Color Set 설정
  start_color();
  init_color(COLOR_BLACK, 0, 0, 0);               // 검은색 Color를 완전히 어두운 색으로 설정
  init_color(COLOR_CYAN, 500, 500, 500);          // 기본 Color로 Gray 색상이 제공되지 않기 때문에 CYAN Color를 회색으로 변경
  init_color(COLOR_MAGENTA, 500, 0, 500);          // 기본 Color로 Gray 색상이 제공되지 않기 때문에 CYAN Color를 회색으로 변경

  init_pair(1, COLOR_CYAN, COLOR_CYAN);           // 1번 쌍에 모서리(Wall), 점수, 미션 윈도우 색상 설정
  init_pair(2, COLOR_BLACK, COLOR_BLACK);         // 2번 쌍에 각 꼭짓점(Immune Wall) 색상 설정
  init_pair(3, COLOR_WHITE, COLOR_WHITE);         // 3번 쌍에 게임화면의 배경 색상(흰색) 설정
  init_pair(4, COLOR_BLACK, COLOR_CYAN);          // 4번 쌍에 점수 윈도우에서 폰트로 사용할 색상 설정
  init_pair(5, COLOR_GREEN, COLOR_GREEN);         // 5번 쌍에 Snake Head 색상 설정
  init_pair(6, COLOR_RED, COLOR_RED);             // 6번 쌍에 Snake Body 색상 설정
  init_pair(7, COLOR_YELLOW, COLOR_YELLOW);     // 7번 쌍에 Growth Potion 색상 설정
  init_pair(8, COLOR_MAGENTA, COLOR_MAGENTA);     // 8번 쌍에 Poision Potion 색상 설정

  //
  int k = 0;
  init_draw(k);
  y = HEIGHT/2, x = WIDTH/2;
  // Immune Wall, Wall, Field와 Character의 Head, Body 초기 설정bool
  map[0][0] = map[0][WIDTH-1] = 2;
  map[HEIGHT-1][0] = map[HEIGHT-1][WIDTH-1] = 2;
  for(int i = 1; i < WIDTH-1; i++)
    map[0][i] = map[HEIGHT-1][i] = 1;
  for(int i = 1; i < HEIGHT-1; i++)
    map[i][0] = map[i][WIDTH-1] = 1;
  map[y][x] = 3;

  // Head 기준으로 오른쪽으로 Body 초기 설정
  body.push_back(pair<int, int>(y, x+1));
  body.push_back(pair<int, int>(y, x+2));
}

void Gate()
{
  if(!IsGate)
  {
    gate_in[0][0] = gate_in[1][0];
    gate_in[0][1] = gate_in[1][1];
    gate_out[0][0] = gate_out[1][0];
    gate_out[0][1] = gate_out[1][1];

    while(map[gate_in[1][0]][gate_in[1][1]] != 1)
    {
      gate_in[1][0] = (rand() % (HEIGHT-1)) + 1;
      gate_in[1][1] = (rand() % (WIDTH-1)) + 1;
    }

    while(map[gate_out[1][0]][gate_out[1][1]] != 1 || (gate_in[1][0] == gate_out[1][0] && gate_in[1][1] == gate_out[1][1]))
    {
      gate_out[1][0] = (rand() % (HEIGHT-1)) + 1;
      gate_out[1][1] = (rand() % (WIDTH-1)) + 1;
    }

    map[gate_in[0][0]][gate_in[0][1]] = 1;
    map[gate_out[0][0]][gate_out[0][1]] = 1;

    map[gate_in[1][0]][gate_in[1][1]] = map[gate_out[1][0]][gate_out[1][1]] = 7;

    IsGate = true;
  }
}

// 종료 전 메모리 반환을 위한 함수
void exit()
{
  // 각 윈도우를 위해 할당한 메모리 반환
  delwin(win1);
  delwin(win2);
  delwin(win3);

  // 종료
  endwin();
}
