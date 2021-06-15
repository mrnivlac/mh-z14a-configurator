/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Wire.h>

const int pinCal = 2;

SoftwareSerial mSerial(7, 6);  // RX, TX
LiquidCrystal_I2C lcd(
    0x27, 20,
    4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const byte setRange[] = {0xff, 0x01, 0x99, 0,   0,
                         0,    0x13, 0x88, 0xcb};  // 5000ppm

const byte calZero[] = {0xff, 0x01, 0x87, 0, 0, 0, 0, 0, 0x78};

const byte offSelfCal[] = {0xff, 0x01, 0x79, 0, 0, 0, 0, 0, 0x86};

void setup() {
  Serial.begin(57600);

  Serial.println("Goodnight moon!");

  Wire.setClock(100000);

  lcd.init();  // initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.home();
  lcd.print("init");

  pinMode(pinCal, INPUT);

  mSerial.begin(9600);
  mSerial.setTimeout(10);

  lcd.setCursor(0, 1);
  lcd.print("SERIAL");
  lcd.setCursor(0, 2);
  lcd.print("ANALOG");

  delay(1000);

  mSerial.write(setRange, 9);
  delay(500);
  mSerial.write(offSelfCal, 9);
  lcd.setCursor(0, 3);
  lcd.print("RANGE 5k; SELFCAL 0");

  lcd.home();
  lcd.print("     ");
}

void loop() {  // run over and over
  if (mSerial.available()) {
    byte recv[9] = {0};
    mSerial.readBytes(recv, 9);
    //    Serial.print('>');
    //    for(byte i = 0; i < 9; i++) Serial.print(recv[i],HEX);
    //    Serial.println();
    uint16_t ser = recv[2] * 256 + recv[3];
    Serial.print("ser: ");
    Serial.println(ser);
    double raw = analogRead(A0);
    delay(100);
    raw += analogRead(A0);
    delay(100);
    raw += analogRead(A0);
    delay(100);
    raw += analogRead(A0);
    raw = raw / 4;
    raw = raw * 5.0 / 1024.0;
    raw = (raw - 0.4) * 5000 / 1.6;
    uint16_t ana = round(raw);
    Serial.print("ana: ");
    Serial.println(ana);

    lcd.setCursor(7, 1);
    lcd.print(ser);
    lcd.print("    ");
    lcd.setCursor(7, 2);
    lcd.print(ana);
    lcd.print("    ");
  }

  static uint32_t last_meas = 0;
  if (millis() - last_meas >= 1000) {
    last_meas = millis();
    byte readCo2[] = {0xff, 0x01, 0x86, 0, 0, 0, 0, 0, 0x79};
    mSerial.write(readCo2, 9);
    //    Serial.println("polled");
    lcd.home();
    lcd.print((unsigned int)(last_meas / 1000));
  }

  if (!digitalRead(pinCal)) {
    delay(100);
    if (!digitalRead(pinCal)) {
      mSerial.write(calZero, 9);
      lcd.setCursor(0, 3);
      lcd.print("CAL ZERO SENT       ");
    }
  }
}
