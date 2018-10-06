#include <OneWire.h>
#include <SPI.h>
//#include <EEPROM.h>

#define LightSensor A0
#define ChipSelect 9

#define LightLow 80
#define LightHigh 450

// 28 FF F3 8B 31 18 1 5C  - On Board
// 28 FF 4C 0 33 18 2 9C
// 28 FF BA 3D 33 18 2 73
// 28 FF 77 5A 33 18 1 49
const byte Hello[18] PROGMEM = {0x0F, 0x0F, 0x0F, 0x0C, 0x0B, 0x0D, 0x0D, 0x00, 0x0F, 0x0F, 0x0F,
                                0x4E, 0x77, 0x37, 0x4F, 0x00, 0x00, 0x00
                               };

const byte ALLaddr[32] PROGMEM = {0x28, 0xFF, 0xF3, 0x8B, 0x31, 0x18, 0x01, 0x5C,
                                  0x28, 0xFF, 0x4C, 0x00, 0x33, 0x18, 0x02, 0x9C,
                                  0x28, 0xFF, 0xBA, 0x3D, 0x33, 0x18, 0x02, 0x73,
                                  0x28, 0xFF, 0x77, 0x5A, 0x33, 0x18, 0x01, 0x49
                                 };

byte Brightness, Brightness1, OldBrightness;
byte disp[4] = {0x0F, 0x0F, 0x0F, 0x0F};
byte OLDdisp[4] = {0x00, 0x80, 0x0F, 0x0F};
const byte key[4] = {8, 7, 3, 2};
const byte led[4] = {A3, A4, A5, A2};
byte i, j, k;
byte X9 = 0xFF;
byte index = 0;
byte addr[8];
byte data[4][12];
int Temp, Light;
unsigned long TimeGetTemp = 0;
unsigned long TimeSkanKey = 0;
byte StepGetTemp = 0;
bool NewDisp = false;

OneWire  ds(10);

// ----- Инициализация
void setup() {
  Serial.begin(19200);
  pinMode(ChipSelect, OUTPUT); // Chip Select
  digitalWrite(ChipSelect, HIGH);
  pinMode(LightSensor, INPUT); // Датчик освещённости

  Show();

  for (i = 0; i < 4; i++) {
    pinMode(led[i], OUTPUT);
    digitalWrite(led[i], HIGH);
    pinMode(key[i], INPUT);
    delay(100);
    digitalWrite(led[i], LOW);
  }
}
// ----- Конец инициализации


// ----- Главный цикл
void loop() {
  GetTemp();
  SkanKey();
  Display();
}
// ----- Конец главного цикла

void GetTemp() {
  if (TimeGetTemp < millis()) {
    switch (StepGetTemp) {
      case 0:
        for (i = 0; i < 4; i++) {
          for (j = 0; j < 8; j++) {
            addr[j] = pgm_read_byte_near(ALLaddr + i * 8 + j);
          }
          ds.reset();
          ds.select(addr);
          ds.write(0x44, 1);
        }
        StepGetTemp = 10;
        TimeGetTemp = millis() + 1000;
        break;
      case 10:
        for (j = 0; j < 4; j++) {
          for (i = 0; i < 8 ; i++) addr[i] = pgm_read_byte_near(ALLaddr + j * 8 + i);
          ds.reset();
          ds.select(addr);
          ds.write(0xBE);
          for ( i = 0; i < 9; i++) data[j][i] = ds.read();
        }
        StepGetTemp = 0;
        TimeGetTemp = millis() + 30000;
        break;
    }
  }
}

