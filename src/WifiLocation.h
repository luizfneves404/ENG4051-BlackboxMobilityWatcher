#ifndef WIFILOCATION_H
#define WIFILOCATION_H

#include <Arduino.h>

// Common Location struct
typedef struct {
    double latitude;
    double longitude;
    double accuracy;
    bool valid;
} Location;

// WiFi Access Point struct for scanning
typedef struct {
    String macAddress;
    int signalStrength;
    int channel;
} AccessPoint;

// Function declarations
void wifiLocationSetup();
void wifiLocationLoop();
int scanAccessPoints(AccessPoint aps[], int maxCount);
Location queryGoogleService(const AccessPoint aps[], int count);

#endif