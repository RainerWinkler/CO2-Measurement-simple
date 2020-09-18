// Uses an Arduino UNO R3
// CO2 Monitor 3
// See https://github.com/RainerWinkler/CO2-Measurement-simple
// Rainer Winkler, MIT License 2020

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

// Configure pins

// Pin 13 is used so that LED L will be active when the dummy load is drawn
int pinDummyLoad = 13;
int pinRxOfSensor = 7;
int pinTxOfSensor = 8;//6->8
int pinPositiveLED = 4;//8->4
int pinWarningLED = 3;//11->3
int pinErrorLED = 2;//9->2
int pinLCDrs = 5;//12->5
int pinLCDen = 6;//10->6
int pinLCDd4 = 9;//5->9
int pinLCDd5 = 10;//4->10
int pinLCDd6 = 11;//3->11
int pinLCDd7 = 12;//2->12

// Configure Serial connection

SoftwareSerial sensorConnection(pinTxOfSensor,pinRxOfSensor); //Tx of sensor is connected to Rx of the configured connection. Same for Rx of sensor

// Configure display

const int rs = pinLCDrs, en = pinLCDen, d4 = pinLCDd4, d5 = pinLCDd5, d6 = pinLCDd6, d7 = pinLCDd7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Commands via Serial Interface

String command = "";

// Configure EEPROM usage for log
// Start of log value (This has to be an even number starting with
// 0. In case of 0 no free EEPROM registers are available
int logIntervallRegister = 0;
int logFirstRegister = 2; 
unsigned long logIntervallMS = 0;
unsigned long  logNextMS = 0;
short logPosition = 1;
short maxPosition = 0;


// Configure additional load to prevent that a powerbank goes into standby mode

unsigned long loadSwitchNextToogleMS = 0;
bool loadActive = false;

unsigned int loadSwitchOffTimeMS = 4500;
unsigned int loadSwitchOnTimeMS = 1000;

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

  // Start serial connection to sensor
  Serial.begin(115200);
  sensorConnection.begin(9600);  

  // Checks during start
  digitalWrite(pinPositiveLED, HIGH);
  digitalWrite(pinWarningLED, HIGH);
  digitalWrite(pinErrorLED, HIGH);

//  getLogIntervall( );

  // Read stored logintervall
  unsigned int storedSec = 0;
  EEPROM.get(logIntervallRegister,storedSec);
  if (storedSec==0){
    storedSec = 300; // Logintervall zero makes no sense
  }
  else if (storedSec==0xFFFF){
    storedSec = 300; // This is assumed to be an EEPROM that was never written. So use 5 Minutes when Logging intervall was never specified.
  }

  logIntervallMS = storedSec * 1000;

  // Welcome messages
  lcd.setCursor(0, 0);
  lcd.print(F("Welcome         ")); 
  lcd.setCursor(0, 1);
  lcd.print(F("CO2 Monitor 3   ")); 
  delay(3000);
  for (int i = 1; i < 2; ++i){
  
    lcd.setCursor(0, 0);
    lcd.print(F("Connect with    ")); 
    lcd.setCursor(0, 1);
    lcd.print(F("115200 Baud     "));
    delay(2000);
  
    lcd.setCursor(0, 0);
    lcd.print(F("Send h for help ")); 
    lcd.setCursor(0, 1);
    lcd.print(F("                ")); 
    delay(2000);

    lcd.setCursor(0, 0);
    lcd.print(F("Logged all      "));
    lcd.setCursor(0, 1);
    lcd.print(storedSec); 
    lcd.print(F(" seconds        ")); 
    delay(2000);
  
    lcd.setCursor(0, 0);
  //lcd.print("1234567890123456"); 
    lcd.print(F("Used      -> 100")); 
    lcd.setCursor(0, 1);
    lcd.print(F("Free      -> 410")); 
    delay(2000);
  
    lcd.setCursor(0, 0);
  //lcd.print("1234567890123456"); 
    lcd.print(F("** <- Bar graph ")); 
    lcd.setCursor(0, 1);
    lcd.print(F("16 x * = 1000ppm")); 
    delay(2000);
  }

  // Prepare Load logic
  loadSwitchNextToogleMS = millis() + loadSwitchOffTimeMS;
  loadActive = false;
  

  // Prepare logging
  logNextMS = millis() + logIntervallMS;

  // Find where to proceed with logging

    for (int index2 = logFirstRegister ; index2 < EEPROM.length()-1;) {     
      unsigned int value;
      EEPROM.get(index2,value);
      if (value >=50000){
        break;
      };
    index2 = index2 + 2;
    logPosition = logPosition + 1;
  }
  
  maxPosition = ((EEPROM.length()-logFirstRegister)/2);
  
  // Make a mark that the device is started
  writeLog(49999);
  
  Serial.print(F("maxPosition "));
  Serial.println(maxPosition);
  Serial.print(F("Logging Intervall [ms]: "));
  Serial.println(logIntervallMS);
  Serial.print(F("Logging started at "));
  Serial.println(logPosition);

  Serial.println(F("Processing started"));
  
}

