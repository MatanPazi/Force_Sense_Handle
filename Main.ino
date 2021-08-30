// Arduino code to communicate with smartphone via BLE
#include <TinyWireM.h>
#include <SoftwareSerial.h>
#include <SSD1306_minimal.h>

SoftwareSerial bluetooth(4,3); // BLE TX-> ATtiny85 PB4, BLE RX-> ATtiny85 PB3
SSD1306_Mini oled;   // OLED object

const float MEAN_DIFF = 0.2;                // Defines a threshold for the change in mean value (To ignore noise)
const float NUM_OF_SAMPLES = 50;            // Defines a base for the # of samples to test if there was a significant change in input
float Num_Of_Samples = NUM_OF_SAMPLES;      // Defines the # of samples to test if there was a significant change in input
byte counter = 0;
float ForceMean = 0;
float PrevForceMean = 0;
bool SendData = 1;
float Offset = 0;

char strValue[5];    // For converting float to string for text display. Needs to be 1 greater than largest # of characters. 20.0 (4 characters total?) so 5.

const int CONFIG = 0x7D;
const int INA = 0x40;
const int CONFIG_ADD = 0x00;
const int SHUNT_ADC = 0X01;
byte resp = 0;
byte stat = 0;
int data = 0;

void setup() {
  bluetooth.begin(9600); // start BLE device
  delay(1000); // wait until BLE device starts
  oled.init(0x3C);   // init OLED display
  oled.clear(); 
  delay(1000);
  TinyWireM.begin(); 
  TinyWireM.beginTransmission(INA);
  TinyWireM.write(CONFIG_ADD);
  TinyWireM.write(0);   
  TinyWireM.write(CONFIG);   
  TinyWireM.endTransmission();
  delay(1000);
}

void loop() {
// Get Shunt voltage  
  float ShuntVoltage = 0;
  TinyWireM.beginTransmission(INA);
  TinyWireM.write(SHUNT_ADC);
  TinyWireM.endTransmission(false);
  TinyWireM.requestFrom(INA, 2);
  delay(5);
  ShuntVoltage = (TinyWireM.read() << 8 | TinyWireM.read());
  ForceMean = ForceMean + ShuntVoltage;
  counter += 1;
//  Is data changing? To save power when values aren't changing
  if (counter >= Num_Of_Samples) {
    counter = 0;
    ForceMean = ForceMean/Num_Of_Samples;
    if ((abs(ForceMean - PrevForceMean)) < MEAN_DIFF) {
      SendData = 0;
      delay(100);
      Num_Of_Samples = NUM_OF_SAMPLES/10;
    }
    else {
      SendData = 1;
      Num_Of_Samples = NUM_OF_SAMPLES;
    }
    PrevForceMean = ForceMean;
    ForceMean = 0;
  }
  if (SendData == 1) {
// Display shunt voltage on OLED    
    dtostrf(ShuntVoltage, 4, 2, strValue);
    oled.cursorTo(0, 0);
    oled.cursorTo(30, 0);
    oled.printString(strValue);
    oled.printString(" Kg");
    delay(70);
// Send shunt voltage to BT module 
    bluetooth.write(byte(ShuntVoltage)); // Send byte to smartphone
  }
  else {
    oled.clear();
  }
}
