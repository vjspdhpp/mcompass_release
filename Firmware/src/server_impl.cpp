
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <soc/usb_serial_jtag_reg.h>

#include "common.h"
#include "func.h"
#include "macro_def.h"
static AsyncWebServer server(80);
const char *PARAM_MESSAGE = "message";
extern CompassState deviceState;

static int isPluggedUSB(void) {
  uint32_t *aa = (uint32_t *)USB_SERIAL_JTAG_FRAM_NUM_REG;
  uint32_t first = *aa;
  vTaskDelay(pdMS_TO_TICKS(10));
  return (int)(*aa - first);
}

static void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

static void apis(void) {
  server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", WiFi.localIP().toString());
  });
  server.on("/setIndex", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("index")) {
      int index = request->getParam("index")->value().toInt();
      deviceState = STATE_SERVER_INDEX;
      showFrame(index);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing index parameter");
    }
  });
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    deviceState = STATE_SERVER_INFO;
    String buildDate = __DATE__;
    String buildTime = __TIME__;
#ifdef BUILD_VERSION
    String buildVersion = BUILD_VERSION;
#else
    String buildVersion = "UNKNOWN";
#endif
    String gitBranch = "UNKNOWN";
#ifdef GIT_BRANCH
    gitBranch = GIT_BRANCH;
#endif
    String gitCommit = "UNKNOWN";
#ifdef GIT_COMMIT
    gitCommit = GIT_COMMIT;
#endif
    request->send(200, "text/json",
                  "{\"buildDate\":\"" + buildDate + "\",\"buildTime\":\"" +
                      buildTime + "\",\"buildVersion\":\"" + buildVersion +
                      "\",\"gitBranch\":\"" + gitBranch +
                      "\",\"gitCommit\":\"" + gitCommit + "\"}");
  });

  server.on("/spawn", HTTP_GET, [](AsyncWebServerRequest *request) {
    deviceState = STATE_SERVER_SPAWN;
    Location location = {
        .latitude = 0.0f,
        .longitude = 0.0f,
    };
    getHomeLocation(location);
    request->send(200, "text/json",
                  "{\"latitude\":\"" + String(location.latitude, 6) +
                      "\",\"longitude\":\"" + String(location.longitude, 6) +
                      "\"}");
  });
  server.on("/setColor", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("color")) {
      String color = request->getParam("color")->value();
      deviceState = STATE_SERVER_COLORS;
      char *endptr;
      int hexRgb = strtol(color.c_str() + 1, &endptr, 16);
      Serial.printf("setColor to %06X\n", hexRgb);
      showSolid(hexRgb);
      request->send(200);
    }
  });

  server.on("/setAzimuth", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("azimuth")) {
      float azimuth = request->getParam("azimuth")->value().toFloat();
      deviceState = STATE_GAME_COMPASS;
      showFrameByAzimuth(azimuth);
      request->send(200);
    }
  });

  server.on("/spawn", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("latitude") && request->hasParam("longitude")) {
      float latitude = request->getParam("latitude")->value().toFloat();
      float longitude = request->getParam("longitude")->value().toFloat();
      Location location = {.latitude = latitude, .longitude = longitude};
      saveHomeLocation(location);
      request->send(200);
    }
  });
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    deviceState = STATE_SERVER_WIFI;
    String ssid = WiFi.SSID();
    String password = WiFi.psk();
    request->send(200, "text/json",
                  "{\"ssid\":\"" + ssid + "\",\"password\":\"" + password +
                      "\"}");
  });
  server.on("/setWiFi", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid") && request->hasParam("password")) {
      String ssid = request->getParam("ssid")->value();
      String password = request->getParam("password")->value();
      Serial.printf("setWiFi: %s %s\n", ssid.c_str(), password.c_str());
      Preferences preferences;
      preferences.begin("wifi", false);
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      preferences.end();
      request->send(200);
      delay(3000);
      esp_restart();
    }
  });
}

static void localHotspot(void) {
  Serial.println("Starting local hotspot");
  WiFi.softAP("The Lost Compass", "");
  Serial.println("Local hotspot started");
}

static void launchServer(const char *defaultFile) {
  if (!MDNS.begin("esp32")) { // Set the hostname to "esp32.local"
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("Launching server");
  server.serveStatic("/", LittleFS, "/").setDefaultFile(defaultFile);
  server.onNotFound(notFound);
  server.begin();
  Serial.println("Server launched");
  apis();
}

void setupServer() {
  Serial.println("Setting up server");
  // 获取储存的WiFi配置
  Preferences preferences;
  preferences.begin("wifi", false);
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end();

  LittleFS.begin(false, "/littlefs", 32);
  // 没有WiFi配置无条件开启热点
  if (ssid.isEmpty()) {
    deviceState = STATE_HOTSPOT;
    Serial.println("No WiFi credentials found");
    localHotspot();
    launchServer("default.html");
    return;
  }

  // 有WiFi配置, 但是没有接入USB,不会开启后台.
  if (isPluggedUSB() == 0) {
    Serial.println("USB not plugged, server will not launch.");
    return;
  }

  Serial.printf("Connecting to %s\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  deviceState = STATE_CONNECT_WIFI;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  deviceState = STATE_COMPASS;

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  launchServer("index.html");
  server.onNotFound(notFound);

  server.begin();
  MDNS.addService("http", "tcp", 80);
}