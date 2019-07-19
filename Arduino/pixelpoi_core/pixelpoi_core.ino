#include <FastLED.h>
#include <SoftwareSerial.h>
#include "characters.h"

#define LED_PIN 7
#define BUTTON_PIN 5
#define NUM_LEDS 16
#define DEFAULT_COLOR CRGB(50, 50, 50)
#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8

/* for BT */
SoftwareSerial BT(8, 9); // for bluetooth serial
String strReceive = ""; // record received string
int terminated = 0; // indicate whether the string is finished

/* for leds */
int blink_interval = 32;
CRGB leds[NUM_LEDS];

/* for button and mode switching */
int btnState = 0;         // variable for reading the pushbutton status
int lastState = 0;
int bounceDelay = 128;
int lastValidChangeTimeStamp = -1 * bounceDelay;
int cnt = 0;
int cntClick = 0;
int cntLongPress = 0;
int thresLongPress = 500;
int mode = 0; // 0: display  1: bt_trans
int indexDisplay = 1; // 0: custom  1~4: demo1~4 (helloworld, pangram, clock, image)



int printable(char c){
  return (int)c>=32 && (int)c<=126;
}


void printLetter(char letter, CRGB color){
  if (!printable(letter)){
      return;
    }
  letter -= 32;
  
  // print the letter
  for (int indexCol=0; indexCol<CHAR_WIDTH; indexCol++){
    for (int indexRow=0; indexRow<CHAR_HEIGHT; indexRow++){
          bool current_bit = bitRead(font[letter][indexCol], 7-indexRow);
          if (current_bit)
            leds[indexRow] = color;
          else
            leds[indexRow] = CRGB(0, 0, 0);
          Serial.print(current_bit);
    }
    Serial.println();
    FastLED.show();
    delay(blink_interval);
  }

  // print the gap between letters
  for (int i=0; i<NUM_LEDS; i++){
    leds[i] = CRGB(0, 0, 0);
    Serial.print(0);
  }
  Serial.println();
  FastLED.show();
  delay(blink_interval);
}


void printString(String &str){
  for (int index=0; index<(int)(str.length()); index++){
    printLetter(str[index], DEFAULT_COLOR);
  }
}



class Demo{
  public:
  Demo();
  static void Helloworld();
  static void Pangram();
};
void Demo::Helloworld(){
  static String strHelloworld = "Hello World! ";
  printString(strHelloworld);
}
void Demo::Pangram(){
  static String strPangram = "The quick onyx goblin jumps over the lazy dwarf. ";
  printString(strPangram);
}



void setup() {
  Serial.begin(9600);
  BT.begin(9600);
  Serial.println("BT is ready!");
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  pinMode(BUTTON_PIN, INPUT); // pin 5 is for mode switching button
}

void loop() {
  /* button handling */
  int btnRead = digitalRead(BUTTON_PIN); // read the state of the pushbutton value:

  if (btnRead != btnState && ((millis()-lastValidChangeTimeStamp)>bounceDelay)){
    
    // LOW to HIGH
    if (btnRead==HIGH && btnState==LOW) {
      //Serial.println(++cnt);
    }
    
    // HIGH to LOW
    else if (millis()-lastValidChangeTimeStamp<thresLongPress){
      // single click
      if (mode == 0){
        indexDisplay ++;
        indexDisplay %= 5;
      }else{
        mode = 0;
      }
    }
    else{
      // long press
      if (mode == 0){
        mode = 1;
      }
    }
    
    btnState = btnRead;
    lastValidChangeTimeStamp = millis();
  }
  
  
  /* bluetooth data-transfer mode */
  if (mode == 1){
    char buf = 0;
    // data transfer from serial port to BT
    if (Serial.available()){
      buf = Serial.read();
      BT.print(buf);
    }
    if (BT.available()){
      // data transfer from BT to serial port
      buf = BT.read();
      Serial.println(buf);
  
      // process received data
      if (terminated){
        // initialize the string
        strReceive = "";
        terminated = 0;
      }
      if (printable(buf)){
        strReceive += buf;
      }else if ((int)strReceive.length()>0){
        // any non-printable char indicates the end of the string
        Serial.print("Received string is : ");
        Serial.println(strReceive);
        Serial.print("String length : ");
        Serial.println((int)strReceive.length());
        terminated = 1;
      }
    }
  }


  /* Display mode */
  if (mode == 0){
    switch(indexDisplay){
      case 0:
        break;
      case 1:
        Demo::Helloworld();
        break;
      case 3:
        Demo::Pangram();
        break;
      case 4:
        break;
    }
  }
}
