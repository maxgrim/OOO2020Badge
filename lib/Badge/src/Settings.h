#ifndef SETTINGS_H_SEEN
#define SETTINGS_H_SEEN

struct Settings {
  uint8_t bootMode;
  uint8_t isAdmin;
};

void settingsSetup(size_t eepromSize, size_t eepromStartAddress);
Settings settingsGetSettings();
void settingsSetSettings(Settings settings);

#endif