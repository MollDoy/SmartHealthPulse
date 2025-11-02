#include <Wire.h>
#include "MAX30105.h"         
#include "heartRate.h"       
#include <MovingAvg.h>       

MAX30105 particleSensor;
// фильтр скользящего среднего
MovingAvg irFilter(10);      

float bpm = 0;
int beatAvg = 0;
const int numReadings = 4;
int readings[numReadings];   
int readIndex = 0;
int total = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("MAX30102 Signal Filtering");

  // инициализация датчика
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Ошибка: MAX30102 не найден. Проверьте подключение!");
    while (1);
  }

  // настройка чувствительности и частоты
  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeIR(0x0A);

  irFilter.begin();

  // инициализация массива усреднения
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
}

void loop() {
  // ИК сигнал
  long irValue = particleSensor.getIR();  
  // красный канал
  long redValue = particleSensor.getRed(); 
  
  int irFiltered = irFilter.reading(irValue); 

  // проверка наличия пальца
  if (irFiltered < 5000) {
    Serial.println("Ожидание пальца на датчике...");
    delay(1000);
    return;
  }

  // определение удара сердца
  if (checkForBeat(irFiltered)) {
    // расчёт времени между ударами
    static long lastBeat = 0;
    long delta = millis() - lastBeat;
    lastBeat = millis();

    bpm = 60 / (delta / 1000.0);
    if (bpm < 255 && bpm > 20) {
      total -= readings[readIndex];
      readings[readIndex] = bpm;
      total += readings[readIndex];
      readIndex = (readIndex + 1) % numReadings;
      beatAvg = total / numReadings;
    }

    Serial.print("IR: "); Serial.print(irFiltered);
    Serial.print(", RED: "); Serial.print(redValue);
    Serial.print(" -> BPM: "); Serial.print(beatAvg);
    Serial.println(" bpm");
  }

  delay(20);

}
