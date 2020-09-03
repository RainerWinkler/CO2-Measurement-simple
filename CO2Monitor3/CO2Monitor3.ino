// CO2 Monitor 3
// Uses an Arduino UNO R3
// See https://github.com/RainerWinkler/CO2-Measurement-simple
// Rainer Winkler, MIT License 2020

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// Configure pins

// Pin 13 is used so that LED L will be active when the dummy load is drawn
int pinDummyLoad = 13;
int pinRxOfSensor = 7;
int pinTxOfSensor = 6;
int pinPositiveLED = 8;
int pinWarningLED = 11;
int pinErrorLED = 9;
int pinLCDrs = 12;
int pinLCDen = 10;
int pinLCDd4 = 5;
int pinLCDd5 = 4;
int pinLCDd6 = 3;
int pinLCDd7 = 2;

// Configure Serial connection

SoftwareSerial sensorConnection(pinTxOfSensor,pinRxOfSensor); //Tx of sensor is connected to Rx of the configured connection. Same for Rx of sensor

// Configure display

const int rs = pinLCDrs, en = pinLCDen, d4 = pinLCDd4, d5 = pinLCDd5, d6 = pinLCDd6, d7 = pinLCDd7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Configure additional load to prevent that a powerbank goes into standby mode

long loadSwitchedOffMS = 0;
long loadSwitchOffTimeMS = 4500;
long loadSwitchOnTimeMS = 1000;

void setup() {

  // Define output pins

  pinMode(pinErrorLED, OUTPUT);
  pinMode(pinWarningLED, OUTPUT);
  pinMode(pinPositiveLED, OUTPUT);
  pinMode(pinDummyLoad, OUTPUT); 

  // Draw load during setup to prevent that powerbank does not go into standby when setup takes too long
  digitalWrite(pinDummyLoad,HIGH);// Draw current to load powerbank
  
  // Configure LCD display
  lcd.begin(16, 2);

  // Test LED during start
  digitalWrite(pinPositiveLED, HIGH);
  digitalWrite(pinWarningLED, HIGH);
  digitalWrite(pinErrorLED, HIGH);
  
  // Welcome messages
  lcd.setCursor(0, 0);
  lcd.print("Welcome"); // 0
  lcd.setCursor(0, 1);
  lcd.print("CO2 Monitor 3"); // 0
  delay(5000);

  // Start serial connection to sensor
  Serial.begin(9600);
  sensorConnection.begin(9600);  
  
}

void loop() {

  // Check whether Load has to be switched on or off

  if ((millis() - loadSwitchedOffMS) > loadSwitchOffTimeMS) 
    {
      loadSwitchedOffMS = millis();
      digitalWrite(pinDummyLoad,HIGH);// Draw current to load powerbank
      delay(loadSwitchOnTimeMS);
      digitalWrite(pinDummyLoad,LOW);
    }  

  int CO2= 0;

  getCO2Value(&CO2);

  lcd.setCursor(0, 1);
  lcd.print("CO2 "); 
  lcd.print(CO2);
  lcd.print(" ppm       ");  
  
  // Red light
  if ( CO2>=1000 ){ digitalWrite(pinErrorLED, HIGH);  }
  else { digitalWrite(pinErrorLED, LOW); };
  
  // Yellow light
  if ( CO2>=700 && CO2<1000 ){ digitalWrite(pinWarningLED, HIGH);    }
  else { digitalWrite(pinWarningLED, LOW); };  
  
  // Green light
  if ( CO2<700 ){ digitalWrite(pinPositiveLED, HIGH);    }
  else { digitalWrite(pinPositiveLED, LOW); };  

  // Bar with intensity
  lcd.setCursor(0, 0);
  if(CO2<438){       lcd.print("                ");} 
  else if(CO2<475){  lcd.print("*               ");} 
  else if(CO2<513){  lcd.print("**              ");} 
  else if(CO2<550){  lcd.print("***             ");} 
  else if(CO2<588){  lcd.print("****            ");} 
  else if(CO2<625){  lcd.print("*****           ");} 
  else if(CO2<663){  lcd.print("******          ");}
  else if(CO2<700){  lcd.print("*******         ");} 
  else if(CO2<738){  lcd.print("********        ");}
  else if(CO2<775){  lcd.print("*********       ");}
  else if(CO2<813){  lcd.print("**********      ");} 
  else if(CO2<850){  lcd.print("***********     ");} 
  else if(CO2<888){  lcd.print("************    ");} 
  else if(CO2<925){  lcd.print("*************   ");} 
  else if(CO2<963){  lcd.print("**************  ");} 
  else if(CO2<1000){ lcd.print("*************** ");}
  else {             lcd.print("****************");} 
  
}

void getCO2Value(int *CO2){

  // Sending command for MH-Z19B according to data sheet
  byte commandReadCO2[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
  
  sensorConnection.write(commandReadCO2, 9);
  
  byte returnValue[9]; 
  memset(returnValue, 0, 9);
  while (sensorConnection.available() == 0) {
    delay(10);
  }
  sensorConnection.readBytes(returnValue, 9);
  
  // Evaluate Return value according to data sheet  
  *CO2 = 256 * (int)returnValue[2] + returnValue[3];
}
