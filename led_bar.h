#ifndef LED_BAR
#define LED_BAR

class LedBar {
  private:
    int ledPins[6];
    int pinLevels[6];
    int brightness[6];
    int delays[6];
    int level;

  public:
    LedBar(int p1, int p2, int p3, int p4, int p5, int p6);
    void setLevel(int level);
    int * getLevels();
    void exception(int err, bool forever);
    void showIssue(int err, bool warning, bool forever);
};

#endif
