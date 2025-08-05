#ifndef NEURAL_MEMORY_H
#define NEURAL_MEMORY_H

#include <Arduino.h>
#include "MogiPerasaan.h"

void simpanStateNeural(Neuron n1, Neuron n2, Neuron n3, Neuron n4, float curiosity, float output);
bool bacaStateNeural(Neuron &n1, Neuron &n2, Neuron &n3, Neuron &n4, float &curiosity, float &output);

#endif
