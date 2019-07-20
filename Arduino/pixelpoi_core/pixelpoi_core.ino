#include <FastLED.h>
#include <SoftwareSerial.h>
#include "characters.h"
#include "images.h"

#define LED_PIN 7
#define BUTTON_PIN 5
#define NUM_LEDS 16
#define DEFAULT_COLOR CRGB(50, 50, 50)
#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8
#define IMG_WIDTH 16
#define IMG_HEIGHT 16

/* for BT */
SoftwareSerial BT(8, 9); // for bluetooth serial
String strReceive = ""; // record received string
int terminated = 0; // indicate whether the string is finished

/* for leds */
int blink_interval = 8;
int printOffset = 0; // currently only for printing letters
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



int btnHandler(){
  /* button handling */
  int btnRead = digitalRead(BUTTON_PIN); // read the state of the pushbutton value:
  int valReturn = 0; // if indexDisplay or mode is changed return 1, otherwise 0

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
      valReturn = 1;
    }
    else{
      // long press
      if (mode == 0){
        mode = 1;
        valReturn = 1;
      }
    }
    
    btnState = btnRead;
    lastValidChangeTimeStamp = millis();
  }
  return valReturn;
}



int printImage(byte img[][16]){
  int terminate = 0;
  
  // print the image
  for (int indexCol=0; indexCol<IMG_WIDTH; indexCol++){
    for (int indexRow=0; indexRow<IMG_HEIGHT; indexRow++){
      leds[indexRow] = colorTable[img[indexCol][indexRow]];
    }
    FastLED.show();
    delay(blink_interval);

    // button handling
    terminate = btnHandler();
    if (terminate){
      return terminate;
    }
  }

  // print the gap between columns
  for (int i=0; i<NUM_LEDS; i++){
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  delay(blink_interval);

  // button handling
  terminate = btnHandler();
  if (terminate){
    return terminate;
  }

  return 0;
}



int printable(char c){
  return (int)c>=32 && (int)c<=126;
}


int printLetter(char letter, CRGB color){
  int terminate = 0;
  if (!printable(letter)){
      return;
    }
  letter -= 32;
  
  // print the letter
  for (int indexCol=0; indexCol<CHAR_WIDTH; indexCol++){
    for (int indexRow=0; indexRow<CHAR_HEIGHT; indexRow++){
          bool current_bit = bitRead(font[letter][indexCol], 7-indexRow);
          if (current_bit)
            leds[indexRow + printOffset] = color;
          else
            leds[indexRow + printOffset] = CRGB(0, 0, 0);
          //Serial.print(current_bit);
    }
    //Serial.println();
    FastLED.show();
    delay(blink_interval);

    // button handling
    terminate = btnHandler();
    if (terminate){
      return terminate;
    }
  }

  // print the gap between letters
  for (int i=0; i<NUM_LEDS; i++){
    leds[i] = CRGB(0, 0, 0);
    //Serial.print(0);
  }
  //Serial.println();
  FastLED.show();
  delay(blink_interval);

  // button handling
  terminate = btnHandler();
  if (terminate){
    return terminate;
  }

  return 0;
}


void printString(String &str){
  for (int index=0; index<(int)(str.length()); index++){
    int terminate = printLetter(str[index], DEFAULT_COLOR);
    // quit if mode is changed before string finish
    if (terminate){
      delay(1000);
      return;
    }
  }
}



class Demo{
  public:
  Demo();
  static void Helloworld();
  static void Pangram();
  static void MySuperG();
};
void Demo::Helloworld(){
  static String strHelloworld = "Hello World! ";
  printString(strHelloworld);
}
void Demo::Pangram(){
  static String strPangram = "The quick onyx goblin jumps over the lazy dwarf. ";
  printString(strPangram);
}
void Demo::MySuperG(){
  printImage(img);
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
  btnHandler();
  
  
  /* bluetooth data-transfer mode */
  if (mode == 1){
    Serial.println("BT mode");
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
    Serial.print("Current mode: 0  Page: ");
    Serial.println(indexDisplay);
    switch(indexDisplay){
      case 0:
        break;
      case 1:
        Demo::Helloworld();
        break;
      case 2:
        Demo::Pangram();
        break;
      case 3:
        break;
      case 4:
        Demo::MySuperG();
        break;
    }
  }
}
