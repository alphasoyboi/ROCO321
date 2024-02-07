// Tom Smale 10533488

#ifndef HSV_CONFIG_H
#define HSV_CONFIG_H

#define HSV_CONFIG_ITEMS 6

union HSVConfig {
    struct {
        int lh, hh;
        int ls, hs;
        int lv, hv;
    };
    int data[6];
};

#define MAX_H 360
#define MAX_SV 1000
#define HSV_CONFIG_FILEPATH "hsv_conf.txt"

HSVConfig loadConfig(const char* filepath);
void saveConfig(const char* filepath, const HSVConfig& config);

#endif // SETTINGS_H
