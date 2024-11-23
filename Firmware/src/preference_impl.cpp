#include <Preferences.h>

#include "func.h"

void saveHomeLocation(float latitude, float longitude) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  preferences.putFloat("targetLat", latitude);
  preferences.putFloat("targetLon", longitude);
  preferences.end();
}

void getHomeLocation(float &latitude, float &longitude) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  if (!preferences.isKey("targetLat") || !preferences.isKey("targetLon")) {
    preferences.end();
    return;
  }
  latitude = preferences.getFloat("targetLat");
  longitude = preferences.getFloat("targetLon");
  preferences.end();
}