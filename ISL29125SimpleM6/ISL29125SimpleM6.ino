/******************************************************************************
ISL29125_interrupts.ino
Example demonstrating use of the ISL29125 RGB sensor library with interrupts.
Jordan McConnell @ SparkFun Electronics
18 Apr 2014
https://github.com/sparkfun/SparkFun_ISL29125_Breakout_Arduino_Library

This example shows how to use the ISL29125 sensor using interrupts. It
demonstrates how to make a more advanced configuration to set this up that goes
beyond the basics needed simply to acquire readings from the sensor. Interrupts
are triggered when sensor readings are above or below set thresholds and this
example also shows how to set those up. It also teaches how to read the sensor
after the interrupt and how to read/clear status flags so another interrupt can
be triggered and so you can interpret the status of the sensor. If you plan on
using this sensor in an interrupt driven project, this is the example for you.

Developed/Tested with:
Arduino Uno
Arduino IDE 1.0.5

Requires:
SparkFun_ISL29125_Arduino_Library

This code is beerware.
Distributed as-is; no warranty is given. 
******************************************************************************/

#include <Wire.h>
#include <SparkFunISL29125.h>
#include <CountUpDownTimer.h>
#include <SuperChrono.h>
CountUpDownTimer T(UP);

// Declare sensor object
SFE_ISL29125 RGB_sensor;

// The red score
unsigned int redScore = 0;

// The green score
unsigned int greenScore=0;

// the winning score
unsigned int winScore=5;
 
//the pin where the button is connected
const int BUTTON=7;

//value used to store the state of the button pin
int buttonStatus=HIGH;

//variable to store the old button pin state
int old_buttonStatus=HIGH;

//varialble to store state of button
int state=0;

//variable to store pause time
unsigned long pTime=0.0;

//variable to store max play time
unsigned long maxTime=60;

long unsigned int red_value = 0; // Stores sensor reading for red light intensity
long unsigned int redMax=0x13E5; //threshold for recording a read hit
long unsigned int green_value=0; // Stores sensor reading for green light intensity
long unsigned int greenMax=0x5200; //threshold for recording a green hit

//function to show time
void elapseTime(){
   Serial.print("Time: ");
   Serial.print(T.ShowMinutes());
   Serial.print(":");
   Serial.println(T.ShowSeconds());
}

//function to show score
void score() {
   Serial.print("Red Score: ");
   Serial.println(redScore);
   Serial.print("Green Score: ");
   Serial.println(greenScore);
}

//function to exectute when game is over
void gameOver(){
   Serial.println("GAME OVER");
   score();
   elapseTime();
   Serial.println("HIT BUTTON TO START NEW GAME"); 
   }

// function to restart the game
void newGame(){
          buttonDebounce();
          redScore=0;
          greenScore=0;
          T.ResetTimer();
          return;
          }
          
//function to check score and elapsed time

void checkScore(){
  if((greenScore==winScore)||(redScore==winScore)){ //check score
      gameOver();
      newGame();
      }
    else {
        continueGame();
         } 
}

void checkTime(){
  if(T.ShowTotalSeconds()>=maxTime){ //check game elapsed time
      gameOver();
      newGame();
      }
 }

// function to detect the button push and avoid bouncing
void buttonDebounce(){
    while (state==0) {
          //Serial.println("button not pushed");
        buttonStatus=digitalRead(BUTTON); // read input value and store it 
        if((buttonStatus==LOW)&& (old_buttonStatus==HIGH)){ // check if button has been pushed (LOW)  or not HIGH
            state=1-state; //
            delay(50); //delay for 50 milliseconds
          }
        old_buttonStatus=buttonStatus;                     
          }
        state=0; // Change the state back to zero for next button push
          }
  
  // function to continue game after button is hit
  void continueGame(){
          score();
          elapseTime();
          Serial.println("HIT BUTTON TO CONTINUE GAME"); 
          buttonDebounce();
          T.ResumeTimer();
          delay(250);
          }

// Set up serial communication, initialize the sensor, and set up interrupts
void setup()
{
  // set the time
 T.SetTimer(0,0,0); //set the run start time
 
 T.StartTimer(); //start the timer

  // Initialize serial communication
  Serial.begin(115200);

  //tell Arduino that BUTTON is an input
  pinMode(BUTTON,INPUT_PULLUP);  // internal pull-up

  // Initialize the ISL29125 and verify its presence
  if (RGB_sensor.init())
  {
    Serial.println("Sensor Initialization Successful\n\r");
  }
  
  // Advanced configuration: Interrupts based solely on red light intensity. ~100ms per sensor reading.
  // Config1: CFG1_MODE_RG - only read red and green
  //          CFG1_10KLUX - 10K Lux is full light scale
  // Config2: CFG2_IR_ADJUST_HIGH - common IR filter setting as a starting point, see datasheet if you desire to calibrate it to your exact lighting situation
  // Config3: CFG3_R_INT - trigger interrupts based on sensor readings for red light intensity
  //          CFG3_INT_PRST8 - only trigger interrupt if red sensor reading crosses threshold 8 consecutive times
  // For other configuration options, look at the SFE_ISL29125.h file in the SFE_ISL29125_Library folder
  // RGB_sensor.config(CFG1_MODE_RG | CFG1_10KLUX, CFG2_IR_ADJUST_HIGH, CFG3_NO_INT | CFG3_INT_PRST8);
 }
 

void loop()
{ 
  T.Timer(); // run the timer
  elapseTime();
  // Read the detected light intensity of the red and green visible spectrum
    red_value = RGB_sensor.readRed();
    green_value = RGB_sensor.readGreen();
  
    if ((red_value>redMax)&&!(green_value>greenMax)) //check if red threshold is exceeded and green threshold is not
    {
        T.PauseTimer(); // pause timer if red or green threshold is exceeded
        redScore++; // increment hit counter for red
        checkScore(); //check score and time
           }
    else if ((green_value>greenMax)&&!(red_value>redMax)) //check if green threshold is exceeded and red threshold is not
    {
        T.PauseTimer(); // pause timer if red or green threshold is exceeded
        greenScore++; // increment hit counter for green
        checkScore(); //check score and time
          }
    else if ((green_value>greenMax)&&(red_value>redMax)) //check if both thresholds are exceeded
    {
        T.PauseTimer(); // pause timer if red or green threshold is exceeded
        redScore++; // increment hit counter for red
        greenScore++; // increment hit counter for red
        checkScore(); //check score and time
           }
    checkTime();
       }


