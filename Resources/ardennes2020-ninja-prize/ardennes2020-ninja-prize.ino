unsigned char leds[] = {7, 8};
unsigned char nrLeds = 2;

short brightness = 0;    // how bright the LED is
char fadeAmount = 5;    // how many points to fade the LED by

// the setup routine runs once when you press reset:
void setup() {
  for(byte i = 0; i < nrLeds; i++) {
    pinMode(leds[i], OUTPUT);
  }
}

// the loop routine runs over and over again forever:
void loop() {  
  for(int i = 0; i < nrLeds; i++) {
    analogWrite(leds[i], brightness);
  }
  
  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 255) {
    fadeAmount = -fadeAmount;
  }
  
  // wait for 30 milliseconds to see the dimming effect
  delay(30);
}
