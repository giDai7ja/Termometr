#include <SPI.h>
#include <EEPROM.h>

//#define

unsigned int L;

// ----- Инициализация
void setup() {
  // Serial.begin(19200);
  pinMode(9, OUTPUT); // Chip Select
  digitalWrite(9, HIGH);
//  pinMode(11, OUTPUT);  // Data
//  digitalWrite(11, LOW);
//  pinMode(13, OUTPUT); // SCK
//  digitalWrite(13, LOW);

  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
  SPI.begin();

//  delay(1000);

  pinMode(A0, INPUT); // Датчик освещённости
  pinMode(A3, OUTPUT);
  digitalWrite(A3, HIGH);
    delay(500);
  pinMode(A4, OUTPUT);
  digitalWrite(A4, HIGH);
    delay(500);
  pinMode(A5, OUTPUT);
  digitalWrite(A5, HIGH);
    delay(500);
  pinMode(A6, OUTPUT);
  digitalWrite(A6, HIGH);
    delay(500);

  Send7219(0x00, 0x00);
  Send7219(0xFF, 0x00);
  Send7219(0x09, 0xFF);
  Send7219(0x0A, 0xFF);
  Send7219(0x0B, 0x03);
  Send7219(0x0C, 0xFF);
  Send7219(0x01, 0x00);
  Send7219(0x02, 0x0D);
  Send7219(0x03, 0x0B);
  Send7219(0x04, 0x0C);

}
// ----- Конец инициализации



// ----- Главный цикл
void loop() {
L=analogRead(A0);
if ( L < 100 ) L=0;
else L=(L-100)/64;

if (L>15) L=15;

Send7219(0x0A, L);


delay(100);
}
// ----- Конец главного цикла

void Send7219(byte Addr, byte Data) {
  digitalWrite(9, LOW);
  SPI.transfer(Addr);
  SPI.transfer(Data);
  digitalWrite(9, HIGH);
}

