#include "WifiLocation.h"
#include "env.h" // Your file with the GOOGLE_API_KEY
#include "certificates.h"

// Conditional includes for real hardware vs. mock data
#ifndef MOCK_WIFI_SCAN
#include <WiFi.h>
#else
#include <cstring>
#endif

#ifndef MOCK_GOOGLE_SERVICE
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#endif

#include <ArduinoJson.h>

// ---------- Constants and State Management ----------
const int MAX_AP = 20; // Max access points to consider
const unsigned long LOCATION_INTERVAL = 60000; // Get location every 60 seconds
unsigned long previousLocationTime = 0;

// Global variable to store the current location
static Location currentLocation = {0, 0, 0, false};

// State machine for non-blocking WiFi scan
enum LocationState {
  STATE_IDLE,
  STATE_SCANNING,
  STATE_SCAN_COMPLETE
};
static LocationState locationState = STATE_IDLE; // Use 'static' to keep state within this file

// ---------- Mock Data (if enabled) ----------
#ifdef MOCK_WIFI_SCAN
const char *mockScan[] = {
    "60:22:32:ac:6a:72 -50 11",
    "40:ed:00:84:f1:46 -57 6",
    "ee:50:00:84:f1:46 -57 6",
    "ee:40:00:84:f1:46 -57 6",
    "62:22:32:bc:6a:72 -57 11",
    "62:22:32:9c:6a:72 -57 11",
    "62:22:32:9c:6a:73 -58 44",
    "62:22:32:ac:6a:73 -58 44",
    "60:22:32:ac:6a:73 -58 44",
    "78:98:e8:c4:f0:e9 -60 1",
    "62:22:32:ab:61:62 -60 1",
    "60:22:32:ab:61:62 -60 1",
    "62:22:32:bb:61:62 -60 1",
    "62:22:32:ac:6a:72 -60 11",
    "62:22:32:bc:6a:73 -60 44",
    "62:22:32:bb:4d:96 -68 1",
    "ee:50:00:84:f1:48 -71 149",
    "60:22:32:ab:4d:96 -72 1",
    "d2:66:1f:23:00:00 -72 6",
    "40:ed:00:84:f1:48 -72 149",
    "ee:40:00:84:f1:48 -73 149",
    "94:57:a5:7d:1a:8c -75 1",
    "62:22:32:ab:61:63 -77 44",
    "60:22:32:ab:61:63 -77 44",
    "62:22:32:9b:4d:96 -78 1",
    "62:22:32:ab:4d:96 -78 1",
    "b0:be:76:12:7f:51 -78 4",
    "74:83:c2:9d:08:a7 -78 6",
    "62:22:32:9b:42:32 -78 11",
    "62:22:32:bb:61:63 -78 44",
    "62:22:32:9b:61:63 -78 44",
    "56:60:f9:b6:a6:0b -80 7",
    "78:98:e8:c4:f0:eb -80 149",
    "60:22:32:ab:36:96 -82 1",
    "62:22:32:9b:61:62 -82 1",
    "b6:69:b0:a8:cb:65 -82 11",
    "62:22:32:9b:4d:97 -86 157",
    "62:22:32:bb:37:47 -87 44",
    "76:83:c2:9e:08:a7 -87 157",
    "60:22:32:ab:37:47 -88 44",
    "62:22:32:ab:4d:97 -88 157",
    "62:22:32:ab:37:47 -88 44",
    "62:22:32:9b:37:47 -88 44",
    "86:83:c2:9e:08:a7 -88 157",
    "60:22:32:ab:4d:97 -93 157"};
const int mockScanCount = sizeof(mockScan) / sizeof(mockScan[0]);
#endif

#ifdef MOCK_GOOGLE_SERVICE
const char* mockGoogleResponse = R"({
  "location": { "lat": -22.9789236, "lng": -43.231435 },
  "accuracy": 14.322
})";
#endif

// ---------- Function Prototypes (for internal use) ----------
int scanAccessPoints(AccessPoint aps[], int maxCount, int networkCount);
Location queryGoogleService(const AccessPoint aps[], int count);
void reconectarWiFi();


// ---------- Public Functions (declared in .h) ----------

void wifiLocationSetup() {
#ifdef MOCK_WIFI_SCAN
  Serial.println("[MOCK] Using mock WiFi scan data");
#else
  WiFi.disconnect();
  delay(100);
  reconectarWiFi();
#endif
  Serial.println("Sistema de localização WiFi iniciado");
}

