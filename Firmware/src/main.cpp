#include <FastLED.h>
#include <OneButton.h>
#include <Preferences.h>
#include <QMC5883LCompass.h>
#include <TinyGPSPlus.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "func.h"
#include "macro_def.h"

static const char *TAG = "MAIN";
extern CRGB leds[NUM_LEDS];
extern QMC5883LCompass compass;
TinyGPSPlus gps;
HardwareSerial GPSSerial(0);
OneButton button(CALIBRATE_PIN, true);

// 目标位置
Location targetLoc = {.latitude = 43.0f, .longitude = 126.0f};

// 当前位置
Location currentLoc = {.latitude = 1000.0f, .longitude = 1000.0f};

// 状态
CompassState deviceState = CompassState::STATE_LOST_BEARING;
CompassState lastDeviceState = CompassState::STATE_LOST_BEARING;
// 工作模式模式
CompassType deviceType = CompassType::LocationCompass;
uint8_t idx = 0;

void setup() {
  // 延时,用于一些特殊情况下能够重新烧录
  delay(800);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  long t = millis();

  while (millis() - t < 2500) {
    showFrame(idx);
    idx++;
    if (idx > NUM_FRAMES) {
      idx = 0;
    }
    delay(30);
  }
  Serial.begin(115200);
  // 配置GPS串口
  GPSSerial.begin(9600, SERIAL_8N1, RX, TX);
  // 创建显示任务
  xTaskCreate(displayTask, "displayTask", 4096, NULL, 2, NULL);
  // 创建位置任务
  xTaskCreate(locationTask, "locationTask", 4096, NULL, 2, NULL);
  xTaskCreate(buttonTask, "buttonTask", 4096, NULL, 2, NULL);
  // 配置校准引脚状态
  pinMode(CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(GPS_EN_PIN, OUTPUT);
  digitalWrite(GPS_EN_PIN, HIGH);
  // 获取目标位置
  getHomeLocation(targetLoc);
  // 初始化罗盘
  compass.init();
  // 校准引脚被按下时候进行校准
  if (digitalRead(CALIBRATE_PIN) == LOW) {
    calibrateCompass();
  }
  // 结束校准阶段, 开启GPS, 等待GPS信号
  digitalWrite(GPS_EN_PIN, LOW);
  deviceState = STATE_WAIT_GPS;
  button.attachClick(
      [](void *scope) {
        switch (deviceState) {
        case CompassState::STATE_COMPASS: {
          if (deviceType == CompassType::LocationCompass) {
            deviceType = CompassType::NorthCompass;
          } else {
            deviceType = CompassType::LocationCompass;
          }
          Serial.print("Toggle Compass Type to ");
          Serial.println(deviceType == CompassType::LocationCompass
                             ? "LocationCompass"
                             : "NorthCompass");
          break;
        }

        default:
          break;
        }
      },
      &button);
  button.attachLongPressStart(
      [](void *scope) {
        switch (deviceState) {
        case CompassState::STATE_COMPASS: {
          // 设置当前地点为Home
          // 检查GPS状态
          if (currentLoc.latitude < 500.0f) {
            saveHomeLocation(currentLoc);
            targetLoc.latitude = currentLoc.latitude;
            targetLoc.longitude = currentLoc.longitude;
            Serial.println("Set Home");
          } else {
            Serial.println("Can't set home");
          }
          break;
        }
        case CompassState::STATE_CONNECT_WIFI: {
          Serial.println("Clear WiFi");
          // 清空WiFi配置
          Preferences preferences;
          preferences.begin("wifi", false);
          preferences.putString("ssid", "");
          preferences.putString("password", "");
          preferences.end();
          delay(3000);
          esp_restart();
        }

        default:
          break;
        }
      },
      &button);
  deviceState = STATE_COMPASS;
  // 插入电脑情况下会启动服务器;
  setupServer();
}

void loop() { delay(1000); }

void displayTask(void *pvParameters) {
  while (1) {
    switch (deviceState) {
    case STATE_LOST_BEARING: {
      // 校准罗盘中
      lostBearing();
      break;
    }
    case STATE_WAIT_GPS: {
      // 等待GPS数据
      theNether();
      delay(50);
      continue;
    }
    case STATE_COMPASS: {
      compass.read();
      float azimuth = compass.getAzimuth();
      if (azimuth < 0) {
        azimuth += 360;
      }
      if (deviceType == CompassType::LocationCompass) {
        // 检测当前坐标是否合法
        if (currentLoc.latitude < 200.0f) {
          showFrameByLocation(targetLoc.latitude, targetLoc.longitude,
                              currentLoc.latitude, currentLoc.longitude,
                              azimuth);
        } else {
          theNether();
          delay(50);
          continue;
        }
      } else {
#if DEBUG_DISPLAY
        Serial.printf("Azimuth = %d\n", azimuth);
#endif
        showFrameByAzimuth(azimuth);
      }
      delay(50);
      break;
    }
    case STATE_CONNECT_WIFI:
      showFrame(idx, CRGB::Green);
      idx++;
      if (idx > NUM_FRAMES) {
        idx = 0;
      }
      delay(30);
      break;
    case STATE_SERVER_COLORS: {
      // showServerColors();
      delay(50);
      break;
    }
    case STATE_SERVER_WIFI: {
      showServerWifi();
      break;
    }
    case STATE_SERVER_SPAWN: {
      showServerSpawn();
      break;
    }
    case STATE_SERVER_INFO: {
      showServerInfo();
      break;
    }
    case STATE_HOTSPOT: {
      showFrame(idx, CRGB::Yellow);
      idx++;
      if (idx > NUM_FRAMES) {
        idx = 0;
      }
      delay(30);
      break;
    }
    default:
      delay(50);
      break;
    }
  }
}

/**
 * @brief 位置任务
 *
 */
void locationTask(void *pvParameters) {
  // esp_task_wdt_init(30, false);
  // 失去GPS信号太久会回到STATE_LOST_BEARING
  while (1) {
    while (GPSSerial.available() > 0) {
      char t = GPSSerial.read();
#if DEBUG_DISPLAY
      Serial.print(t);
#endif
      if (gps.encode(t)) {
#if DEBUG_DISPLAY
        Serial.print("Location: ");
#endif
        if (gps.location.isValid()) {
#if DEBUG_DISPLAY
          Serial.print(gps.location.lat(), 6);
          Serial.print(",");
          Serial.print(gps.location.lng(), 6);
#endif
          // 坐标有效情况下更新本地坐标
          currentLoc.latitude = static_cast<float>(gps.location.lat());
          currentLoc.longitude = static_cast<float>(gps.location.lng());
        } else {
#if DEBUG_DISPLAY
          Serial.print("INVALID");
#endif
        }

#if DEBUG_DISPLAY
        Serial.print("  Date/Time: ");
        if (gps.date.isValid()) {
          Serial.print(gps.date.month());
          Serial.print("/");
          Serial.print(gps.date.day());
          Serial.print("/");
          Serial.print(gps.date.year());
        } else {
          Serial.print("INVALID");
        }

        Serial.print(" ");
        if (gps.time.isValid()) {
          if (gps.time.hour() < 10)
            Serial.print("0");
          Serial.print(gps.time.hour());
          Serial.print(":");
          if (gps.time.minute() < 10)
            Serial.print("0");
          Serial.print(gps.time.minute());
          Serial.print(":");
          if (gps.time.second() < 10)
            Serial.print("0");
          Serial.print(gps.time.second());
          Serial.print(".");
          if (gps.time.centisecond() < 10)
            Serial.print("0");
          Serial.print(gps.time.centisecond());
        } else {
          Serial.print("INVALID");
        }
        Serial.println();
#endif
        // Serial.println("available()");
      }
    }
    esp_task_wdt_reset(); // 定期喂狗
    delay(1000);
    // Serial.println("No GPS data");
  }
}

void buttonTask(void *pvParameters) {
  while (1) {
    button.tick();
    delay(10);
  }
}