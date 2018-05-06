#include "Button.h"

#define PRESSED     HIGH
#define NOT_PRESSED LOW

#define DEBOUNCE_TIME   200

Button::Button(int pin)
  : pin(pin), current(NOT_PRESSED)
{
  pinMode(this->pin, INPUT);
}


int Button::read() {
  current = digitalRead(pin);

  // If the button state changes to pressed, remember start time
  if (current == PRESSED 
      && previous == NOT_PRESSED 
      && (millis() - firstTime) > DEBOUNCE_TIME) {
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
    return -1;
  }
}

long Button::getMilli() {
  if(this->read() == PRESSED) {
    return this->millis_held;
  } else {
    return -1;
  }
}
