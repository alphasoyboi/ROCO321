// Tom Smale 10533488

#ifndef HSV_CONFIG_H
#define HSV_CONFIG_H

#define HSV_CONFIG_ITEMS 6

union HSVConfig {
    struct {
        int lh, ls, lv;
        int hh, hs, hv;
    };
    int data[6];
};

HSVConfig loadConfig(const char* filepath);
void saveConfig(const char* filepath, const HSVConfig& config);

#endif // SETTINGS_H
