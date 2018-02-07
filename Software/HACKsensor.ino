/*__________________________
HACKBerry original code
キャリブレーションテストプログラム03
 小笠原佑樹
 2015/11/19
 
 ＠プロセス
 キャリブレーションボタンを押す
 ↓
 500m待機s
 ↓
 4500msセンサの取得値と指の動作速度を比例させてキャリブレーションを継続
 ↓
 キャリブレーション脱出。通常動作。

 _______________________
ECE Paris PFE1704
  01/2018
  HACKSensor
  Adding feedback functions and changes in the servomotors positions
 */


#include <Servo.h>

//Micro
const int calibPin0 =  A6; //set the MAX value of the sensor input
const int calibPin1 =  A5; //set the MIN value of the sensor input
const int thumbPin =  A4; //change the thumb position among three preset values
const int fingerPin =  A3; //lock or unlock the position of middle finger, ring finger and pinky
const int analogInPin0 = A0; //sensor input
const int feedbackPin = A11; // Strain gauge input

Servo myservo0; //controls index finger
Servo myservo1; //controls middle finger, ring finger and pinky
Servo myservo2; //controls thumb
Servo feedbackServo; // controls servo

float target = 0;
boolean thumbPinState = 1;
boolean fingerPinState = 1;

int count = 0;
int mode = 0;
int val = 0;

int feedbackPosition = 0; //position of the feedback servo

int swCount0 = 0;
int swCount1 = 0;
int swCount2 = 0;
int swCount3 = 0;

int sensorValue = 0; // value read from the myoware sensor
int sensorMax = 700;
int sensorMin = 0;
int threshold = 0;


//speed settings
int speedMax = 7;
int speedMin = 0;
int speedReverse = -3;
int speed = 0;

int positionMax = 150;
int positionMin = 0;
int position =0;
int prePosition = 0;

int thumbPinch = 45;
int thumbOpen = 110;


int indexMin = 180;//extend
int indexMax = 45;//flex

int middleMin = 110;//extend 
int middleMax = 45;//flex

int thumbPos = 90;
int indexPos = 90;
int middlePos = 90;

void setup() {
  Serial.begin(9600); 

  pinMode(calibPin0, INPUT); //MAX A6
  digitalWrite(calibPin0, HIGH);

  pinMode(calibPin1, INPUT); //MIN A5
  digitalWrite(calibPin1, HIGH);

  pinMode(thumbPin, INPUT); //A4
  digitalWrite(thumbPin,HIGH);

  pinMode(fingerPin, INPUT); //A3
  digitalWrite(fingerPin,HIGH);

  pinMode(feedbackPin, INPUT); //A11


  myservo0.attach(6);//index 
  myservo1.attach(5);//middle
  myservo2.attach(3);//thumb
  feedbackServo.attach(11); //feedback servo
}

void loop() {


  //===============小笠原追加====================
  while(1) {
    myservo0.write(indexMin);  //indexサーボを初期位置にする。
    myservo1.write(middleMin);  //middleサーボを初期位置にする。
    myservo2.write(thumbOpen);  //Thumb open
    Serial.println("Waiting for Calibration...");
    delay(10);
    if(digitalRead(calibPin0) == LOW) {
      calibration();
      break;
    }
  }
  //===========================================

  while(1) {

    sensorValue = ReadSens_and_Condition();
    delay(25);

    if(digitalRead(calibPin0) == LOW){//A6
      swCount0 += 1;
    }
    else{
      swCount0 = 0;
    }

    if(swCount0 == 10){
      swCount0 = 0;
      //sensorMax = ReadSens_and_Condition();  
      calibration();

    }

    if(digitalRead(calibPin1) == LOW){//A5
      swCount1 += 1;
    }
    else{
      swCount1 = 0;
    }

    if(swCount1 == 10){
      swCount1 = 0;
      sensorMin = ReadSens_and_Condition() + threshold;
    }

    if(digitalRead(thumbPin) == LOW){//A4
      swCount2 += 1;
    }
    else{
      swCount2 = 0;
    }

    if(swCount2 == 8){
      swCount2 = 0;
      thumbPinState = !thumbPinState;    
      while(digitalRead(thumbPin) == LOW){
        delay(1);
      }    
    }


    if(digitalRead(fingerPin) == LOW){//A3
      swCount3 += 1;
    }
    else{
      swCount3 = 0;
    }
    if(swCount3 == 10){
      swCount3 = 0;
      fingerPinState = !fingerPinState;
      while(digitalRead(fingerPin) == LOW){
        delay(1);
      }    
    }


    //status
    Serial.print("Min=");
    Serial.print(sensorMin);
    Serial.print(",Max=");
    Serial.print(sensorMax);
    Serial.print(",Value=");
    Serial.print(sensorValue);
    Serial.print(",thumb=");
    Serial.print(thumbPinState);
    Serial.print(",finger=");
    Serial.print(fingerPinState);  
    Serial.print(",indexPos=");
    Serial.print(indexPos);  
    Serial.print(",thumb=");
    Serial.print(swCount3);
    Serial.print(",speed=");
    Serial.print(speed);

    Serial.print(",thumbPinState=");
    Serial.print(thumbPinState);
    Serial.print(",fingerPinState=");
    Serial.println(fingerPinState);  

    //calculate speed
    if(sensorValue < (sensorMin+(sensorMax-sensorMin)/8)){
      speed = speedReverse;
    }
    else if(sensorValue < (sensorMin+(sensorMax-sensorMin)/4)){
      speed = 0;
    }
    else{
      speed = map(sensorValue, sensorMin, sensorMax, speedMin, speedMax);
    }

    //calculate position
    position = prePosition + speed;
    if(position < positionMin) position = positionMin;
    if(position > positionMax) position = positionMax;
    prePosition = position;
    //motor
    indexPos=map(position,positionMin,positionMax,indexMin, indexMax);

    myservo0.write(indexPos);


    if(fingerPinState == HIGH){
      middlePos=map(position,positionMin,positionMax,middleMax, middleMin);
      myservo1.write(middlePos);


      Serial.print("Min=");
      Serial.print(positionMin);
      Serial.print("Max=");
      Serial.print(positionMax);   
      Serial.print("MinMap=");
      Serial.print(middleMin);
      Serial.print("MaxMap=");
      Serial.print(middleMax);   
      Serial.print("Pos=");
      Serial.print(middlePos);
    
    }

    switch(thumbPinState){
    case 0://pinch
      myservo2.write(thumbPinch);
      delay(10);
      myservo2.write(thumbPinch);
      delay(10);
      myservo2.write(thumbPinch);
      break;
    case 1://open
     myservo2.write(thumbOpen);
      delay(10);
      myservo2.write(thumbOpen);
      delay(10);
      myservo2.write(thumbOpen);
      break;
    default:
      break;
    }

    // The feedback happens after the movement
    feedbackFunction(); 

  }
}