void wifiLocationLoop() {
  unsigned long currentTime = millis();

  switch (locationState) {
    case STATE_IDLE:
      if (currentTime - previousLocationTime >= LOCATION_INTERVAL) {
        Serial.println("--- Iniciando verificação de localização ---");
#ifndef MOCK_WIFI_SCAN
        WiFi.scanNetworks(true); // Start ASYNCHRONOUS scan
#endif
        locationState = STATE_SCANNING;
        previousLocationTime = currentTime;
      }
      break;

    case STATE_SCANNING: {
#ifdef MOCK_WIFI_SCAN
      Serial.println("[MOCK] Scan complete.");
      locationState = STATE_SCAN_COMPLETE;
#else
      int8_t scanResult = WiFi.scanComplete();
      if (scanResult >= 0) {
        Serial.printf("Scan completo! %d redes encontradas.\n", scanResult);
        locationState = STATE_SCAN_COMPLETE;
      } else if (scanResult == WIFI_SCAN_FAILED) {
        Serial.println("A varredura de WiFi falhou.");
        locationState = STATE_IDLE;
      }
      // If still running, do nothing and wait for the next loop iteration
#endif
      break;
    }

    case STATE_SCAN_COMPLETE: {
      AccessPoint aps[MAX_AP];
      int networkCount = 0;

#ifdef MOCK_WIFI_SCAN
      networkCount = mockScanCount;
#else
      networkCount = WiFi.scanComplete();
#endif
      
      int populatedCount = scanAccessPoints(aps, MAX_AP, networkCount);
      Location loc = queryGoogleService(aps, populatedCount);

      // Update the global current location
      currentLocation = loc;

      if (loc.valid) {
        Serial.printf("Location: %.6f, %.6f (accuracy: %.2f m)\n", loc.latitude, loc.longitude, loc.accuracy);
      } else {
        Serial.println("Falha ao obter a localização.");
      }
      
      // Return to IDLE state to wait for the next interval
      locationState = STATE_IDLE;
      break;
    }
  }
}

// ---------- Internal Implementation Functions ----------

void reconectarWiFi() {
#ifndef MOCK_WIFI_SCAN
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin("Projeto", "2022-11-07");
    Serial.print("Conectando ao WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.print("conectado!\nEndereço IP: ");
    Serial.println(WiFi.localIP());
  }
#endif
}

int scanAccessPoints(AccessPoint aps[], int maxCount, int networkCount) {
#ifdef MOCK_WIFI_SCAN
  int n = min(maxCount, mockScanCount);
  for (int i = 0; i < n; i++) {
    char buf[64];
    strcpy(buf, mockScan[i]);
    char* token = strtok(buf, " ");
    aps[i].macAddress = String(token);
    token = strtok(nullptr, " ");
    aps[i].signalStrength = atoi(token);
    token = strtok(nullptr, " ");
    aps[i].channel = atoi(token);
  }
  return n;
#else
  int n = min(networkCount, maxCount);
  for (int i = 0; i < n; i++) {
    aps[i].macAddress = WiFi.BSSIDstr(i);
    aps[i].signalStrength = WiFi.RSSI(i);
    aps[i].channel = WiFi.channel(i);
  }
  WiFi.scanDelete(); // Free memory from scan results
  return n;
#endif
}

Location queryGoogleService(const AccessPoint aps[], int count) {
  Location loc = {0, 0, 0, false};
  if (count == 0) {
    Serial.println("No access points found to query Google Service.");
    return loc;
  }
  
  JsonDocument doc;
  doc["considerIp"] = false;
  for (int i = 0; i < count; i++) {
    doc["wifiAccessPoints"][i]["macAddress"] = aps[i].macAddress;
    doc["wifiAccessPoints"][i]["signalStrength"] = aps[i].signalStrength;
    doc["wifiAccessPoints"][i]["channel"] = aps[i].channel;
  }

#ifdef MOCK_GOOGLE_SERVICE
  Serial.println("[MOCK] Sending payload to Google API:");
  serializeJsonPretty(doc, Serial);
  Serial.println();
  JsonDocument respDoc;
  deserializeJson(respDoc, mockGoogleResponse);
  if (respDoc.containsKey("location")) {
    loc.latitude = respDoc["location"]["lat"];
    loc.longitude = respDoc["location"]["lng"];
    loc.accuracy = respDoc["accuracy"];
    loc.valid = true;
  }
#else
  reconectarWiFi();
  
  // Create secure WiFi client for HTTPS
  WiFiClientSecure client;
  client.setCACert(get_google_ca_cert());
  
  HTTPClient http;
  String url = String("https://www.googleapis.com/geolocation/v1/geolocate?key=") + String(GOOGLE_API_KEY);
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  
  String body;
  serializeJson(doc, body);
  
  Serial.println("Sending HTTPS request to Google Geolocation API...");
  int code = http.POST(body);
  
  if (code == 200) {
    String response = http.getString();
    JsonDocument respDoc;
    DeserializationError error = deserializeJson(respDoc, response);
    
    if (error) {
      Serial.printf("JSON parsing error: %s\n", error.c_str());
    } else if (respDoc.containsKey("location")) {
      loc.latitude = respDoc["location"]["lat"];
      loc.longitude = respDoc["location"]["lng"];
      loc.accuracy = respDoc["accuracy"];
      loc.valid = true;
      Serial.println("Location successfully retrieved");
    } else {
      Serial.println("Invalid response format from Google API");
    }
  } else {
    Serial.printf("HTTPS request failed with code: %d\n", code);
    if (code > 0) {
      Serial.printf("Response: %s\n", http.getString().c_str());
    }
  }
  
  http.end();
#endif
  return loc;
}

Location getCurrentLocation() {
  return currentLocation;
}