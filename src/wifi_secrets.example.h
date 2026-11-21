// Copy this file to wifi_secrets.h (same folder) and fill in your networks.
// wifi_secrets.h is gitignored, so your passwords are never committed.
// Networks are tried in order: put your home WiFi first and your phone hotspot second.
#pragma once

struct WifiNet { const char* ssid; const char* pass; };

static const WifiNet WIFI_NETS[] = {
    {"HomeWifiName", "home-password"},
    {"PhoneHotspotName", "hotspot-password"},
};
#define WIFI_NET_COUNT (sizeof(WIFI_NETS) / sizeof(WIFI_NETS[0]))
