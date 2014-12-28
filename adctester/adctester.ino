#include <Wire.h>
#include <LiquidCrystal.h>
// I2C address for MCP3422 - base address for MCP3424
#define MCP3422_ADDRESS 0X68
#define MCP3422_ADDRESS2 0X69
// fields in configuration register
#define MCP342X_GAIN_FIELD 0X03 // PGA field
#define MCP342X_GAIN_X1    0X00 // PGA gain X1
#define MCP342X_GAIN_X2    0X01 // PGA gain X2
#define MCP342X_GAIN_X4    0X02 // PGA gain X4
#define MCP342X_GAIN_X8    0X03 // PGA gain X8

#define MCP342X_RES_FIELD  0X0C // resolution/rate field
#define MCP342X_RES_SHIFT  2    // shift to low bits
#define MCP342X_12_BIT     0X00 // 12-bit 240 SPS
#define MCP342X_14_BIT     0X04 // 14-bit 60 SPS
#define MCP342X_16_BIT     0X08 // 16-bit 15 SPS
#define MCP342X_18_BIT     0X0C // 18-bit 3.75 SPS

#define MCP342X_CONTINUOUS 0X10 // 1 = continuous, 0 = one-shot

#define MCP342X_CHAN_FIELD 0X60 // channel field
#define MCP342X_CHANNEL_1  0X00 // select MUX channel 1
#define MCP342X_CHANNEL_2  0X20 // select MUX channel 2
#define MCP342X_CHANNEL_3  0X40 // select MUX channel 3
#define MCP342X_CHANNEL_4  0X60 // select MUX channel 4

#define MCP342X_START      0X80 // write: start a conversion
#define MCP342X_BUSY       0X80 // read: output not ready

int lederror = 9;
int ledok = 10;
LiquidCrystal lcd(6, 7, A0, A1, A2, A3);
long Voltage[8]; // Array used to store results
byte passed = 1;


//------------------------------------------------------------------------
// default adc congifuration register - resolution and gain added in setup()
uint8_t adcConfig = MCP342X_START | MCP342X_CHANNEL_1 | MCP342X_CONTINUOUS;
uint8_t adcConfigb = MCP342X_START | MCP342X_CHANNEL_1 | MCP342X_CONTINUOUS;
// divisor to convert ADC reading to milivolts
uint16_t mvDivisor;
//------------------------------------------------------------------------------
void halt(void)
{
 
  lcd.clear() ;
  lcd.setCursor(0, 1);
  lcd.print("I2C Error");
  while(1);
}
//------------------------------------------------------------------------------
// read mcp342x data
uint8_t mcp342xRead(int32_t &data)
{
  // pointer used to form int32 data
  uint8_t *p = (uint8_t *)&data;
  // timeout - not really needed?
  uint32_t start = millis();
  do {
    // assume 18-bit mode
    Wire.requestFrom(MCP3422_ADDRESS, 4);
    if (Wire.available() != 4) {
      Serial.println("read failed");
      return false;
    }
    for (int8_t i = 2; i >= 0; i--) {
      p[i] = Wire.read();
    }
    // extend sign bits
    p[3] = p[2] & 0X80 ? 0XFF : 0;
    // read config/status byte
    uint8_t s = Wire.read();
    if ((s & MCP342X_RES_FIELD) != MCP342X_18_BIT) {
      // not 18 bits - shift bytes for 12, 14, or 16 bits
      p[0] = p[1];
      p[1] = p[2];
      p[2] = p[3];
    }
    if ((s & MCP342X_BUSY) == 0) return true;
  } while (millis() - start < 500); //allows rollover of millis()
  Serial.println("read timeout");
  return false;
}

