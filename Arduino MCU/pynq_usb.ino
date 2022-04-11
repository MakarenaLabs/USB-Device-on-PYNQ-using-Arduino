#include "HID-Project.h"
#include "pins_arduino.h"

const int pinLed = LED_BUILTIN;
const int pinButton = 2;
#define UL unsigned long
#define US unsigned short

int ss_pin = PB7;

void SlaveInit(void) {
  Serial1.begin(115200);
}

//WW
void setup() {
  
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, HIGH);

  SlaveInit();  // set up SPI slave mode
  delay(1000);
  
  // Sends a clean report to the host. This is important on any Arduino type.
  BootKeyboard.begin();
}

void loop() {


  while (!Serial1.available()){;}
  char c = Serial1.read();
  if(c == 'W'){
    digitalWrite(pinLed, LOW);
    BootKeyboard.write(KEY_W);
    delay(1000);
    digitalWrite(pinLed, HIGH);
  }

}
