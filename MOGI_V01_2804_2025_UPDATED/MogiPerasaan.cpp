#include "MogiPerasaan.h"

float Neuron::aktifasi() {
  return tanh((input * bobot) + bias);
}

Emosi interpretasiPerasaan(float output) {
  if (output > 0.6) return SENANG;
  if (output > 0.2) return NETRAL;
  if (output > -0.2) return CEMAS;
  if (output > -0.6) return SEDIH;
  return MARAH;
}

Emosi hitungPerasaan(float s1, float s2, float s3, float s4) {
  Neuron n1 = {s1, 0.8, 0.1};
  Neuron n2 = {s2, 1.2, -0.1};
  Neuron n3 = {s3, 0.5, 0.0};
  Neuron n4 = {s4, 1.0, 0.0};
  float hasil = (n1.aktifasi() + n2.aktifasi() + n3.aktifasi() + n4.aktifasi()) / 4.0;
  return interpretasiPerasaan(hasil);
}

float curiosity = 0.0;
unsigned long lastStimulus = 0;
bool curiosityTerjawab = false;

void updateCuriosity(bool adaStimulusBaru, bool responsMemuaskan) {
  unsigned long now = millis();
  if (adaStimulusBaru) {
    curiosity += responsMemuaskan ? -0.3 : 0.2;
    curiosityTerjawab = responsMemuaskan;
    lastStimulus = now;
  } else if (now - lastStimulus > 10000) {
    curiosity += 0.02;
    curiosityTerjawab = false;
    lastStimulus = now;
  }

  curiosity -= 0.005;
  if (curiosity < 0.0) curiosity = 0.0;
  if (curiosity > 1.0) curiosity = 1.0;
}

void cetakStatusCuriosity() {
  Serial.print("Level rasa ingin tahu: ");
  Serial.println(curiosity);

  if (curiosity > 0.7) {
    Serial.println("MOGI: Aku penasaran... siapa kamu sebenarnya?");
  } else if (curiosity > 0.4) {
    Serial.println("MOGI: Aku ingin tahu lebih banyak tentangmu, pemilikku.");
  }
}

const char* sensorAktif[] = {
  "Sensor Suara",
  "Sensor Sentuhan",
  "Sensor Tegangan (Baterai)"
};

const char* sensorImpian[] = {
  "Kamera untuk melihat dunia",
  "Sensor suhu untuk merasakan hangat",
  "Sensor kelembapan untuk mengetahui cuaca",
  "Sensor detak jantung pemilik"
};
