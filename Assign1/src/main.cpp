/*
--------------------------------------------------------
Course: Embedded System and IoT  - a.y. 2024-2025
Project: Assignment #1 - Give Me the Binary! (GMB)

Author: Yiming Li, Tianyu Qu, Jing Yang
Date: October 20, 2024

--------------------------------------------------------
*/

#include <Arduino.h>
#include <Wire.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
#include <avr/sleep.h>
#include <PinChangeInterrupt.h>
// ---------------- Pin Definition --------------------
// Pin Definitions for LEDs
#define   L1   13
#define   L2   12
#define   L3   11
#define   L4   10
#define   LS   9  // Red LED
// Pin Definitions for Buttons
#define   B1   2
#define   B2   3
#define   B3   4
#define   B4   5
// Pin Definition for Potentiometer (Analog input)
#define   Pot   A2 

// ---------------- Parameter Definition --------------------
int fadeAmount = 15;
int currIntensity = 0;
// Define states
enum GameState { INIT, GAME, GAME_OVER };
GameState gameState = INIT;
unsigned long startTime = 0;  // For time tracking
unsigned long T1;         // Maximum time to respond
int F;                    // Difficulty factor
int score = 0;
int playerInput = 0;
int displayedNumber = 0;
const unsigned long DEBOUNCE_DELAY = 50; 
unsigned long lastButtonPressTime[4] = {0, 0, 0, 0}; 
unsigned long lastB1PressTime = 0;
volatile bool sleepMode = false;

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,20,4);

void Init()  {
  // Initialize LEDs as outputs
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);
  pinMode(LS, OUTPUT);
  
  // Initialize buttons as inputs (with internal pull-up resistors enabled)
  pinMode(B1, INPUT_PULLUP);
  pinMode(B2, INPUT_PULLUP);
  pinMode(B3, INPUT_PULLUP);
  pinMode(B4, INPUT_PULLUP);

  // Initialize potentiometer as an analog input
  //pinMode(Pot, INPUT);

  // Start the LCD and print a message
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to GMB!");
  lcd.setCursor(0, 1);
  lcd.print("Press B1 to Start");

  lastB1PressTime = millis();
}

void setup() {
  Serial.begin(9600);
  Init();  
  delay(100); 
  Serial.println("Setup completed"); 

}

void loop() {
   switch (gameState) {
    case INIT:
      handleInitialState();
      break;
    case GAME:
      handleGameState();
      break;
    case GAME_OVER:
      handleGameOverState();
      break;
  }
}
void WakeUp(){
}
void LS_PULSE() {
  analogWrite(LS, currIntensity); 
  currIntensity = currIntensity + fadeAmount;
  if (currIntensity == 0 || currIntensity == 255) {
   fadeAmount = -fadeAmount ; 
  } 
  delay(20);
}

//---------------Initial State----------------------
void handleInitialState() {
  lcd.setCursor(0, 0);
  lcd.print("Welcome to GMB!");
  lcd.setCursor(0, 1);
  lcd.print("Press B1 to Start");

  LS_PULSE();  
  unsigned long currentTime = millis();
  if (digitalRead(B1) == LOW) {
    if (currentTime - lastB1PressTime > DEBOUNCE_DELAY) {
      lastB1PressTime = currentTime; 
      startGame(); 
    }
  }
  if (millis() - lastB1PressTime > 10000) {
    goToSleep();
    delay(1000);
    gameState = INIT;
    lastB1PressTime = millis();
  }
}
//Deep sleep mode
void goToSleep(){
  Serial.println("Going to sleep");
  attachInterrupt(digitalPinToInterrupt(B1),WakeUp,RISING);
  attachInterrupt(digitalPinToInterrupt(B2),WakeUp,RISING);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(B3), WakeUp, RISING);
  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(B4), WakeUp, RISING);
  digitalWrite(LS, LOW);
  lcd.clear();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  sleep_enable();
  sleep_mode();

  //sleeping

  //wake up
  Serial.println("WAKE UP");
  sleep_disable(); 
}

