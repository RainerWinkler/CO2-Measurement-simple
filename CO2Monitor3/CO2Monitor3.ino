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
char character;

// Configure EEPROM usage for log
// Start of log value (This has to be an even number starting with
// 0. In case of 0 no free EEPROM registers are available
int logFirstRegister = 0;
unsigned long logIntervallMS = 120000;
unsigned long  logNextMS = 0;
short position2 = 1;
short maxPositions = 0;


// Configure additional load to prevent that a powerbank goes into standby mode

unsigned long loadSwitchNextToogleMS = 0;
bool loadActive = false;

unsigned long loadSwitchOffTimeMS = 4500;
unsigned long loadSwitchOnTimeMS = 1000;

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

  // Checks during start
  digitalWrite(pinPositiveLED, HIGH);
  digitalWrite(pinWarningLED, HIGH);
  digitalWrite(pinErrorLED, HIGH);
  
  // Welcome messages
  lcd.setCursor(0, 0);
  lcd.print("Welcome"); // 0
  lcd.setCursor(0, 1);
  lcd.print("CO2 Monitor 3 D"); // 0
  delay(5000);

  // Start serial connection to sensor
  Serial.begin(115200);
  sensorConnection.begin(9600);  

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
    position2 = position2 + 1;
  }
  
  maxPositions = ((EEPROM.length()-logFirstRegister)/2);
  
  // Make a mark that the device is started
  writeLog(49999);
  
  Serial.print("maxPositions");
  Serial.println(maxPositions);
  Serial.print("Logging Intervall [ms]: ");
  Serial.println(logIntervallMS);
  Serial.print("Logging started at ");
  Serial.println(position2);

  Serial.println("Processing started");
  
}

void loop() {

//  Serial.println("Loop is started");

  // Process commands

//  if (Serial.available()){
//    command = Serial.readString();      
//     
//  }

    while(Serial.available()) {
      character = Serial.read();
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

  lcd.setCursor(0, 1);
  lcd.print("CO2 "); 
  lcd.print(CO2);
  lcd.print(" ppm       ");  
  lcd.setCursor(13,1);
  lcd.print(maxPositions-position2);
  
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
  else {             lcd.print("****************");}; 

 

  // Log when required
  if (millis()> logNextMS){
      writeLog(CO2);
      logNextMS = logNextMS + logIntervallMS;    
  }
//    int nextAdress = logFirstRegister+position2*2;
//    if (nextAdress+2>EEPROM.length()){
//      Serial.println("Log is full");
//    }
//    else {
//      int thisAdress = nextAdress-2;
//      unsigned int co2value = CO2;
//      unsigned int stopValue = 50000;
//      EEPROM.put(thisAdress,co2value);
//      EEPROM.put(nextAdress,stopValue);
//      Serial.print("Logged at position ");
//      Serial.print(position2);
//      Serial.print("\t");
//      Serial.println(CO2);
//      position2 = position2 + 1;
//        
//    };
        
  
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
  Serial.print("Command received: ");
  Serial.println(command); 

  if (command == "c\n"){
    Serial.println("Logged data will be cleared");
    EEPROM.put(logFirstRegister,50000);
    // Restart logging
    position2 = 1;
    logNextMS = millis();
  }

  else if (command == "l\n"){
    // Display EEPROM content
    short position = 1;
    for (int index = logFirstRegister ; index < EEPROM.length()-1;) {     
      unsigned int value;
      EEPROM.get(index,value);
      if (value>=50000){
        break;
      }
      Serial.print(position);
      Serial.print("\t");
      Serial.println(value);

      index = index + 2;
      position = position + 1;
  }
  }
  else if (command == "h\n"){
    Serial.println("Help"); 
    Serial.println("Send h to get help");
    Serial.println("Send c to clear all logged data"); 
    Serial.println("Send l to display all logged data"); 
  };
}
    
void writeLog(unsigned int value ){
    // Log when required
//  if (millis()> logNextMS ){
    int nextAdress = logFirstRegister+position2*2;
    if (nextAdress+2>EEPROM.length()){
      Serial.println("Log is full");
    }
    else {
      int thisAdress = nextAdress-2;
      unsigned int co2value = value;
      unsigned int stopValue = 50000;
      EEPROM.put(thisAdress,co2value);
      EEPROM.put(nextAdress,stopValue);
      Serial.print("Logged at position ");
      Serial.print(position2);
      Serial.print("\t");
      Serial.println(value);
      position2 = position2 + 1;
        
    };
//    logNextMS = logNextMS + logIntervallMS;            
//  }
};