void loop() {

    while(Serial.available()) {
      char character = Serial.read();
      command.concat(character);
  }

  if (command != "") {
    processCommand();
  }

  command = "";

  // Toogle load
  if ( millis() > loadSwitchNextToogleMS)
    {
      if (loadActive)
        {
          loadSwitchNextToogleMS = millis()+loadSwitchOffTimeMS;  
          digitalWrite(pinDummyLoad,LOW);               
        }
      else
        {
          loadSwitchNextToogleMS = millis()+loadSwitchOnTimeMS;
          digitalWrite(pinDummyLoad,HIGH);
          
        };
      loadActive = ! loadActive;
    }

  // Check whether Load has to be switched on or off

  short CO2 = 0;

  getCO2Value(&CO2);

  // Bar with intensity
  lcd.setCursor(0, 0);
  if(CO2<438){       lcd.print(F("                "));} 
  else if(CO2<475){  lcd.print(F("*               "));} 
  else if(CO2<513){  lcd.print(F("**              "));} 
  else if(CO2<550){  lcd.print(F("***             "));} 
  else if(CO2<588){  lcd.print(F("****            "));} 
  else if(CO2<625){  lcd.print(F("*****           "));} 
  else if(CO2<663){  lcd.print(F("******          "));}
  else if(CO2<700){  lcd.print(F("*******         "));} 
  else if(CO2<738){  lcd.print(F("********        "));}
  else if(CO2<775){  lcd.print(F("*********       "));}
  else if(CO2<813){  lcd.print(F("**********      "));} 
  else if(CO2<850){  lcd.print(F("***********     "));} 
  else if(CO2<888){  lcd.print(F("************    "));} 
  else if(CO2<925){  lcd.print(F("*************   "));} 
  else if(CO2<963){  lcd.print(F("**************  "));} 
  else if(CO2<1000){ lcd.print(F("*************** "));}
  else {             lcd.print(F("****************"));}; 

  if (CO2<925){
    // Write log position as long as not too many stars are displayed
    lcd.setCursor(13,0);
    lcd.print(logPosition - 1);
  }

  lcd.setCursor(0, 1);
  lcd.print("CO2 "); 
  lcd.print(CO2);
  lcd.print(" ppm            ");  
  lcd.setCursor(13,1);
  lcd.print(maxPosition-logPosition);
  
  // Red light
  if ( CO2>=1000 ){ digitalWrite(pinErrorLED, HIGH);  }
  else { digitalWrite(pinErrorLED, LOW); };
  
  // Yellow light
  if ( CO2>=700 && CO2<1000 ){ digitalWrite(pinWarningLED, HIGH);    }
  else { digitalWrite(pinWarningLED, LOW); };  
  
  // Green light
  if ( CO2<700 ){ digitalWrite(pinPositiveLED, HIGH);    }
  else { digitalWrite(pinPositiveLED, LOW); };  


  // Log when required
  if (millis()> logNextMS){
      writeLog(CO2);
      logNextMS = logNextMS + logIntervallMS;    
  }
      
  
}

void getCO2Value(short *CO2){

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
  *CO2 = 256 * (short)returnValue[2] + returnValue[3];
}