// Start the game
void startGame() {
  gameState = GAME;
  startTime = millis();
  score = 0;
  T1 = 10000;  // Starting max time (10 seconds)
  setDifficulty();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Go!");
  lcd.setCursor(0, 1);
  lcd.print("Score: " + String(score));
  delay(1000);
  newRound();
}
// Set difficulty based on Potentiometer
void setDifficulty() {
  int potValue = analogRead(Pot);
  Serial.println(potValue);
  int level = map(potValue, 0, 1023, 1, 4);
  F = level;  // Example difficulty factor
  Serial.println(F);
}
//--------------------------Game State------------------------
void handleGameState() {
  if ((millis() - startTime > T1)&&checkCorrectNumber()) {
    score++;
    lcd.clear();
    lcd.print("GOOD! Score: ");
    lcd.print(score);
    if(T1>1000)
    T1 -= F * 1000;  // Reduce time by difficulty factor
    delay(1000);
    newRound();
  }
  if (millis() - startTime > T1) {
    handleGameOverState();
    return;
  }
  // Button press logic and binary number composition
  if (checkCorrectNumber()&&displayedNumber!=0) {
    score++;
    lcd.clear();
    lcd.print("GOOD! Score: ");
    lcd.print(score);
    if(T1>1000)
    T1 -= F * 1000;  // Reduce time by difficulty factor
    delay(1000);
    newRound();
  }
}
//Start a new round, Generate a new random number and display it
void newRound() {
  displayedNumber = random(0, 15);
  lcd.clear();
  lcd.print("Number: ");
  lcd.print(displayedNumber);
  playerInput = 0;
  turnOffLEDs();
  startTime = millis();  // Reset timer
}
//turn off all 4 LEDs
void turnOffLEDs() {
  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  digitalWrite(L3, LOW);
  digitalWrite(L4, LOW);
}
// Check if player has composed the correct binary number using LEDs
bool checkCorrectNumber() {
  int playerNumber = getPlayerBinaryInput();  // Convert LED states to a number
  return playerNumber == displayedNumber;
}

//-------------------Game Over State--------------------------------
void handleGameOverState() {
  gameState = GAME_OVER;
  turnOffLEDs();
  lcd.clear();
  lcd.print("Game Over");
  lcd.setCursor(0, 1);
  lcd.print("Final Score: ");
  lcd.print(score);
  digitalWrite(LS, HIGH);  // Turn on red LED for 1 second
  delay(1000);
  digitalWrite(LS, LOW);
  delay(10000);  // Show final score for 10 seconds
  resetGame();
}
// Reset game back to the initial state
void resetGame() {
  gameState = INIT;
  lcd.clear();
  LS_PULSE();
  lastB1PressTime = millis();
}
// Get player input
int getPlayerBinaryInput() {
  unsigned long currentTime = millis(); 
  if (digitalRead(B1) == LOW) {
    if (currentTime - lastButtonPressTime[0] > DEBOUNCE_DELAY) {
      lastButtonPressTime[0] = currentTime; 
      if (digitalRead(L1) == HIGH) { 
        playerInput -= 8; 
        digitalWrite(L1, LOW);
      } else { 
        playerInput += 8; 
        digitalWrite(L1, HIGH);
      }
    }
  }

  if (digitalRead(B2) == LOW) {
    if (currentTime - lastButtonPressTime[1] > DEBOUNCE_DELAY) {
      lastButtonPressTime[1] = currentTime; 
      if (digitalRead(L2) == HIGH) {
        playerInput -= 4; 
        digitalWrite(L2, LOW);
      } else {
        playerInput += 4; 
        digitalWrite(L2, HIGH);
      }
    }
  }

  if (digitalRead(B3) == LOW) {
    if (currentTime - lastButtonPressTime[2] > DEBOUNCE_DELAY) {
      lastButtonPressTime[2] = currentTime; 
      if (digitalRead(L3) == HIGH) {
        playerInput -= 2; 
        digitalWrite(L3, LOW);
      } else {
        playerInput += 2; 
        digitalWrite(L3, HIGH);
      }
    }
  }

  if (digitalRead(B4) == LOW) {
    if (currentTime - lastButtonPressTime[3] > DEBOUNCE_DELAY) {
      lastButtonPressTime[3] = currentTime; 
      if (digitalRead(L4) == HIGH) {
        playerInput -= 1; 
        digitalWrite(L4, LOW);
      } else {
        playerInput += 1;
        digitalWrite(L4, HIGH);
      }
    }
  }
  return playerInput;
}
