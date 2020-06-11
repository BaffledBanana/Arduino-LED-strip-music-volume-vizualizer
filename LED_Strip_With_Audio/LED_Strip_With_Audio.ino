#include <FastLED.h>

const int led_count = 30; //max | bars for a max amplitude
const int rmsHistory = 4; //how many values of amplitude history to store

int analogPin = A0;
int signalStrength;
int upper = 20;
int addaptiveUpper = true;
int lower = 15;
int addaptiveLower = false;
int h = 0;

int rmsValues[rmsHistory];
int rmsCounter = 0;

char levelStr[led_count];

CRGB leds[led_count]; //its actually BRG

//HSV used to easily make the leds slowly do a rainbow color effect
struct HSV { //hue, saturation, value
  int h = 0; //0 - 359
  float s = 0; //0 - 1
  float v = 0; //0 - 1
  };

struct HSV hsv;

void setup() {
  Serial.begin(9600);
  signalStrength = analogRead(analogPin);
  lower = signalStrength;
  //set all rms value history to 0
  for(int i = 0; i < rmsHistory; i++){
    rmsValues[i] = 0;
  }

  rmsCounter = 0;
  h = 0;

  FastLED.addLeds<WS2812B, 12, RGB>(leds, led_count);

  hsv.h = SetH(289);
  hsv.s = SetS(0.9f);
  hsv.v = SetV(0.9f);

  Clear();
  for(int x =0; x < led_count; x++){
      leds[x] = HSVtoRGB(hsv);
  } 
  FastLED.show();
  FastLED.show();
  FastLED.show();
}

void loop() {
  signalStrength = analogRead(analogPin);
  //Serial.println(signalStrength);
  
  FloorSignal();

  //DebugSignal();

  signalStrength = map(signalStrength, lower, upper, 0, led_count);

  /*Serial.print("Signal: ");
  Serial.print(signalStrength);*/
  //Serial.print(CalcRMS(signalStrength)); 
  int level = CalcRMS(signalStrength);
  PrintLevel(level);
  //PrintLevel(signalStrength);

  hsv.h = SetH(h);
  h++;

  Clear();
  for(int x =0; x < level; x++){
      leds[x] = HSVtoRGB(hsv); //CRGB(255, 208, 0) purple
  } 

  //DebugColor(HSVtoRGB(hsv), hsv);

  FastLED.show();
}

void DebugSignal(){
  Serial.println();
  Serial.print("Lower: ");
  Serial.print(lower);
  Serial.print(" || RAW Signal: ");
  Serial.print(signalStrength);
  Serial.print(" || Upper: ");
  Serial.print(upper);
  Serial.println();
}

void DebugColor(CRGB color, struct HSV hsv){
  Serial.println("RGB:");
  Serial.print("B: ");
  Serial.print(color.b);
  Serial.print(" || R: ");
  Serial.print(color.r);
  Serial.print(" || G: ");
  Serial.print(color.g);
  Serial.println("HSV");
  Serial.print("H: ");
  Serial.print(hsv.h);
  Serial.print(" || S: ");
  Serial.print(hsv.s);
  Serial.print(" || V: ");
  Serial.print(hsv.v);
}

void PrintLevel(int level){
  //clear string
  for(int i = 0; i < led_count; i++){
    levelStr[i] = ' ';
  }
  //levelStr[led_count - 1] = '\n';
  //add bars
  for(int i =0; i <= level - 1; i++){
    levelStr[i] = '|';
  }
  Serial.write(levelStr, led_count -1 );
  Serial.print("\n");
}

void FloorSignal(){
  if(addaptiveUpper){
    if(signalStrength > upper){upper = signalStrength;}
  }else{
    if(signalStrength > upper){ signalStrength = upper;} //floor it
  }

  if(addaptiveLower){
    if(signalStrength < lower){lower = signalStrength;}
  }else{
    if(signalStrength < lower){ signalStrength = lower;} //floor it
  }

  if(signalStrength < 0){signalStrength = 0; lower = 0;} //shouldnt happen but eh
  
}

int CalcRMS(int newVal){
  if(rmsCounter = rmsHistory){
    //move them all back one index
    for(int i = 0; i <= rmsHistory - 1; i++){
      rmsValues[i] = rmsValues[i+1];
    }
    //add new val to the beggining of the line
    rmsValues[rmsHistory - 1] = newVal;

    //calculate rms
    int RMS = 0;
    float sum = 0;
    for(int i = 0; i <= rmsHistory - 1; i++){
      sum += rmsValues[i]*rmsValues[i];
    }
    sum = sum/(rmsHistory - 1);
    RMS = (int)sqrt(sum);
    return RMS;
  }else{
    rmsValues[rmsCounter] = newVal;
    rmsCounter++;  
    return 0;
  }
}

void Clear(){
    for(int x =0; x < led_count; x++){
      leds[x] = CRGB(0,0,0);
      }
  }

int SetH(int h){
  if(h >= 0){return h - ((int)h/360)*360;}
  else{return h + ((int)-h/360)*360;}
  }
float SetS(float s){
  if(s >= 0){return s - ((int)s);}
  else{return s + ((int)-s);}
  }
float SetV(float v){
  if(v >= 0){return v - ((int)v);}
  else{return v + ((int)-v);}
  }

//my algorythm for converting from HSV to RGB. Works perfectly
CRGB HSVtoRGB(struct HSV hsv){

  //error handle
  //if(hsv.h < 0){hsv.h = 0;}
  //if(hsv.h > 359){hsv.h = 359;}
  if(hsv.s < 0){hsv.s = 0;}
  if(hsv.s > 1){hsv.s = 1;}
  if(hsv.v > 1){hsv.v = 1;}
  if(hsv.v < 0){hsv.v = 0;}
  
  float C = (float)(hsv.v * hsv.s);
  float X = (float)(C * (1-Modulis(Mod(((float)hsv.h/60),2)-1)));
  float m = (float)(hsv.v - C);

  float r = 0; //0 - 255
  float g = 0; //0 - 255
  float b = 0; //0 - 255
  
  if(hsv.h >= 0 && hsv.h < 60){r = C; g = X; b = 0;}
  else if(hsv.h >= 60 && hsv.h < 120){r = X; g = C; b = 0;}
  else if(hsv.h >= 120 && hsv.h < 180){r = 0; g = C; b = X;}
  else if(hsv.h >= 180 && hsv.h < 240){r = 0; g = X; b = C;}
  else if(hsv.h >= 240 && hsv.h < 300){r = X; g = 0; b = C;}
  else if(hsv.h >= 300 && hsv.h < 360){r = C; g = 0; b = X;}
  /*
  struct MyRGB rgb;
  rgb.r = (g+m)*255;
  rgb.g = (r+m)*255;
  rgb.b = (b+m)*255; */

  return CRGB((int)((b+m)*255),(int)((r+m)*255),(int)((g+m)*255));
}


float Modulis(float n){
  if(n >= 0){return n;}
  else{return -n;}
  }

float Mod(float n, float k){
  while(n >= k){
    n -= k;
    }
    return n;
  }
