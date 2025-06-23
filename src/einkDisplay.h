#ifndef EINK_DISPLAY_H
#define EINK_DISPLAY_H

#include <Arduino.h>

// Function pointer type for MQTT publishing
typedef void (*MQTTPublishCallback)(const char* topic, const char* payload);

// Structure to hold preferences data
struct PreferenciasData {
  int speedMax;
  int accMax;
};

// Setup and loop functions
void einkDisplaySetup();
void einkDisplayLoop();
void mostraTela();

// MQTT callback setter
void setMQTTPublishCallback(MQTTPublishCallback callback);

// Preferences MQTT sender (now can be called externally)
void enviaDadosPreferenciasMQTT();

#endif
