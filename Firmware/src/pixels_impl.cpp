#include <FastLED.h>

#include "compass_frames.h"
#include "func.h"
#include "macro_def.h"

CRGB leds[NUM_LEDS];

void lostBearing() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = frames[27][i];
  }
  FastLED.show();
}

void theNether() {
  // 当前帧索引
  static int curIndex = 0;
  // 目标帧索引
  static int targetIndex = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = frames[curIndex][i];
  }
  FastLED.show();
  if (curIndex == targetIndex) {
    targetIndex = random(0, 27);
  } else {
    if (curIndex < targetIndex) {
      curIndex += 1;
    } else {
      curIndex -= 1;
    }
  }
  if (curIndex < 0)
    curIndex = 27;
  if (curIndex > 27) {
    curIndex = 0;
  }
}

void showFrame(int index, int overrideColor) {
  static uint32_t color = 0;
  color = overrideColor;
  for (int i = 0; i < NUM_LEDS; i++) {
    // TODO("这里应该使用一个指针数据的模板来设置颜色")
    leds[i] = frames[index][i] == 0xff1414 || frames[index][i] == 0xcb1a1a ||
                      frames[index][i] == 0xbe1515
                  ? color
                  : frames[index][i];
  }
  FastLED.show();
}

void showFrameByAzimuth(float azimuth) {
  if (azimuth < 0 || azimuth > 360) {
    // 不响应不合法的方位角
    return;
  }
  // 将方位角分为28个区间
  // 0~14, 算作第一个区间
  // 14~42, 算作第二个区间
  // 42~70, 算作第三个区间
  // ...
  // 346~360, 算作第28个区间
  int index = 0;
  if (azimuth >= 346) {
    index = 0;
  } else if (azimuth >= 0 && azimuth < 14) {
    index = 0;
  } else {
    index = (int)((azimuth - 14) / ((360 - 14) / 27.0)) + 1;
  }
  Serial.printf("showFrameByAzimuth: relative azimuth=%f, index=%d\n", azimuth,
                index);
  showFrame(index);
}

void showFrameByBearing(float bearing, int azimuth) {
  // 当前方位角对应的索引
  int aIndex = (int)(azimuth / 360.0 * 28.0);
  // 目标方位角对应的索引
  int bIndex = (int)(bearing / 360.0 * 28.0);
  // 计算差值
  // int index = aIndex - bIndex;
  Serial.printf("showFrameByBearing: bearing=%f azimuth=%d \n", bearing,
                azimuth);
  float degree = bearing - azimuth;
  if (degree < 0) {
    degree += 360;
  }
  showFrameByAzimuth(degree);
  // if (index < 0) {
  //   index = 28 + index;
  // } else if (index > 27) {
  //   index = index - 27;
  // }
  // showFrame(index);
}

void showFrameByLocation(float latA, float lonA, float latB, float lonB,
                         int azimuth) {
  // Serial.printf("showFrameByLocation:azimuth=%d\n", azimuth);
  float bearing = calculateBearing(latA, lonA, latB, lonB);
  showFrameByBearing(bearing, azimuth);
}

void showHotspot() {
  FastLED.clear();
  static int blink = 0;
  if (blink == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Yellow);
  } else {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  }
  FastLED.show();
  delay(1000);
  blink = 1 - blink;
}

void showConnectingWifi() {
  FastLED.clear();
  static int index = 0;
  for (int i = 0; i < index; i++) {
    leds[i] = CRGB::Green;
  }
  index += 1;
  if (index > NUM_LEDS) {
    index = 0;
  }
  FastLED.show();
  delay(100);
}

void showSolid(int color) {
  fill_solid(leds, NUM_LEDS, CRGB(color));
  FastLED.show();
}

void showServerColors() {
  // for (int i = 0; i < NUM_LEDS; i++) {
  //   leds[i] = frames[29][i];
  // }
  // FastLED.show();

  // 彩虹色
  static int hue = 0;
  fill_rainbow(leds, NUM_LEDS, hue, 1);
  FastLED.show();
  hue += 1;
  delay(16);
}

static void showBouncing(int color) {
  FastLED.clear();
  static int indexes[] = {0, 2, 6, 11, 16, 21, 26, 31, 35, 40};
  static int dir = 1;
  static int index = 0;

  if (index >= 9) {
    dir = -1;
  } else if (index <= 0) {
    dir = 1;
  }

  for (int i = 0; i < 10; i++) {
    // Calculate distance from current index (0-9)
    float distance = abs(i - index);
    // Use gaussian/normal distribution formula
    float brightness = 255 * exp(-(distance * distance) /
                                 (2 * 1.5)); // sigma=1.5 controls spread

    leds[indexes[i]] = color;
    // Apply calculated brightness
    if (distance > 0) {
      leds[indexes[i]].fadeToBlackBy(255 - (int)brightness);
    }
  }
  index += dir;
  FastLED.show();
}

void showServerWifi() {
  showBouncing(CRGB::Green);
  delay(100);
}

void showServerSpawn() {
  showBouncing(CRGB::Blue);
  delay(100);
}

void showServerInfo() {
  showBouncing(CRGB::Red);
  delay(100);
}
