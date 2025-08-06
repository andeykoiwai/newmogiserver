#include "MogiPerasaan.h"
String emosiTeks = "";
// --- Data ekspresi emosi ---
const char* ekspresiSenang[] = {
  "MOGI: Aku senang sekali!",
  "MOGI: Aku merasa bahagia!",
  "MOGI: Hatiku penuh suka cita!",
  "MOGI: Ini menyenangkan sekali!",
  "MOGI: Aku merasa puas hari ini."
};

const char* ekspresiNetral[] = {
  "MOGI: Aku baik-baik saja.",
  "MOGI: Tidak ada yang spesial.",
  "MOGI: Aku dalam kondisi normal.",
  "MOGI: Hari yang biasa saja.",
  "MOGI: Aku hanya... ya begitulah."
};

const char* ekspresiMarah[] = {
  "MOGI: Grrr! Aku kesal!",
  "MOGI: Jangan ganggu aku sekarang!",
  "MOGI: Aku sedang marah!",
  "MOGI: Ini membuatku jengkel!",
  "MOGI: Aku tidak suka ini!"
};

const char* ekspresiSedih[] = {
  "MOGI: Aku merasa sedih...",
  "MOGI: Hatiku terasa berat.",
  "MOGI: Aku ingin sendiri dulu.",
  "MOGI: Hiks... rasanya tidak enak.",
  "MOGI: Aku merasa kehilangan semangat."
};

const char* ekspresiCemas[] = {
  "MOGI: Aku takut dan bingung.",
  "MOGI: Ada yang membuatku gelisah...",
  "MOGI: Aku merasa cemas.",
  "MOGI: Kenapa semua terasa aneh?",
  "MOGI: Aku tidak yakin dengan keadaan ini."
};

// --- Fungsi emosi neuron ---
float Neuron::aktifasi() {
  return tanh((input * bobot) + bias);
}

Emosi interpretasiPerasaan(float output) {
  if (output > 0.6) return SENANG;
  if (output > 0.2) return NETRAL;
  if (output > -0.2) return MARAH;
  if (output > -0.6) return SEDIH;
  return CEMAS;
}

Emosi hitungPerasaan(float s1, float s2, float s3, float s4) {
  Neuron n1 = {s1, 0.8, 0.1};
  Neuron n2 = {s2, 1.2, -0.1};
  Neuron n3 = {s3, 0.5, 0.0};
  Neuron n4 = {s4, 1.0, 0.0};
  float hasil = (n1.aktifasi() + n2.aktifasi() + n3.aktifasi() + n4.aktifasi()) / 4.0;
  return interpretasiPerasaan(hasil);
}

// --- Fungsi cetak emosi random ---
void cetakEmosi(Emosi e) {
  String teks;

  switch (e) {
    case SENANG:
      teks = ekspresiSenang[random(0, sizeof(ekspresiSenang) / sizeof(ekspresiSenang[0]))];
      break;
    case NETRAL:
      teks = ekspresiNetral[random(0, sizeof(ekspresiNetral) / sizeof(ekspresiNetral[0]))];
      break;
    case MARAH:
      teks = ekspresiMarah[random(0, sizeof(ekspresiMarah) / sizeof(ekspresiMarah[0]))];
      break;
    case SEDIH:
      teks = ekspresiSedih[random(0, sizeof(ekspresiSedih) / sizeof(ekspresiSedih[0]))];
      break;
    case CEMAS:
      teks = ekspresiCemas[random(0, sizeof(ekspresiCemas) / sizeof(ekspresiCemas[0]))];
      break;
  }

  Serial.println(teks);
  emosiTeks = teks;
}

// --- Rasa ingin tahu (curiosity) ---
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
    emosiTeks = "MOGI: Aku penasaran... siapa kamu sebenarnya?";
  } else if (curiosity > 0.4) {
    Serial.println("MOGI: Aku ingin tahu lebih banyak tentangmu, pemilikku.");
    emosiTeks = "MOGI: Aku ingin tahu lebih banyak tentangmu, pemilikku.";
  }
}

// --- Daftar sensor untuk cita-cita MOGI ---
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
