#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

#define ONE_WIRE_BUS 10

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned int L;
unsigned int LL;
byte addr[8];
byte data[12];
int celsius;
byte i;

//OneWire ds(10);

// ----- Инициализация
void setup() {
  Serial.begin(19200);
  pinMode(9, OUTPUT); // Chip Select
  digitalWrite(9, HIGH);

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
  //  pinMode(A2, OUTPUT);
  //  digitalWrite(A2, HIGH);
  delay(500);

  Serial.println("0");

  /*  Send7219(0x01, 0x00);
    Send7219(0x02, 0x0D);
    Send7219(0x03, 0x0B);
    Send7219(0x04, 0x0C);
  */
  //  ds.search(addr);
  sensors.begin();
  Serial.println("1110");

}
// ----- Конец инициализации



// ----- Главный цикл
void loop() {
  /*
    Serial.print("11");
    LL = analogRead(A0);
    if ( LL < 100 ) LL = 0;
    else LL = (LL - 100) / 64;
    Serial.print("12");
    if (LL > 15) LL = 15;
    Serial.print("13");
    Send7219(0x0A, (byte)LL);
  */

  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  celsius = sensors.getTempCByIndex(0) * 10;
  Serial.println(celsius/10);

  SendTemp();
}
// ----- Конец главного цикла

void Send7219(byte Addr, byte Data) {
  digitalWrite(9, LOW);
  SPI.transfer(Addr);
  SPI.transfer(Data);
  digitalWrite(9, HIGH);
}

void SendTemp() {
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));
  SPI.begin();

  Send7219(0x00, 0x00);
  Send7219(0xFF, 0x00);
  Send7219(0x09, 0xFF);
  Send7219(0x0A, 0xFF);
  Send7219(0x0B, 0x03);
  Send7219(0x0C, 0xFF);

  L = celsius % 10;
  Serial.print("C=");
  Serial.println(celsius);
  Serial.print("L=");
  Serial.println(L);
  Send7219(0x01, L);
  celsius = celsius / 10;
  L = celsius % 10;
  Serial.print("C=");
  Serial.println(celsius);
  Serial.print("L=");
  Serial.println(L);
  Send7219(0x02, L + 128);
  celsius = celsius / 10;
  L = celsius % 10;
  Serial.print("C=");
  Serial.println(celsius);
  Serial.print("L=");
  Serial.println(L);
  Send7219(0x03, L);
  celsius = celsius / 10;
  L = celsius % 10;
  Serial.print("C=");
  Serial.println(celsius);
  Serial.print("L=");
  Serial.println(L);
  Send7219(0x04, L);

  SPI.end();
}
