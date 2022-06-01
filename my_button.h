#ifndef MY_BUTTON
#define LED_BAR

class MyButton {
  private:
    int pin;
    
  public:
    MyButton(int p);
    void setupPin();
    bool isDePressed();
    bool isLongPress();
};

#endif
