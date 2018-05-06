#include "RGB_LED.h"
#include "Arduino.h"

RGB_LED::RGB_LED(int red, int green, int blue)
  : red(red), green(green), blue(blue) 
{
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
}

void RGB_LED::writeState(int r, int g, int b) {
  digitalWrite(this->red, r);
  digitalWrite(this->blue, b);
  digitalWrite(this->green, g);
}

void RGB_LED::blink(int r, int g, int b, long duration) {
  this->writeState(r, g, b);
  delay(duration);
}
