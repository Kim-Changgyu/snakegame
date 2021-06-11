// 시간 함수
class Timer {
public:
    clock_t startTime;
    clock_t currentTime;
    double tick;

    Timer() {};

    ~Timer() {};

    void startTimer(){
      startTime = clock();
    }

    void updateTime() {
      currentTime = clock();
      tick = (double)(currentTime - startTime)/CLOCKS_PER_SEC;
    }

    unsigned int getPlayTime() {
      return tick;
    }

    double getTick() {
      return tick;
    }
};
