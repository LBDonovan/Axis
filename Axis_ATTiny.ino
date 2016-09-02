#include "TinyWireS.h"     
#include <AccelStepper.h>

#define I2C_SLAVE_ADDR  10  //CHANGE WHEN PROGRAMMING!!! (don't use 0 - reserved for broadcast)

//pins
const int SDAPin = 4;  //ICpin 7
const int SCKPin = 6;  //ICpin 9

const int stepPin = 10;  //ICPin 13 ??
const int dirPin = 2;  //ICPin 5

const int m0Pin = 9;  //ICPin 12
const int m1Pin = 8;  //ICPin 11
const int m2Pin = 7;  //ICPin 10

const int sleepPin = 5;  //ICPin8

const int homeSwitchPin = 3;  //ICPin6

//modes
int mode = 6; 
  //modes:
  //0 = paused 
  //1 = seeking
  //2 = constantvelocity
  //3 = homing
  //4 = offset
  //5 = newMode
  //6 = justbooted
int nextMode = 0;

//variables
int microStepping = 32;

boolean homed = false;

long targetPos = 0;
long targetVel = 0;
long targetMaxVel = 0;
long targetAcc = 0;

boolean posNeedsUpdate = false;
boolean velNeedsUpdate = false;
boolean maxVelNeedsUpdate = false;
boolean accNeedsUpdate = false;

byte message[8];

long maxSpeed =        200*microStepping;
long maxAccelaration = 200*microStepping;

AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);

void setup()
{    
  //oscillator calibration
  // not needed for crystal
  //OSCCAL = 0x79;
  
  //microstepping pins
  pinMode(m0Pin, OUTPUT);
  pinMode(m1Pin, OUTPUT);
  pinMode(m2Pin, OUTPUT);
  
  //set microstepping
  setMicroStepping(microStepping);
  
  //sleep pin
  pinMode(sleepPin, OUTPUT);
  digitalWrite(sleepPin, HIGH);
  
  //home switch - use internal pullup. pin is low when home.
  pinMode(homeSwitchPin, INPUT_PULLUP);
  
  // init I2C Slave mode
  TinyWireS.begin(I2C_SLAVE_ADDR);
  
  // I2C event handlers
  TinyWireS.onRequest(requestEvent);
  TinyWireS.onReceive(recieveI2C);

  stepper.setAcceleration(maxAccelaration);	
  stepper.setMaxSpeed(maxSpeed);
}

void loop()
{
  
  TinyWireS_stop_check();  //this is the fucker that makes it work
  
  if (mode == 1){  //normal 
    if (!stepper.run()){
      mode = 0;  //paused
    }    
  } else if(mode == 2){  //constant velocity 
    stepper.runSpeed();
  } else if(mode == 3){  //homing
    goHome();
  } else if(mode == 4){  //offset
    if (!stepper.run()){
      mode = 0;  //paused
      homed = true;
      stepper.setCurrentPosition(0);
    }
  }else if(mode == 5){  //newMode
    newMode();
  }
}

void newMode(){
  mode = nextMode;
  if (posNeedsUpdate){
    stepper.moveTo(targetPos);
    posNeedsUpdate = false;
  }
  if (velNeedsUpdate){
    stepper.setSpeed(targetVel);
    velNeedsUpdate = false;
  }
  if (maxVelNeedsUpdate){
    stepper.setMaxSpeed(targetMaxVel);
    maxVelNeedsUpdate = false;
  }
  if (accNeedsUpdate){
    stepper.setAcceleration(targetAcc);
    accNeedsUpdate = false;
  }
}

void requestEvent()
{  
    TinyWireS.send(mode);
}

void goHome(){
  
  int homeSwitch = digitalRead(homeSwitchPin);
  
  if (homeSwitch == HIGH){  //motor is not home
    stepper.runSpeed();
  } else {  //home!
    mode = 0;  //paused
    homed = true;
    stepper.setCurrentPosition(0);
  }
}

