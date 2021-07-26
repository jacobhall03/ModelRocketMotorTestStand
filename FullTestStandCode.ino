#include <HX711_ADC.h>
#include <SD.h>

//pins:
const int HX711_dout = 8; //mcu > HX711 dout pin
const int HX711_sck = 9; //mcu > HX711 sck pin
const int BUZZER = 7; //onboard buzzer pinout
const int LED = 6; //onboard led pinout
const int SWITCH = 5; //onboard switch pinout
const int SD_CS = 10; //SD Card chip select pin

//constructors:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
File myFile; 

//variables:
float calibrationValue = 215.50; // paste the calibration value from Examples/HX711_ADC/Calibration
unsigned long t = 0; //for HX711
unsigned long starttime; //for 30 second timer
unsigned long currenttime; //for 30 second timer
#define NOTE 1047 //for buzzer tone



void setup() {
  //declaring pinModes
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);
  //pinMode(SD_CS, OUTPUT);

  //initializing load cell
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  #if defined(ESP8266)|| defined(ESP32)
  #endif
  unsigned long stabilizingtime = 2000; // precision right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    LoadCell.setSamplesInUse(1);
    Serial.println("Startup is complete");
  }
  LoadCell.setSamplesInUse(1);

  //initializing Micro SD Card
  Serial.print("Initializing SD card..."); 
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
  }
  Serial.println("initialization done.");
  myFile = SD.open("test.txt", FILE_WRITE);
  //if the file is opened, write to it
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("time(s),mass(g),force(N)");
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
    for(int i=0; i<3; i++){
      tone(BUZZER, NOTE, 200);
      digitalWrite(LED, HIGH);
      delay(200);
      digitalWrite(LED, LOW);
      delay(200);
      tone(BUZZER, NOTE, 200);
      digitalWrite(LED, HIGH);
      delay(200);
      digitalWrite(LED, LOW);
      delay(200);
    }
    return;
  }

  //user countdown sequence
  Serial.println("A buzzer/led will turn on/off every second while the Arduino is counting down.");
  Serial.println("The buzzer will remain on for 10 seconds after countdown, during this time you should ignite the motor.");
  Serial.println("The data is being logged to the sd card and the serial monitor while the led is on.");
  Serial.println("Press the onboard switch to start the countdown and manually fire the igniter when the led and buzzer turns on continously.");
  while(digitalRead(SWITCH))
  {};
  for(int i=0;i<10;i++){
    Serial.println(10-i);
    tone(BUZZER, NOTE, 250);
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(750);
  };

  //initializing data capture sequence
  Serial.println("FIRE WITHIN 10 SECONDS FOR BEST RESULTS!");
  tone(BUZZER, NOTE, 10000); //comment this to keep buzzer from toning for 10 seconds
  digitalWrite(LED, HIGH);
  starttime = millis();

  //running data capture sequence
  while (currenttime <= starttime + 30000){
    currenttime = millis();
    static boolean newDataReady = 0;
    const int serialPrintInterval = 0; //increase value to slow down serial print activity

    // check for new data/start next conversion:
    if (LoadCell.update()) newDataReady = true;

   // get smoothed value from the dataset:
    if (newDataReady) {
      if (millis() > t + serialPrintInterval) {
        float i = LoadCell.getData();
        Serial.print("Load_cell output val: ");
        Serial.println(i);
        myFile.print(t); myFile.print(","); myFile.print(i); myFile.print(","); myFile.println((i/1000)*9.807);
        newDataReady = 0;
        t = millis();
        }
    }
  }
  digitalWrite(LED, LOW);
  myFile.close();
}

void loop() {
  //no code here...
}
