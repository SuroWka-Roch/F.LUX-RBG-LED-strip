#include <FastLED.h>
#define COLOR_ORDER GRB
#define NUM_LEDS 16
#define DATA_PIN 6
#define DAY_TIME 3600.0/6.0
#define TRANZISION_TIME DAY_TIME/20.0;

float brightness = 10.0;
CRGB leds[NUM_LEDS];

class State {
  public:
  uint8_t Red, Blue,Green;
  unsigned long timeOf = 0;
  State (int, int, int);
  State (int,int,int,unsigned long);
};

State day(255,255,255,DAY_TIME-DAY_TIME*(1.5/24.0)),
      afternoon(255,200,150,DAY_TIME*(9/24.0)),
      night(230,44,0,DAY_TIME*(15/24.0));
      
State tableOfDay[] = {day,afternoon,night};
unsigned long KeeperOfTheTime,offset;
int currentState = 0;

void setup() {
	// sanity check delay - allows reprogramming if accidently blowing power w/leds
   	delay(2000);
    Serial.begin(9600);
    pinMode(LED_BUILTIN,OUTPUT);
    KeeperOfTheTime = 0.0;
    offset =  0.0;
    FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
}

void loop() {
   unsigned long myTime = getTime();
   Serial.println(myTime);
   int tempState = shouldBeState(myTime);
   State curPal = SetPalette(tempState);
   setAllLEDS(curPal);
   FastLED.show();
   delay(200);
}

CRGB colour(uint8_t r,uint8_t b, uint8_t g){
  float wsp = (float)((brightness/100));
  r = (uint8_t)(int)(r*wsp);
  g = (uint8_t)(g*wsp);
  b = (uint8_t)(b*wsp);
  return CRGB(g,r,b);
}

void blink(int time) {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(time);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(time);
}

void errorBlink(int a){ 
  for(int i=0; i<a ;i++){
    blink(200);
  }
}

State::State(int r,int b,int g){
  Red = r;
  Blue = b;
  Green = g;  
}
State::State(int r,int b,int g,unsigned long t){
  Red = r;
  Blue = b;
  Green = g;
  timeOf = (unsigned long)t;
}

unsigned long sec(unsigned long myTime){
  return (unsigned long) myTime/1000;  
}
unsigned long hours(unsigned long sec){
  return (unsigned long) sec/3600;  
}

unsigned long getTime(){
	unsigned long myTime = sec(millis()) + offset;
	while(myTime>DAY_TIME){
		myTime = myTime - DAY_TIME;
	}
	if (KeeperOfTheTime >= DAY_TIME){  KeeperOfTheTime = myTime;  }
	else {    
    	if( KeeperOfTheTime > myTime ) { offset = KeeperOfTheTime - myTime; }
   	}
   KeeperOfTheTime = myTime;
   return myTime;
}

State SetPalette(int tempState){
	if (currentState!=tempState){
		makeTranzision(tableOfDay[currentState],tableOfDay[tempState],lineFun);
	}
	currentState = tempState;
	return tableOfDay[tempState];
}

int shouldBeState(unsigned long time){
	int tempState = 0;   
	int tableSize = sizeof(tableOfDay)/sizeof(*tableOfDay);
	for(int i = 0; i < tableSize;i++){
    	if(time<tableOfDay[i].timeOf){
      		tempState++;
    	} 
	}
	if(tempState == 0 ){ return tempState;	}
	else{
		return tableSize - tempState;
	}
	return tempState;
}
void makeTranzision(State curState, State toState,int tranzisionFun(int,int,float)){
	unsigned long beginTime = getTime();
	unsigned long endTime = beginTime + TRANZISION_TIME;
	while(endTime > getTime()){
		unsigned long tempTime = getTime();
		if (tempTime<beginTime){
			endTime = endTime - DAY_TIME;
		}
		float where =(float)(tempTime - beginTime)/(endTime- beginTime);
		Serial.println(where);
		State tempState = State( tranzisionFun(curState.Red,toState.Red,where),
								 tranzisionFun(curState.Blue,toState.Blue,where),
								 tranzisionFun(curState.Green,toState.Green,where));
		setAllLEDS(tempState);
		FastLED.show();
		delay(50);
	}
}

void setAllLEDS(State curPal){
	for(int i=0;i<NUM_LEDS;i++){
		leds[i] =  colour(curPal.Red,curPal.Blue,curPal.Green);
	}
}

int lineFun(int from, int to ,float x){
	//x is from 0 to 1
	int value= (int) from + (int)((float)(to - from)*x);
	Serial.println(value);
	return value;
}