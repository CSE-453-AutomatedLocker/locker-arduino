#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Arduino.h"

class Button {
  public:
    Button(int pin);
    bool isPressed();
    bool isHold();
    bool isLong();
    int read();
    long getSec();
    long getMilli();

  private:
    int pin;
    int current;         // Current state of the button
    byte previous = LOW;
    
    long millis_held;    // How long the button was held (milliseconds)
    long secs_held;      // How long the button was held (seconds)
    long prev_secs_held;// How long the button was held in the previous check

    unsigned long firstTime; // how long since the button was first pressed 
};

#endif
