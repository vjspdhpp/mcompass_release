#include <FastLED.h>
#include <OneButton.h>
#include <QMC5883LCompass.h>
#include <TinyGPSPlus.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "func.h"
#include "macro_def.h"

// 调试输出控制宏
#define DEBUG_DISPLAY 0

static const char *TAG = "MAIN";
extern CRGB leds[NUM_LEDS];
extern QMC5883LCompass compass;
TinyGPSPlus gps;
HardwareSerial MySerial(0);  // Create a new HardwareSerial class.
OneButton button(CALIBRATE_PIN, true);

// 目标位置
float targetLat = 43.0;
float targetLon = 126.0;

// 当前位置
float currentLat = 31.0;
float currentLon = 117.0;

// 状态
uint8_t compassState = STATE_LOST_BEARING;
// 模式
uint8_t mode = 0;
// 是否失去GPS信号
bool locationReady = true;

void setup() {
  // 延时,用于一些特殊情况下能够重新烧录
  delay(800);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  long t = millis();
  uint32_t color = 0;
  while (millis() - t < 2500) {
    fill_rainbow(leds, NUM_LEDS, color);
    FastLED.show();
    color++;
  }
  Serial.begin(115200);
  // 配置GPS串口
  MySerial.begin(9600, SERIAL_8N1, RX, TX);
  // 创建显示任务
  xTaskCreate(displayTask, "displayTask", 4096, NULL, 2, NULL);
  // 创建位置任务
  xTaskCreate(locationTask, "locationTask", 4096, NULL, 2, NULL);
  // 配置校准引脚状态
  pinMode(CALIBRATE_PIN, INPUT_PULLUP);
  pinMode(GPS_EN_PIN, OUTPUT);
  digitalWrite(GPS_EN_PIN, HIGH);
  // 获取目标位置
  getHomeLocation(targetLat, targetLon);
  // 初始化罗盘
  compass.init();
  // 校准引脚被按下时候进行校准
  if (digitalRead(CALIBRATE_PIN) == LOW) {
    calibrateCompass();
  }
  // 结束校准阶段, 开启GPS, 等待GPS信号
  digitalWrite(GPS_EN_PIN, LOW);
  compassState = STATE_WAIT_GPS;
  Serial.println("loop.");
  button.attachLongPressStart(
      [](void *scope) {
        // 设置当前地点为Home
        // 检查GPS状态
        if (locationReady) {
          saveHomeLocation(currentLat, currentLon);
          targetLat = currentLat;
          targetLon = currentLon;
          locationReady = false;
          Serial.println("Set Home");
        } else {
          Serial.println("Can't set home");
        }
      },
      &button);
  // 插入电脑情况下会启动服务器;
  setupServer();
}

void loop() {
  button.tick();
  delay(10);
}

void displayTask(void *pvParameters) {
  while (1) {
    switch (compassState) {
      case STATE_LOST_BEARING: {
        // 校准罗盘中
        lostBearing();
        break;
      }
      case STATE_WAIT_GPS: {
        // 等待GPS数据
        theNether();
        break;
      }
      case STATE_SHOW_FRAME: {
        Serial.println("STATE_SHOW_FRAME");
        compass.read();
        float azimuth = compass.getAzimuth();
        if (azimuth < 0) {
          azimuth += 360;
        }
        if (mode == 0) {
          showFrameByLocation(targetLat, targetLon, currentLat, currentLon,
                              azimuth);
        } else {
#if DEBUG_DISPLAY
          Serial.printf("Azimuth = %d\n", azimuth);
#endif
          showFrameByAzimuth(azimuth);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        break;
      }
      case STATE_CONNECT_WIFI:
        showConnectingWifi();
        break;
      case STATE_SERVER_COLORS: {
        // showServerColors();
        vTaskDelay(pdMS_TO_TICKS(50));
      } break;
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
        showHotspot();
        break;
      }
      default:
        vTaskDelay(pdMS_TO_TICKS(50));
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
    while (MySerial.available() > 0) {
      char t = MySerial.read();
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
          currentLat = static_cast<float>(gps.location.lat());
          currentLon = static_cast<float>(gps.location.lng());
          compassState = STATE_SHOW_FRAME;
          locationReady = false;
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
          if (gps.time.hour() < 10) Serial.print("0");
          Serial.print(gps.time.hour());
          Serial.print(":");
          if (gps.time.minute() < 10) Serial.print("0");
          Serial.print(gps.time.minute());
          Serial.print(":");
          if (gps.time.second() < 10) Serial.print("0");
          Serial.print(gps.time.second());
          Serial.print(".");
          if (gps.time.centisecond() < 10) Serial.print("0");
          Serial.print(gps.time.centisecond());
        } else {
          Serial.print("INVALID");
        }
        Serial.println();
#endif
        // Serial.println("available()");
      }
    }
    esp_task_wdt_reset();  // 定期喂狗
    vTaskDelay(pdMS_TO_TICKS(1000));
    // Serial.println("No GPS data");
  }
}