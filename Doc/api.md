---
outline: deep
---

# API

path | 类型 |描述 | 参数
-|-|-|-
/ip | GET | 获取当前设备IP | 无
/setIndex | POST |调试用, 直接显示第N帧 | index: Int
/info | GET |获取设备基本信息 | 无
/wifi | GET |获取设备保存的WiFi | 无
/wifi | GET |获取设备保存的WiFi | 无
/spawn | GET |获取当前出生点 | 无
/spawn | POST |设置当前出生点 | latitude:Float,longitude:Float
/setColor | POST |调试用,所有灯珠显示该颜色 | color: String, eg: #ff5252
/setAzimuth | POST | 设置方位角, Mod用的就是这个接口 | azimuth: Float
/setWiFi | POST | 设置WiFi | ssid: String, password: String
