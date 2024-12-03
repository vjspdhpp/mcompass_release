#include <Preferences.h>

#include "func.h"

void saveHomeLocation(Location location) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  preferences.putFloat("targetLat", location.latitude);
  preferences.putFloat("targetLon", location.longitude);
  preferences.end();
}

void getHomeLocation(Location &location) {
  Preferences preferences;
  preferences.begin("mcompass", false);
  if (!preferences.isKey("targetLat") || !preferences.isKey("targetLon")) {
    preferences.end();
    return;
  }
  location.latitude = preferences.getFloat("targetLat");
  location.longitude = preferences.getFloat("targetLon");
  preferences.end();
}