uint8_t mcp342xRead2(int32_t &data)
{
  // pointer used to form int32 data
  uint8_t *p = (uint8_t *)&data;
  // timeout - not really needed?
  uint32_t start = millis();
  do {
    // assume 18-bit mode
    Wire.requestFrom(MCP3422_ADDRESS2, 4);
    if (Wire.available() != 4) {
      Serial.println("read failed");
      return false;
    }
    for (int8_t i = 2; i >= 0; i--) {
      p[i] = Wire.read();
    }
    // extend sign bits
    p[3] = p[2] & 0X80 ? 0XFF : 0;
    // read config/status byte
    uint8_t s = Wire.read();
    if ((s & MCP342X_RES_FIELD) != MCP342X_18_BIT) {
      // not 18 bits - shift bytes for 12, 14, or 16 bits
      p[0] = p[1];
      p[1] = p[2];
      p[2] = p[3];
    }
    if ((s & MCP342X_BUSY) == 0) return true;
  } while (millis() - start < 500); //allows rollover of millis()
  Serial.println("read timeout");
  return false;
}
//------------------------------------------------------------------------------
// write mcp342x configuration byte
uint8_t mcp342xWrite(uint8_t config)
{
  Wire.beginTransmission(MCP3422_ADDRESS);
  Wire.write(config);
  Wire.endTransmission();
}

uint8_t mcp342xWrite2(uint8_t config)
{
  Wire.beginTransmission(MCP3422_ADDRESS2);
  Wire.write(config);
  Wire.endTransmission();
}

//------------------------------------------------------------------------------
int chan = 0;
int gain = 0;
int res = 0;


void setup()
{
  pinMode(lederror, OUTPUT);     
  pinMode(ledok, OUTPUT);  
  digitalWrite(lederror, LOW);
  digitalWrite(ledok, LOW);
  
   lcd.begin(16, 2);
   
    lcd.clear() ;
  lcd.setCursor(0, 1);
  lcd.print("Testing");
  Wire.begin();
  //uint8_t chan = 0XFF, gain = 0XFF, res = 0XFF;

/*
  for (uint8_t add = 0X0; add < 0X80; add++) {
    Wire.requestFrom(add, (uint8_t)1);
    if (Wire.available()) {
       lcd.clear() ;
    lcd.setCursor(0, 0);
    lcd.print(add);
    delay(200);
    //  Serial.print("Add: ");
    //  Serial.println(add, HEX);
    }
  }
  */
  dotest();
  showresults();

}
//------------------------------------------------------------------------------

void dotest() {
  lcd.clear() ;
    mvDivisor = 1 << (gain + 2*res);
   for(int x=0;x<=3;x++){
     adcConfig = MCP342X_START | MCP342X_CHANNEL_1 | MCP342X_CONTINUOUS;
     adcConfig |= x << 5 | res << 2 | gain;  
     mcp342xWrite(adcConfig);
     int32_t data;
     if (!mcp342xRead(data)) halt();
     if (!mcp342xRead(data)) halt();
     if (!mcp342xRead(data)) halt();
    if (data < 823) {
      lcd.setCursor(x, 0);
      lcd.print(x+1);
      lcd.setCursor(x, 1);
      lcd.print("L");
      passed = 0;
    }
    else if (data > 832) {
      lcd.setCursor(x, 0);
      lcd.print(x+1);
      lcd.setCursor(x, 1);
      lcd.print("H");
      passed = 0;
    } else {
      lcd.setCursor(x, 0);
      lcd.print("=");
    }
    delay(100);
   }
   for(int x=0;x<=3;x++){
     adcConfigb = MCP342X_START | MCP342X_CHANNEL_1 | MCP342X_CONTINUOUS;
     adcConfigb |= x << 5 | res << 2 | gain;  
     mcp342xWrite2(adcConfigb);
     int32_t data1;
     if (!mcp342xRead2(data1)) halt();
     if (!mcp342xRead2(data1)) halt();
     if (!mcp342xRead2(data1)) halt();
    if (data1 < 823) {
      lcd.setCursor(x+4, 0);
      lcd.print(x+5);
      lcd.setCursor(x+4, 1);
      lcd.print("L");
      passed = 0;
    }
    else if (data1 > 832) {
      lcd.setCursor(x+4, 0);
      lcd.print(x+5);
      lcd.setCursor(x+4, 1);
      lcd.print("H");
      passed = 0;
    } else {
      lcd.setCursor(x+4, 0);
      lcd.print("=");
    }
    delay(100);
   }
  
}
void showresults() {
  if (passed == 1) {
    digitalWrite(ledok, HIGH);
    lcd.setCursor(12, 1);
    lcd.print("Pass"); // print results
  } else {
    digitalWrite(lederror, HIGH);
    lcd.setCursor(12, 1);
    lcd.print("Fail"); // print results
  }
}


void loop()
{
 
}
