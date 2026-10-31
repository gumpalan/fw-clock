#include "Topics.h"
#include <cstring>
#include <cstdio>

namespace logic {

Topics::Topics(const char* device_id) {
    strncpy(device_id_, device_id, sizeof(device_id_) - 1);
    device_id_[sizeof(device_id_) - 1] = '\0';

    build(status_,       "gumpalan/", "/status");
    build(sensor_state_, "gumpalan/", "/sensor/state");
    build(lamp_state_,   "gumpalan/", "/lamp/state");
    build(lamp_set_,     "gumpalan/", "/lamp/set");

    // Discovery: gumpalan/discovery/<id>
    snprintf(discovery_, sizeof(discovery_), "gumpalan/discovery/%s", device_id_);
}

void Topics::build(char* out, const char* prefix, const char* suffix) {
    snprintf(out, TOPIC_MAX, "%s%s%s", prefix, device_id_, suffix);
}

} // namespace logic
