#pragma once

/// Declare Macros
#define NUM_LEDS 42
/// 罗盘状态
#define STATE_LOST_BEARING 0
#define STATE_WAIT_GPS 1
#define STATE_SHOW_FRAME 2
#define STATE_CONNECT_WIFI 3
#define STATE_SERVER_COLORS 4
#define STATE_SERVER_INDEX 5
#define STATE_SERVER_WIFI 6
#define STATE_SERVER_SPAWN 7
#define STATE_SERVER_INFO 8
#define STATE_GAME_COMPASS 9
#define STATE_HOTSPOT 10

/// 引脚定义
#define DATA_PIN 6
#define CALIBRATE_PIN 9
#define GPS_EN_PIN 0