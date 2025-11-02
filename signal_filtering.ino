#include <Wire.h>
#include "MAX30105.h"         // библиотека SparkFun MAX3010x
#include "heartRate.h"        // для расчёта BPM
#include <MovingAvg.h>        // для фильтрации

MAX30105 particleSensor;
MovingAvg irFilter(10);       // фильтр скользящего среднего

float bpm = 0;
int beatAvg = 0;
const int numReadings = 4;
int readings[numReadings];    // массив для усреднения
int readIndex = 0;
int total = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("SmartHealthPulse — Week 3–4: MAX30102 Signal Filtering");

  // Инициализация датчика
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Ошибка: MAX30102 не найден. Проверьте подключение!");
    while (1);
  }

  // Настройка чувствительности и частоты
  particleSensor.setup(); // стандартные настройки
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeIR(0x0A);

  irFilter.begin();

  // Инициализация массива усреднения
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;
  }
}

void loop() {
  long irValue = particleSensor.getIR();  // ИК сигнал
  long redValue = particleSensor.getRed(); // Красный канал

  int irFiltered = irFilter.reading(irValue); // фильтрация скользящим средним

  // Проверка наличия пальца
  if (irFiltered < 5000) {
    Serial.println("Ожидание пальца на датчике...");
    delay(1000);
    return;
  }

  // Определение удара сердца
  if (checkForBeat(irFiltered)) {
    // Расчёт времени между ударами
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

  delay(20); // частота измерений
}