void SkanKey() {
  if (TimeSkanKey < millis()) {

    Light = analogRead(LightSensor);
    Brightness = map(constrain((Light), LightLow, LightHigh), LightLow, LightHigh, 3, 15);
    Brightness1 = map(constrain((Light + 5), LightLow, LightHigh), LightLow, LightHigh, 3, 15);
    if ( ( Brightness == Brightness1 ) && (Brightness != OldBrightness) ) {
      NewDisp = true;
      OldBrightness = Brightness;
    }

    for (i = 0; i < 4; i++) {
      if (digitalRead(key[i])) {
        if (i != index) NewDisp = true;
        index = i;
      }
    }

    if ( (OneWire::crc8(data[index], 8)) == data[index][8] ) {
      int16_t raw = (data[index][1] << 8) | data[index][0];

      Temp = ( abs(raw) / 1.6 ) + 0.5;
      for ( i = 0 ; i < 4 ; i++ ) {
        disp[i] = Temp % 10;
        Temp = Temp / 10;
      }
      disp[1] += 128; // Десятичная точка
      Temp = ( abs(raw) / 1.6 ) + 0.5;
      if (Temp < 1000) disp[3] = 0x0F;
      if (Temp < 100) disp[2] = 0x0F;
      if (raw < 0) disp[3] = 0x0A;
      X9 = 0xFF;
    }
    else {
      X9 = 0xF8;
      disp[0] = 0x00;
      disp[1] = 0x05;
      disp[2] = 0x05;
      disp[3] = 0x0B;
    }

    for (i = 0; i < 4; i++) {
      if (OLDdisp[i] != disp[i] ) {
        NewDisp = true;
        OLDdisp[i] = disp[i];
      }
    }

    TimeSkanKey = millis() + 100;
  }
}

void Display() {
  if (NewDisp) {

    SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
    SPI.begin();

    Send7219(0xFF, 0x00);
    Send7219(0x09, X9);
    Send7219(0x0A, Brightness);
    Send7219(0x0B, 0x03);
    Send7219(0x0C, 0xFF);
    for ( i = 0 ; i < 4 ; i++ ) Send7219(i + 1, disp[i]);

    SPI.end();

    for ( i = 0 ; i < 4 ; i++ )
      if ( index == i ) digitalWrite(led[i], HIGH);
      else     digitalWrite(led[i], LOW);

    NewDisp = false;
  }
}

void Send7219(byte Addr, byte Data) {
  digitalWrite(ChipSelect, LOW);
  SPI.transfer(Addr);
  SPI.transfer(Data);
  digitalWrite(ChipSelect, HIGH);
}

void Show() {
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
  SPI.begin();

  Send7219(0xFF, 0x00);
  Send7219(0x09, 0x00);
  Send7219(0x0A, 0x0F);
  Send7219(0x0B, 0x03);
  Send7219(0x0C, 0x00);
  for ( i = 0 ; i < 4 ; i++ ) Send7219(i + 1, 0x00);

  delay(500);

  /*  // HELLO
    for (j = 0; j < 8 ; j++ ) {
      for ( i = 0 ; i < 4 ; i++ ) Send7219(i + 1, pgm_read_byte_near(Hello + 3 - i + j) );
      delay(250);
    }
    for ( i = 0 ; i < 4 ; i++ ) Send7219(i + 1, 0x0F);
    delay(1000);
    // HELLO
  */

  // CAHE
  Send7219(0x0C, 0x0F);

  for (j = 0; j < 4 ; j++ ) {
    for ( i = 0 ; i < 4 - j ; i++ ) {
      Send7219(i + 1, pgm_read_byte_near(Hello + 11 + j) );
      for ( k = 0 ; k < i; k++ ) Send7219(k + 1, 0x00 );
      delay(120);
    }
    delay(250);
  } // Написали
  delay(1000);
  for (j = 0; j < 4 ; j++ ) {
    for ( i = 0 ; i < 4 ; i++ ) Send7219(i + 1, pgm_read_byte_near(Hello + 14 - i + j) );
    delay(120);
  } // Уехали
  Send7219(0x0C, 0x00);
  // CAHE
  delay(500);

  // 60
  Send7219(0x09, 0x0F);
  Send7219(4, 0x0F);
  Send7219(3, 0x06);
  Send7219(2, 0x00);
  Send7219(1, 0x0F);
  for ( i = 0 ; i < 3 ; i++ ) {
    Send7219(0x0C, 0x0F);
    delay(500);
    Send7219(0x0C, 0x00);
    delay(500);
  }
  //60

  SPI.end();
}
