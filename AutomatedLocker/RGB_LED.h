#ifndef __LEDUTILS_H__
#define __LEDUTILS_H__

class RGB_LED {
  private:
    int red;
    int blue;
    int green;
  public:
    RGB_LED(int red, int green, int blue);
    void writeState(int r, int g, int b);
    void blink(int r, int b, int g, long delay);
};

#endif // __LEDUTILS_H__