void recieveI2C(uint8_t numBytes){
  
  //first byte is instruction
  //1 means position is set - go required
  //2 means velocity is set - go required
  //3 means max velocity is set - go required
  //4 means acceleration is set - go required
  //5 means change microstepping - go NOT required
  //6 means home the motor, velocity is set - go NOT required
  //7 means set home offset - go required
  //10 means GO!
  
  //payload values are all 4 bytes, high byte first, to be assembled into a long
 
  //cache message in array
  //maximum length 8 bytes - anything else is thrown away
  for (int i=0; i<numBytes; i++){
    if (i < 8){
      message[i] = TinyWireS.receive();
    } else {
      TinyWireS.receive();
    }
  }
  
  byte instruction = message[0];
  
  switch (instruction){

    case 1:  //payload: 1 long (4bytes), position    
    {
      nextMode = 1;  //normal
      
      targetPos  = (long)message[1] << 24;
      targetPos += (long)message[2] << 16;
      targetPos += (long)message[3] << 8;
      targetPos += (long)message[4];
      
      posNeedsUpdate = true;
      
    }  break;

    case 2:  //payload: 1 long (4bytes), velocity
    {
      nextMode = 2;  //constant velocity

      targetVel  = (long)message[1] << 24;
      targetVel += (long)message[2] << 16;
      targetVel += (long)message[3] << 8;
      targetVel += (long)message[4];

      velNeedsUpdate = true;
      
    }  break;
    
    case 3:  //payload: 1 long (4bytes), max velocity
    {
      nextMode = 1;  //normal
    
      targetMaxVel  = (long)message[1] << 24;
      targetMaxVel += (long)message[2] << 16;
      targetMaxVel += (long)message[3] << 8;
      targetMaxVel += (long)message[4];

      maxVelNeedsUpdate = true;
      
    }  break;
    
    case 4:  //payload: 1 long (4bytes), acceleration
    {
      nextMode = 1;  //normal
    
      targetAcc  = (long)message[1] << 24;
      targetAcc += (long)message[2] << 16;
      targetAcc += (long)message[3] << 8;
      targetAcc += (long)message[4];

      accNeedsUpdate = true;
      
    }  break;
    
    case 5:  //payload 1 byte, microStepping
    {
      setMicroStepping((int)message[1]);
      
    }  break;
    
    case 6:  //payload: 1 int (2 bytes) velocity
    {
      mode = 3;  //homing
      homed = false;
      
      byte velH = message[1];
      byte velL = message[2];
      word vel = word(velH, velL);
      
      stepper.setSpeed((int)vel);
      
    }  break;
    
    case 7:  //payload: 1 long (4bytes) offset position
    {
      nextMode = 4;  //offset
      homed = false;
      
      targetPos  = (long)message[1] << 24;
      targetPos += (long)message[2] << 16;
      targetPos += (long)message[3] << 8;
      targetPos += (long)message[4];

      posNeedsUpdate = true;
      
    }  break;
    
    case 10:  //Go! payload: none
    {
      mode = 5;     
    }  break;
    
  }
  	
}

void setMicroStepping(int target){
  
  microStepping = target;
  
  //data from pololu drv8825 website
  
  switch (microStepping) {
    case 1:
      digitalWrite(m0Pin, LOW);
      digitalWrite(m1Pin, LOW);
      digitalWrite(m2Pin, LOW);
      break;
    case 2:
      digitalWrite(m0Pin, HIGH);
      digitalWrite(m1Pin, LOW);
      digitalWrite(m2Pin, LOW);
      break;
    case 4:
      digitalWrite(m0Pin, LOW);
      digitalWrite(m1Pin, HIGH);
      digitalWrite(m2Pin, LOW);
      break;
    case 8:
      digitalWrite(m0Pin, HIGH);
      digitalWrite(m1Pin, HIGH);
      digitalWrite(m2Pin, LOW);
      break;
    case 16:
      digitalWrite(m0Pin, LOW);
      digitalWrite(m1Pin, LOW);
      digitalWrite(m2Pin, HIGH);
      break;
    case 32:
      digitalWrite(m0Pin, HIGH);
      digitalWrite(m1Pin, LOW);
      digitalWrite(m2Pin, HIGH);
      break;
    default: 
      break;
  }
  
}
