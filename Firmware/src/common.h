#pragma once

/// @brief 罗盘状态
typedef enum {
  STATE_LOST_BEARING = 0,   // 丢失方位
  STATE_WAIT_GPS = 1,       // 等待GPS信号
  STATE_COMPASS = 2,        // 罗盘工作状态
  STATE_CONNECT_WIFI = 3,   // 连接WiFi中
  STATE_SERVER_COLORS = 10, // 网页颜色选项卡激活
  STATE_SERVER_INDEX,       // 控制指定索引的frame
  STATE_SERVER_WIFI,        // 网页WiFi选项卡激活
  STATE_SERVER_SPAWN,       // 网页出生点选项卡激活
  STATE_SERVER_INFO,        // 网页信息选项卡激活
  STATE_GAME_COMPASS = 100, // 游戏联动罗盘中
  STATE_HOTSPOT = 200       // WiFi热点状态中
} CompassState;

/// @brief 罗盘工作模式
typedef enum {
  LocationCompass, // 地点指针
  NorthCompass,    // 指南针
} CompassType;

/// @brief 坐标
typedef struct {
  float latitude;
  float longitude;
} Location;