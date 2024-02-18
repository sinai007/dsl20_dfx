#ifndef _togglebutton_h_
#define _togglebutton_h_
#include <Bounce.h>

class ToggleButton {

  public:
    ToggleButton(const char * pName, int pPin): name(pName) {
      //  name = pName;
       digitalPin = pPin;
       toggle = new Bounce( digitalPin, 5); 
    };
    ~ToggleButton() {
      delete toggle;
    };
    void setup() {
      pinMode(digitalPin, INPUT_PULLUP);
    };
    void update(void (*toggled)() ) {
      toggle -> update ( );
 
      // // Get the update value
      int value = toggle -> read();
      if( value != lastRead && value == 0) {
        buttonState = !buttonState;
        toggled();
      }
      lastRead = value;  
    };
    int buttonState = 1;
  private:
    const char * name;
    int digitalPin;
    int lastRead = 1;
    Bounce * toggle;
};

#endif