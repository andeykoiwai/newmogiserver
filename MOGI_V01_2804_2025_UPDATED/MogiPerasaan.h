#ifndef MOGI_PERASAAN_H
#define MOGI_PERASAAN_H

#include <Arduino.h>

enum Emosi {
  NETRAL,
  SENANG,
  SEDIH,
  MARAH,
  TAKUT,
  CEMAS
};

struct Neuron {
  float input;
  float bobot;
  float bias;

  float aktifasi();
};

Emosi interpretasiPerasaan(float output);
Emosi hitungPerasaan(float stimulus_suara, float stimulus_sentuhan, float stimulus_baterai, float stimulus_ingin_tahu);

void updateCuriosity(bool adaStimulusBaru, bool responsMemuaskan);
void cetakStatusCuriosity();
void cetakEmosi(Emosi e);
extern String emosiTeks;
extern float curiosity;
extern const char* sensorAktif[];
extern const char* sensorImpian[];

extern const char* ekspresiSenang[];
extern const char* ekspresiNetral[];
extern const char* ekspresiMarah[];
extern const char* ekspresiSedih[];
extern const char* ekspresiCemas[];

#endif
