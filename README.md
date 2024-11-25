# 一个现实世界中的Minecraft罗盘
![Index](./Doc/public/MCompass.png)

Front|Bottom
-|-
![Front](./Doc/public/FrontPCB.png)|![Bottom](./Doc/public/BottomPCB.png)

## 声明
* **Minecraft游戏素材版权均归微软所有**, 所以本项目不提供游戏中罗盘的图片素材
    * 面板文件仅提供外形, 像素块已绘制好
    * 罗盘取模原始图片恕不提供
    * PCB背面丝印的标准银河字母字体版权不明确. 如果有任何借此项目进行商业的行为, **请删除背面标准银河字母**

## 编译说明
固件使用PlatformIO平台Arduino框架编写, 已将依赖库迁移至本地lib文件夹下.
PlatformIO的安装方式请自行搜索;

`Firmware/assets`文件夹内附带了一个**extract_pixels.py**脚本, 会检测`Firmware/assets`文件夹内`compass{id}.bmp`的10x5图像, 根据LED灯珠排布取模,生成对应的头文件.

服务端程序使用next.js. 安装好node.js后,进入Server文件夹执行`npm i`安装依赖.

执行`npm run build`拷贝生成的`Server/out`文件夹内容到`Firmware/data`文件夹下,使用PlatformIO自带的`Build Filesystem Image`和`Upload Filesystem Image`指令上传到设备.

## 功能说明
* 首次插上电脑启动会创建一个`The Lost Compass`的热点, 连接后打开浏览器输入[esp32.local](http://esp32.local), 输入WiFi和密码进行配网,配网完成后装置会重启;
* 当罗盘插上电脑后开机, 此时会启动后台服务, 使用[esp32.local](http://esp32.local)访问后台;
* 开机后显示彩虹图案时按住按钮会进入罗盘校准模式, 请举着罗盘飞来飞去, 尽可能多的让罗盘在各个方向旋转.
* 在室外开阔环境下才能够有GPS信号, 没有GPS信号指针会乱转的.
* 在有GPS信号的情况下, 长按按钮可以设置当前地点为新的出生点.

## API
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

## 材料说明
* PCB板厚选择1.0mm, 黑色;
* 面板选择1.0mm 半透黑色亚克力,无需背胶;
* 匀光材料选择PET LGT075J,无需背胶;
* 电池选择 213455 500MAH
* 滚花螺母 M2*3*3.2
* 螺丝 M2*4 内六角

## 模型
[MakerWorld](https://makerworld.com.cn/zh/models/667420#profileId-611642)
![MakerWorld CN](./Doc/public/makerworldcn.jpg)

## PCB工程及面板文件
[PCB工程文件和面板](https://oshwhub.com/chaosgoo/wcompass)

## 参考资料
[使用GPS坐标来计算距离和方位角 by 蓝色的飘漂](https://johnnyqian.net/blog/gps-locator.html)
