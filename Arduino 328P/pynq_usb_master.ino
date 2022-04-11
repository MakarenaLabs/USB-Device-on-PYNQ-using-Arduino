#include <Wire.h>

char buffer[100];
int incount=0;
int outcount=0;
int x=0;

#define MAX_CHAR 1
bool new_char = false;

void setup (void)
{
  Serial.begin(115200);
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  
}  // end of setup

void loop()
{
 if(new_char){
    new_char = false;
    //Serial.println("ricevuto!");    
    Serial.write(buffer[0]);    
    buffer[0] = 0;
 }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{

  new_char = true;
  buffer[0] = Wire.read();
}