//センサ読み取り
int ReadSens_and_Condition() {
  int i;
  int sval;
  for(i= 0; i<20; i++){
    sval = sval + analogRead(analogInPin0); //for other SensorBoard 
  }
  sval = sval/20;
  return sval;
}



//=================================以下小笠原追加===================================

int sensorMin_temp;

void calibration() {

  myservo0.write(indexMin);  //index
  myservo1.write(middleMax);  //middle
  myservo2.write(thumbOpen);  //Thumb open

  Serial.println("please wait...");
  delay(500);
  Serial.println("start");

  sensorMin = ReadSens_and_Condition();
  sensorMax = sensorMin+1;

  unsigned long time = millis();

  while( millis() < time+7500 ) {

    sensorValue = ReadSens_and_Condition();
    delay(25);

    if( sensorValue < sensorMin ) {
      sensorMin = sensorValue;
      sensorMin_temp = sensorMin + (sensorMax-sensorMin)/4;
    }
    else if( sensorValue > sensorMax )sensorMax = sensorValue;
    else;

    //calculate speed
    if(sensorValue < (sensorMin_temp+(sensorMax-sensorMin)/8)){
      speed = speedReverse;
    }
    else if(sensorValue < (sensorMin_temp+(sensorMax-sensorMin)/4)){
      speed = 0;
    }
    else{
      speed = map(sensorValue, sensorMin_temp, sensorMax, speedMin, speedMax);
    }

    //calculate position
    position = prePosition + speed;
    if(position < positionMin) position = positionMin;
    if(position > positionMax) position = positionMax;
    prePosition = position;
    //motor
    indexPos=map(position,positionMin,positionMax,indexMin, indexMax);
    myservo0.write(indexPos);


    Serial.print("IndexPos=");
    Serial.print(indexPos);
    Serial.print("Min=");
    Serial.print(sensorMin);
    Serial.print(",Min_temp=");
    Serial.print(sensorMin_temp);
    Serial.print(",Max=");
    Serial.print(sensorMax);
    Serial.print(",Value=");
    Serial.print(sensorValue);
    Serial.print(",time=");
    Serial.print(time);
    Serial.print(",millis=");
    Serial.print(millis());
    Serial.print("Button=");
    Serial.println(digitalRead(calibPin0));


  }
  sensorMin += (sensorMax-sensorMin)/4;
}


//=================================Feedback===================================//
void feedbackFunction()
{
  int feedbackValue = 0;

  feedbackValue = analogRead(feedbackPin);

  if(feedbackValue < 600)
    feedbackServo.write(0);
  else if (feedbackValue > 970)
    feedbackServo.write(90);
  else if (feedbackValue > 600 && feedbackValue < 700)
  {
    feedbackPosition = map(feedbackValue,600,700,0,45);
    feedbackServo.write(feedbackPosition);
  }
  else if (feedbackValue > 700 && feedbackValue < 970)
  {
    feedbackPosition = map(feedbackValue,700,970,45,90);
    feedbackServo.write(feedbackPosition);
  }
      Serial.print("feedbackValue=");
      Serial.print(feedbackValue);   
      Serial.print("feedbackPosition=");
      Serial.print(feedbackPosition);

}
