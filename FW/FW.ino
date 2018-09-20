#include <OneWire.h>
#include <SPI.h>
#include <EEPROM.h>

#define LightSensor A0
#define ChipSelect 9

byte Brightness;
byte disp[4] = {0x0F, 0x0F, 0x0F, 0x0F};
byte key[4] = {8, 7, 3, 2};
byte led[4] = {A3, A4, A5, A2};
byte i;
byte X9 = 0xFF;
byte index = 0;
byte addr[4][8];
byte data[4][12];
int Temp;
byte Hello[11] = {0x0F, 0x0F, 0x0F, 0x0C, 0x0B, 0x0D, 0x0D, 0x00, 0x0F, 0x0F, 0x0F};
unsigned long TimeGetTemp = 0;
unsigned long TimeSkanKey = 0;
unsigned long TimeDisplay = 4000;
byte StepGetTemp = 0;

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

  ds.search(addr[index]);
  ds.reset_search();

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
          ds.reset();
          ds.select(addr[i]);
          ds.write(0x44, 1);
        }
        StepGetTemp = 10;
        TimeGetTemp = millis() + 1000;
        break;
      case 10:
        for (byte j = 0; j < 4; j++) {
          ds.reset();
          ds.select(addr[j]);
          ds.write(0xBE);
          for ( i = 0; i < 9; i++) data[j][i] = ds.read();
        }
        StepGetTemp = 0;
        TimeGetTemp = millis() + 10000;
        break;
    }
  }
}

void SkanKey() {
  if (TimeSkanKey < millis()) {
    for (i = 0; i < 4; i++) {
      if (digitalRead(key[i])) {
        if (i != index) TimeDisplay = 0;
        index = i;
      }
    }

    if ( (OneWire::crc8(data[index], 8)) == data[index][8] ) {
      int16_t raw = (data[index][1] << 8) | data[index][0];

      Temp = abs(raw) / 1.6;
      for ( i = 0 ; i < 4 ; i++ ) {
        disp[i] = Temp % 10;
        Temp = Temp / 10;
      }
      disp[1] += 128; // Десятичная точка
      if (raw < 0) disp[3] = 0x0A;
      raw = abs(raw) / 1.6;
      if (raw < 1000) disp[3] = 0x0F;
      if (raw < 100) disp[2] = 0x0F;
      X9 = 0xFF;
    }
    else {
      X9 = 0xF8;
      disp[0] = 0x00;
      disp[1] = 0x05;
      disp[2] = 0x05;
      disp[3] = 0x0B;
    }
    TimeSkanKey = millis() + 100;
  }
}

void Display() {
  if (TimeDisplay < millis()) {
    Brightness = map(constrain(analogRead(LightSensor), 200, 600), 200, 600, 0, 15);

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
    TimeDisplay = millis() + 5000;
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
  Send7219(0x09, 0xFF);
  Send7219(0x0A, 0xFF);
  Send7219(0x0B, 0x03);
  Send7219(0x0C, 0xFF);
  for ( i = 0 ; i < 4 ; i++ ) Send7219(i + 1, 0x0F);

  for (byte j = 0; j < 8 ; j++ ) {
    for ( i = 0 ; i < 4 ; i++ ) Send7219(i + 1, Hello[3 - i + j]);
    delay(250);
  }

  for ( i = 0 ; i < 4 ; i++ ) Send7219(i + 1, 0x0F);

  SPI.end();
}

