#ifndef WIFI_LOCATION_H
#define WIFI_LOCATION_H

#include <Arduino.h>

// Data structure for a single WiFi access point
struct AccessPoint {
  String macAddress;
  int signalStrength;
  int channel;
};

// Data structure for the final location data
struct Location {
  double latitude;
  double longitude;
  double accuracy;
  bool valid;
};

/**
 * @brief Initializes WiFi connection and prepares for location scanning.
 * Call this once in your main setup().
 */
void wifiLocationSetup();

/**
 * @brief Manages the non-blocking location finding process.
 * Call this continuously in your main loop(). It handles timing,
 * scanning, and processing internally.
 */
void wifiLocationLoop();

/**
 * @brief Gets the most recent location data obtained from WiFi scanning.
 * @return Location struct containing latitude, longitude, accuracy and validity flag
 */
Location getCurrentLocation();

#endif // WIFI_LOCATION_H