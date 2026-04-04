#include <stdint.h>

struct Settings {
  int scale;
  uint32_t dmg_palette[4];
  uint8_t fullscreen;
  float volume;
  char key_a[64];
  char key_b[64];
  char key_start[64];
  char key_select[64];
  char key_up[64];
  char key_down[64];
  char key_left[64];
  char key_right[64];
  char key_pause[64];
  char key_screenshot[64];
  char last_rom[512];
  char recent_roms[10][512];
  int recent_rom_count;
};

extern struct Settings g_settings;

void settings_defaults(struct Settings* s);
void settings_load(struct Settings* s);
void settings_save(const struct Settings* s);
void settings_add_recent_rom(struct Settings* s, const char* path);
