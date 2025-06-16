// Define these to use mock services
// Comment out to use real hardware/APIs
// #define MOCK_WIFI_SCAN
// #define MOCK_GOOGLE_SERVICE

#include "WifiLocation.h"
#include "env.h"

#ifndef MOCK_WIFI_SCAN
#include <WiFi.h>
#else
#include <cstring>
#endif

#ifndef MOCK_GOOGLE_SERVICE
#include <HTTPClient.h>
#endif

#include <ArduinoJson.h>

// ---------- Mock Data ----------
#ifdef MOCK_WIFI_SCAN
// Example scan data lines: "62:22:32:ac:6a:72 -53 11"
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
// Example Google API response payload
const char *mockGoogleResponse = R"({
  "location": {
    "lat": -22.9789236,
    "lng": -43.231435
  },
  "accuracy": 14.322
})";
#endif

void reconectarWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin("Projeto", "2022-11-07");
    Serial.print("Conectando ao WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(1000);
    }
    Serial.print("conectado!\nEndereço IP: ");
    Serial.println(WiFi.localIP());
  }
}

// Function prototypes
int scanAccessPoints(AccessPoint aps[], int maxCount);
Location queryGoogleService(const AccessPoint aps[], int count);

void wifiLocationSetup()
{
#ifdef MOCK_WIFI_SCAN
  Serial.println("[MOCK] Using mock WiFi scan data");
#else
  reconectarWiFi();
#endif
}

void wifiLocationLoop()
{
  // Scan or mock-scan APs
  reconectarWiFi();
  const int MAX_AP = 10;
  AccessPoint aps[MAX_AP];
  int count = scanAccessPoints(aps, MAX_AP);

  // Query Google (mock or real)
  Location loc = queryGoogleService(aps, count);

  if (loc.valid)
  {
    Serial.printf("Location: %.6f, %.6f (accuracy: %.2f m)\n", loc.latitude, loc.longitude, loc.accuracy);
  }
  else
  {
    Serial.println("Failed to obtain location");
  }
}

// ---------- Implementation ----------
int scanAccessPoints(AccessPoint aps[], int maxCount)
{
#ifdef MOCK_WIFI_SCAN
  int n = min(maxCount, mockScanCount);
  for (int i = 0; i < n; i++)
  {
    // parse mac, rssi, channel
    char buf[64];
    strcpy(buf, mockScan[i]);
    char *token = strtok(buf, " ");
    aps[i].macAddress = String(token);
    token = strtok(nullptr, " ");
    aps[i].signalStrength = atoi(token);
    token = strtok(nullptr, " ");
    aps[i].channel = atoi(token);
  }
  return n;
#else
  int n = WiFi.scanNetworks();
  n = min(n, maxCount);
  for (int i = 0; i < n; i++)
  {
    aps[i].macAddress = WiFi.BSSIDstr(i);
    aps[i].signalStrength = WiFi.RSSI(i);
    aps[i].channel = WiFi.channel(i);
  }
  return n;
#endif
}

Location queryGoogleService(const AccessPoint aps[], int count)
{
  Location loc{0, 0, 0, false};

  // Build JSON payload
  DynamicJsonDocument doc(2048);
  doc["considerIp"] = false;
  JsonArray arr = doc.createNestedArray("wifiAccessPoints");
  for (int i = 0; i < count; i++)
  {
    JsonObject obj = arr.createNestedObject();
    obj["macAddress"] = aps[i].macAddress;
    obj["signalStrength"] = aps[i].signalStrength;
    obj["channel"] = aps[i].channel;
  }

#ifdef MOCK_GOOGLE_SERVICE
  Serial.println("[MOCK] Sending payload to Google API:");
  serializeJsonPretty(doc, Serial);
  Serial.println();

  // Parse mock response
  DynamicJsonDocument respDoc(512);
  deserializeJson(respDoc, mockGoogleResponse);
  JsonObject r = respDoc.as<JsonObject>();
  if (r.containsKey("location"))
  {
    loc.latitude = r["location"]["lat"];
    loc.longitude = r["location"]["lng"];
    loc.accuracy = r["accuracy"];
    loc.valid = true;
  }
  return loc;
#else
  // Real HTTP POST to Google
  HTTPClient http;
  String url = String("https://www.googleapis.com/geolocation/v1/geolocate?key=") + String(GOOGLE_API_KEY); // get from env
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  String body;
  serializeJson(doc, body);
  int code = http.POST(body);
  if (code == 200)
  {
    String resp = http.getString();
    DynamicJsonDocument respDoc(512);
    deserializeJson(respDoc, resp);
    JsonObject r = respDoc.as<JsonObject>();
    if (r.containsKey("location"))
    {
      loc.latitude = r["location"]["lat"];
      loc.longitude = r["location"]["lng"];
      loc.accuracy = r["accuracy"];
      loc.valid = true;
    }
  }
  else
  {
    Serial.printf("HTTP error: %d\n", code);
  }
  http.end();
  return loc;
#endif
}
