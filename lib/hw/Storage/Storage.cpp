#include "Storage.h"

Storage::Storage() {}

void Storage::begin() {
    // Buka namespace NVS. false = read/write mode.
    prefs_.begin(NS, false);
}

void Storage::saveBrightness(uint8_t percent) {
    if (percent > 100) percent = 100;
    prefs_.putUChar(KEY, percent);
}

uint8_t Storage::loadBrightness() {
    return prefs_.getUChar(KEY, 50); // default 50% jika belum ada
}