void processCommand( ){
  Serial.print(F("Command received: "));
  Serial.println(command); 

  if (command == "c\n"){
    // Clear log
    Serial.println(F("Logged data will be cleared"));
    EEPROM.put(logFirstRegister,50000); // This marks the end of the log
    // Restart logging
    logPosition = 1;
    logNextMS = millis();
  }

  else if (command == "l\n"){
    // Output Log result
    short position = 1;
    for (int index = logFirstRegister ; index < EEPROM.length()-1;) {     
      unsigned int value;
      EEPROM.get(index,value);
      if (value>=50000){
        // This is a stop value, no further display needed
        break;
      }
      Serial.print(position);
      Serial.print("\t");
      Serial.println(value);

      index = index + 2;
      position = position + 1;
    }
  }
  else if (command == "sca\n"){
    // activate self calibration
    // Sending command for MH-Z19B according to data sheet
    byte commandSelfCalOn[9] = {0xFF,0x01,0x79,0xA0,0x00,0x00,0x00,0x00,0xE6}; // on
  
    sensorConnection.write(commandSelfCalOn, 9);
    Serial.println(F("Self calibration is on"));  
  }
  else if (command == "sco\n"){
    // deactivate self calibration
    // Sending command for MH-Z19B according to data sheet
    byte commandSelfCalOff[9] = {0xFF,0x01,0x79,0x00,0x00,0x00,0x00,0x00,0x86}; // off
  
    sensorConnection.write(commandSelfCalOff, 9);
    Serial.println(F("Self calibration is off"));  
    
  }
  else if (command == "cal\n"){
    // Calibrate sensor
    // Sending command for MH-Z19B according to data sheet
    byte commandCalibrate[9] = {0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78}; // calibrate sensor
  
    sensorConnection.write(commandCalibrate, 9);
    Serial.println(F("Calibration to 400 ppm is done"));  
    
  }
  else if (command.startsWith("li")){
    command.remove(0, 2);
    unsigned long inputSec = 0;
    inputSec = command.toInt( );
    if (inputSec >= 0xFFFF){
      Serial.println("65534 sec is maxium. Entered value is ignored");
    }
    else if (inputSec == 0){
      Serial.println("0 seconds is not allowed. Entered value is ignored");
    }
    else {
      logIntervallMS = inputSec * 1000;
      EEPROM.put(logIntervallRegister,inputSec);
      Serial.print(F("New log interval is "));
      Serial.print(inputSec);
      Serial.println(F(" seconds"));
      Serial.println(F("Clear the log to make sure that always the same intervall is used."));
      // Now change expected time for logging
      logNextMS = millis() + logIntervallMS;
    }
  }
  else if (command == "h\n"){
    Serial.println(F("Help")); 
    Serial.println(F("Send h to get help"));
    Serial.println(F("li followed by seconds to set log intervall. Example: li300 to log all 300 seconds (Lowercase LI).")); 
    Serial.println(F("Send c to clear all logged data")); 
    Serial.println(F("Send l to display all logged data (Lowercase L)")); 
    Serial.println(F("Send sca to activate self calibration (Default)")); 
    Serial.println(F("Send sco to deactivate self calibration")); 
    Serial.println(F("Send cal to calibrate (CO2 has to be 400 ppm, wait 20 Minutes in this environment)")); 
  };
}

//void getLogIntervall( ){
//  // Read stored logintervall
//  unsigned int storedSec = 0;
//  EEPROM.get(logIntervallRegister,storedSec);
//  if (storedSec==0){
//    storedSec = 300; // Logintervall zero makes no sense
//  }
//  else if (storedSec==0xFFFF){
//    storedSec = 300; // This is assumed to be an EEPROM that was never written. So use 5 Minutes when Logging intervall was never specified.
//  }
//  logIntervallMS = storedSec * 1000;
//  Serial.print(F("Logging will be done all "));
//  Serial.print(storedSec);
//  Serial.println(F(" seconds"));
//}
    
void writeLog(unsigned int value ){
    int nextAdress = logFirstRegister+logPosition*2;
    if (nextAdress+2>EEPROM.length()){
      Serial.println(F("Log is full"));
    }
    else {
      int thisAdress = nextAdress-2;
      unsigned int co2value = value;
      unsigned int stopValue = 50000;
      EEPROM.put(thisAdress,co2value);
      EEPROM.put(nextAdress,stopValue);
      Serial.print(F("Logged at position "));
      Serial.print(logPosition);
      Serial.print("\t");
      Serial.println(value);
      logPosition = logPosition + 1;
        
    };
};
