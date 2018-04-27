#include "Button.h"

#define PRESSED     HIGH
#define NOT_PRESSED LOW

#define TIME_DIFF   200
#define PUSH_DIFF

Button::Button(int pin)
  : pin(pin), current(NOT_PRESSED)
{
  pinMode(this->pin, INPUT);
}


int Button::read() {
  current = digitalRead(pin);

  // If the button state changes to pressed, remember start time
  if (current == PRESSED && previous == NOT_PRESSED && (millis() - firstTime) > TIME_DIFF) {
    firstTime = millis();
  }

  millis_held = (millis() - firstTime);
  secs_held = millis_held / 1000;

  previous = current;
  return current;
}

long Button::getSec() {
  if(this->read() == PRESSED) {
    return this->secs_held;
  } else {
    return 0;
  }
}

long Button::getMilli() {
  if(this->read() == PRESSED) {
    return this->millis_held;
  } else {
    return 0;
  }
}
