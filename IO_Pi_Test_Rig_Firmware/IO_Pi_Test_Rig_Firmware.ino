/*
*  Firmware Version 1.0
*  Test Rig for Servo Pi V1.0
*  Copyright Apexweb Ltd 2014.
*/

#include <Wire.h>
#include <EEPROM.h>

// Define registers values from datasheet
const byte address1 = 0x20;
const byte address2 = 0x21;

const byte IODIRA = 0x00;
const byte IODIRB = 0x01;
const byte GPIOA = 0x14;
const byte GPIOB = 0x15;

// Define Pin addresses
const int ERROR_PIN = 9;
const int Test_PIN = 8;

// variables
char testrunning = 0;
char connectionstatus = 0;

// main code
void setup()
{

  pinMode(Test_PIN, INPUT); // pin for push button
  pinMode(ERROR_PIN, OUTPUT); // pin for error led
  digitalWrite(ERROR_PIN, 0);

  Wire.begin();           // Wire must be started!


  initIO();  
  test();
  test();
}

void loop(){
    if (digitalRead(Test_PIN) == HIGH) {
        while (digitalRead(Test_PIN) == HIGH){
            delay(10);
        }
        testrunning = 1;
        test();
    }
    else{
       testrunning = 0; 
    }
}

void test(){
    writeIO(address2, GPIOA, 0x00);
    writeIO(address2, GPIOB, 0x00);
    writeIO(address1, GPIOA, 0xAA);
    writeIO(address1, GPIOB, 0x55);
    delay(500);
    writeIO(address1, GPIOA, 0x55);
    writeIO(address1, GPIOB, 0xAA);
    delay(500);
    writeIO(address1, GPIOA, 0x00);
    writeIO(address1, GPIOB, 0x00);
    writeIO(address2, GPIOA, 0xAA);
    writeIO(address2, GPIOB, 0x55);
    delay(500);
    writeIO(address2, GPIOA, 0x55);
    writeIO(address2, GPIOB, 0xAA);
    delay(500);
}

//init object with i2c address, default is 0x40 for ServoPi board

void initIO() {
    writeIO(address1, IODIRA, 0x00);
    writeIO(address1, IODIRB, 0x00);
    writeIO(address2, IODIRA, 0x00);
    writeIO(address2, IODIRB, 0x00);
    writeIO(address1, GPIOA, 0x00);
    writeIO(address1, GPIOB, 0x00);
    writeIO(address2, GPIOA, 0x00);
    writeIO(address2, GPIOB, 0x00);
   // if (readIO(address1, IODIRA) != 0x00){showerror();}
   // if (readIO(address2, IODIRA) != 0x00){showerror();}
}

void showerror(){
    digitalWrite(ERROR_PIN, 1);
}



void writeIO(byte addr, byte command, byte value) {	// LED_ON and LED_OFF are 12bit values (0-4095);
    Wire.beginTransmission(addr);
    Wire.write(command);    
    Wire.write(value);
    Wire.endTransmission();
}

byte readIO(byte addr, byte command) {	// LED_ON and LED_OFF are 12bit values (0-4095);
    Wire.beginTransmission(addr);
   // byte out = Wire.read(command);
    Wire.endTransmission();
    return 0;
}


