#include "NeuralMemory.h"
#include <FFat.h>
#include <ArduinoJson.h>

void simpanStateNeural(Neuron n1, Neuron n2, Neuron n3, Neuron n4, float curiosity, float output) {
  File file = FFat.open("/neural_state.json", FILE_WRITE);
  if (!file) {
    Serial.println("Gagal menyimpan neural state!");
    return;
  }

  DynamicJsonDocument doc(512);
  doc["neuron"]["suara"]["input"]      = n1.input;
  doc["neuron"]["suara"]["bobot"]      = n1.bobot;
  doc["neuron"]["suara"]["bias"]       = n1.bias;

  doc["neuron"]["sentuhan"]["input"]   = n2.input;
  doc["neuron"]["sentuhan"]["bobot"]   = n2.bobot;
  doc["neuron"]["sentuhan"]["bias"]    = n2.bias;

  doc["neuron"]["baterai"]["input"]    = n3.input;
  doc["neuron"]["baterai"]["bobot"]    = n3.bobot;
  doc["neuron"]["baterai"]["bias"]     = n3.bias;

  doc["neuron"]["penasaran"]["input"]  = n4.input;
  doc["neuron"]["penasaran"]["bobot"]  = n4.bobot;
  doc["neuron"]["penasaran"]["bias"]   = n4.bias;

  doc["curiosity"] = curiosity;
  doc["last_emosi_output"] = output;
  doc["last_timestamp"] = millis();

  serializeJson(doc, file);
  file.close();
}

bool bacaStateNeural(Neuron &n1, Neuron &n2, Neuron &n3, Neuron &n4, float &curiosity, float &output) {
  if (!FFat.exists("/neural_state.json")) return false;
  File file = FFat.open("/neural_state.json", FILE_READ);
  if (!file) return false;

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error) return false;

  n1.input = doc["neuron"]["suara"]["input"];
  n1.bobot = doc["neuron"]["suara"]["bobot"];
  n1.bias  = doc["neuron"]["suara"]["bias"];

  n2.input = doc["neuron"]["sentuhan"]["input"];
  n2.bobot = doc["neuron"]["sentuhan"]["bobot"];
  n2.bias  = doc["neuron"]["sentuhan"]["bias"];

  n3.input = doc["neuron"]["baterai"]["input"];
  n3.bobot = doc["neuron"]["baterai"]["bobot"];
  n3.bias  = doc["neuron"]["baterai"]["bias"];

  n4.input = doc["neuron"]["penasaran"]["input"];
  n4.bobot = doc["neuron"]["penasaran"]["bobot"];
  n4.bias  = doc["neuron"]["penasaran"]["bias"];

  curiosity = doc["curiosity"];
  output = doc["last_emosi_output"];
  return true;
}
