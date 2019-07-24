#include <FastLED.h>
#include <SoftwareSerial.h>
#include "characters.h"
#include "images.h"

#define LED_PIN 7
#define BUTTON_PIN 5
#define BAT_PIN 4 // battery charging indication pin
#define NUM_LEDS 16
#define DEFAULT_COLOR CRGB(200, 200, 200)
#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8
#define IMG_WIDTH 16
#define IMG_HEIGHT 16

/* for BT */
SoftwareSerial BT(8, 9); // for bluetooth serial
char strReceive[64] = {0}; // record received string
int terminated = 0; // indicate whether the string is finished

/* for leds */
int blink_interval = 8;
int printOffset = 0; // currently only for printing letters
CRGB leds[NUM_LEDS];

/* for button and mode switching */
int btnState = 0;         // variable for reading the pushbutton status
int lastState = 0;
int bounceDelay = 128;
unsigned long lastValidChangeTimeStamp = 0; // the int in arduino is actually int16
int cnt = 0;
int cntClick = 0;
int cntLongPress = 0;
int thresLongPress = 500;
int mode = 0; // 0: display  1: bt_trans 2: charging
int prev_mode = 0; // store mode before charging
int indexDisplay = 1; // 0: custom  1~4: demo1~4 (helloworld, pangram, clock, image)



int chargingHandler(){
  int charging = digitalRead(BAT_PIN);
  if (charging){
    if(mode != 2){
      prev_mode = mode;
      mode = 2; 
    }
  }else if (mode == 2){
    mode = prev_mode;
    return 1; // if switch back, return 1
  }
  return 0;
}


int btnHandler(){
  // showing charging animation
  chargingHandler();
  
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
      Serial.println("Clicked");
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
      Serial.println("Long Pressed");
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



void flashColor(CRGB color){
  for (int i = 0; i < NUM_LEDS; i++){
    leds[i] = color;
  }
  FastLED.show();
  delay(blink_interval*10);
  btnHandler();
  for (int j = 0; j < NUM_LEDS; j++){
    for (int i = 0; i <= j; i++){
      leds[i].fadeToBlackBy( 64 );
    }
    FastLED.show();
    delay(blink_interval*10);
    btnHandler();
  }
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
    /*
    if (terminate){
      return terminate;
    }*/
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
    /*
    if (terminate){
      return terminate;
    }*/
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
  terminate |= btnHandler();
  if (terminate){
    return terminate;
  }

  return 0;
}


void printString(char* str){
  for (int index=0; index<(int)(strlen(str)); index++){
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
  static void Math();
};
void Demo::Helloworld(){
  static char* strHelloworld = "Hello World! ";
  printString(strHelloworld);
}
void Demo::Pangram(){
  static char* strPangram = "The quick onyx goblin jumps over the lazy dwarf. ";
  printString(strPangram);
}
void Demo::MySuperG(){
  printImage(bmpMySuperG);
}
void Demo::Math(){
  static char* strMath = "1+1=2 ";
  printString(strMath);
}



void setup() {
  Serial.begin(9600);
  BT.begin(9600);
  Serial.println("BT is ready!");
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS); // I'm actually using WS2812B, however the colors are inaccurate
  pinMode(BUTTON_PIN, INPUT); // pin 5 is for mode switching button
  pinMode(BAT_PIN, INPUT); // pin 4 is for charging detection
}

void loop() {
  /* button handling */
  btnHandler();


  /* charging mode */
  if (mode == 2){
    Serial.println("Charging");
    flashColor(CRGB::Green);
  }  
  

  /* bluetooth data-transfer mode */
  if (mode == 1){
    Serial.println("BT mode");
    flashColor(CRGB::Blue);
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
        strcpy(strReceive, "");
        terminated = 0;
      }
      if (printable(buf)){
        strcat(strReceive, buf);
      }else if ((int)strlen(strReceive)>0){
        // any non-printable char indicates the end of the string
        // if the string length is positive it's valid
        Serial.print("Received string is : ");
        Serial.println(strReceive);
        Serial.print("String length : ");
        Serial.println((int)strlen(strReceive));
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
        Demo::Math();
        break;
      case 4:
        Demo::MySuperG();
        break;
    }
  }
}
