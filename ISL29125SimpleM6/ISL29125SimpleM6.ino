//include the following libraries
#include <Wire.h>
#include <SparkFunISL29125.h>
#include <CountUpDownTimer.h>
#include <Bounce2.h>
//define what i/o pin is associated with which button
const int PlayPauseButton = 4;
const int MinusRedScoreButton = 5;
const int MinusGreenScoreButton = 6;
const int MinusTimeButton = 7;

CountUpDownTimer T(UP);

// Declare sensor object
SFE_ISL29125 RGB_sensor;

//initiate bounce object for each button
Bounce PlayPause = Bounce();
Bounce MinusRedScore = Bounce();
Bounce MinusGreenScore = Bounce();
Bounce MinusTime = Bounce();

// The red score
unsigned int redScore = 0;

boolean playPauseStatus = true;
boolean gameStatus = false;

// The green score
unsigned int greenScore = 0;

// the winning score
const unsigned int WINSCORE = 5;

// integer used to store hit status
int hitStatus = 0;

//value used to store the state of the button pin
int buttonStatus = HIGH;

//variable to store max play time
const unsigned MAXTIME = 60;

//variable to store time of previous time check
unsigned lastTime = 0;

long unsigned int redValue = 0; // Stores sensor reading for red light intensity
long unsigned int REDTRIGGER = 0x1000; //threshold for recording a red hit
long unsigned int greenValue = 0; // Stores sensor reading for green light intensity
long unsigned int GREENTRIGGER = 0x3000; //threshold for recording a green hit

//function to show time
void elapseTime() {
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
void gameOver() {
  Serial.println("GAME OVER");
  score(); //print score
  elapseTime(); //print time
  redScore = 0;
  greenScore = 0;
  T.ResetTimer();
  Serial.println("HIT BUTTON TO START NEW GAME");
  buttonControl();
  T.ResumeTimer();
}

// function to restart the game
void newGame() {
  redScore = 0;
  greenScore = 0;
  T.ResetTimer();
  buttonControl();
  T.ResumeTimer();
}

//function to check score and elapsed time
boolean checkScore() {
  if ((greenScore == WINSCORE) || (redScore == WINSCORE)) {
    return (true);
  }
  else {
    return (false);
  }
}

boolean checkTime() {
  if (T.ShowTotalSeconds() >= MAXTIME) {
    return (true);
  }
  else {
    return (false);
  }
}

// function to detect the button push and avoid bouncing
void buttonControl() {
  while (playPauseStatus == false) {
    PlayPause.update();
    playPauseStatus = PlayPause.fell();
  }
  playPauseStatus = false;
}


// function to continue game after button is hit
void continueGame() {
  score(); //print score
  elapseTime(); //print time
  Serial.println("HIT BUTTON TO CONTINUE GAME");
  buttonControl();
  T.ResumeTimer();
}

// Set up serial communication, initialize the sensor, and set up interrupts
void setup() {
  // set the time
  T.SetTimer(0, 0, 0); //set the run start time

  T.StartTimer(); //start the timer

  // Initialize serial communication
  Serial.begin(115200);

  //set up bottons with internal pull-up
  pinMode(PlayPauseButton, INPUT_PULLUP);
  pinMode(MinusRedScoreButton, INPUT_PULLUP);
  pinMode(MinusGreenScoreButton, INPUT_PULLUP);
  pinMode(MinusTimeButton, INPUT_PULLUP);

  //set up bounce instance for each botton
  PlayPause.attach(PlayPauseButton);
  PlayPause.interval(10); //interval in ms
  MinusRedScore.attach(MinusRedScoreButton);
  MinusRedScore.interval(5); //interval in ms
  MinusGreenScore.attach(MinusGreenScoreButton);
  MinusGreenScore.interval(5); //interval in ms
  MinusTime.attach(MinusTimeButton);
  MinusTime.interval(5); //interval in ms

  //Initialize the ISL29125 and verify its presence
  if (RGB_sensor.init()) {
    Serial.println("Sensor Initialization Successful\n\r");
  }
  if (!(RGB_sensor.init())) {
    Serial.println("Sensor Initialization Not Successful\n\r");
  }
  // Advanced configuration: Interrupts based solely on red light intensity. ~100ms per sensor reading.
  // Config1: CFG1_MODE_RG - only read red and green
  //          CFG1_10KLUX - 10K Lux is full light scale
  // Config2: CFG2_IR_ADJUST_HIGH - common IR filter setting as a starting point, see datasheet if you desire to calibrate it to your exact lighting situation
  // Config3: CFG3_R_INT - trigger interrupts based on sensor readings for red light intensity
  //          CFG3_INT_PRST8 - only trigger interrupt if red sensor reading crosses threshold 8 consecutive times
  // For other configuration options, look at the SFE_ISL29125.h file in the SFE_ISL29125_Library folder
  //RGB_sensor.config(CFG1_MODE_RG | CFG1_10KLUX | CFG1_12BIT | CFG1_ADC_SYNC_TO_INT, CFG2_IR_OFFSET_ON | CFG2_IR_ADJUST_HIGH, CFG3_NO_INT);
}

void loop()
{
  T.Timer(); // run the timer
  if (T.TimeHasChanged() ) { // this prevents the time from being constantly shown.
    elapseTime();
  }
  // Read the detected light intensity of the red and green visible spectrum
  redValue = RGB_sensor.readRed();
  greenValue = RGB_sensor.readGreen();
  hitStatus = (greenValue > GREENTRIGGER) * 2 + (redValue > REDTRIGGER);
  switch (hitStatus) {
    case 1: // only red scores a hit
      redScore++; // increment hit counter for red
      break;
    case 2: // only green scores a hit
      greenScore++; // increment hit counter for green
      break;
    case 3:  // both grean and red score hits
      redScore++; // increment hit counter for red
      greenScore++; // increment hit counter for red
      break;
  }
  if (hitStatus != 0 && not((greenScore == WINSCORE) || (redScore == WINSCORE))) {
    T.PauseTimer(); // pause timer
    score(); //print score
    elapseTime(); //print time
    hitStatus = 0;
    Serial.println("HIT BUTTON TO CONTINUE GAME");
    playPauseStatus = false;
    buttonControl();
    T.ResumeTimer();
  }
  if ((T.ShowTotalSeconds() >= MAXTIME) || ((greenScore == WINSCORE) || (redScore == WINSCORE))) {
    T.PauseTimer(); // pause timer
    Serial.println("GAME OVER");
    score(); //print score
    elapseTime(); //print time
    redScore = 0;
    greenScore = 0;
    hitStatus = 0;
    T.ResetTimer();
    Serial.println("HIT BUTTON TO START NEW GAME");
    playPauseStatus = false;
    buttonControl();
    T.ResumeTimer();
  }
}




