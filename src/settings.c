#include "gbemu/settings.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir((p), 0755)
#endif

#include "tomlc17.h"

struct Settings g_settings;

void settings_defaults(struct Settings* s) {
  s->scale = 3;
  s->fullscreen = false;
  s->dmg_palette[0] = 0x9a9e3f;
  s->dmg_palette[1] = 0x496b22;
  s->dmg_palette[2] = 0x0e450b;
  s->dmg_palette[3] = 0x1b2a09;
  s->volume = 1.0F;
  s->mute = false;
  (void)snprintf(s->key_a, sizeof(s->key_a), "S");
  (void)snprintf(s->key_b, sizeof(s->key_b), "A");
  (void)snprintf(s->key_start, sizeof(s->key_start), "Return");
  (void)snprintf(s->key_select, sizeof(s->key_select), "Left Shift");
  (void)snprintf(s->key_up, sizeof(s->key_up), "Up");
  (void)snprintf(s->key_down, sizeof(s->key_down), "Down");
  (void)snprintf(s->key_left, sizeof(s->key_left), "Left");
  (void)snprintf(s->key_right, sizeof(s->key_right), "Right");
  (void)snprintf(s->key_pause, sizeof(s->key_pause), "P");
  (void)snprintf(s->key_screenshot, sizeof(s->key_screenshot), "F2");
  s->last_rom[0] = '\0';
  s->recent_rom_count = 0;
}

static void config_path(char* buf, size_t len) {
#ifdef _WIN32
  const char* appdata = getenv("APPDATA");
  if (!appdata) appdata = ".";
  (void)snprintf(buf, len, "%s\\gbemu\\settings.toml", appdata);
#else
  const char* home = getenv("HOME");
  if (!home) {
    home = ".";
  }
  (void)snprintf(buf, len, "%s/.config/gbemu/settings.toml", home);
#endif
}

static void mkdir_config_dir(void) {
#ifdef _WIN32
  const char* appdata = getenv("APPDATA");
  if (!appdata) return;
  char dir[512];
  (void)snprintf(dir, sizeof(dir), "%s\\gbemu", appdata);
  MKDIR(dir);
#else
  const char* home = getenv("HOME");
  if (!home) {
    return;
  }
  char dir[512];
  (void)snprintf(dir, sizeof(dir), "%s/.config", home);
  MKDIR(dir);
  (void)snprintf(dir, sizeof(dir), "%s/.config/gbemu", home);
  MKDIR(dir);
#endif
}

static void load_key(const toml_datum_t tbl, const char* name, char* dst,
                     size_t len) {
  toml_datum_t v = toml_get(tbl, name);
  if (v.type == TOML_STRING) {
    (void)snprintf(dst, len, "%s", v.u.s);
  }
}

static void write_str(FILE* fp, const char* s) {
  (void)fputc('"', fp);
  for (; *s; s++) {
    if (*s == '"' || *s == '\\') {
      (void)fputc('\\', fp);
    }
    (void)fputc(*s, fp);
  }
  (void)fputc('"', fp);
}

void settings_save(const struct Settings* s) {
  mkdir_config_dir();
  char path[1024];
  config_path(path, sizeof(path));
  FILE* fp = fopen(path, "w");
  if (!fp) {
    return;
  }

  (void)fprintf(fp, "[display]\n");
  (void)fprintf(fp, "scale = %d\n", s->scale);
  (void)fprintf(fp, "fullscreen = %s\n", (int)s->fullscreen ? "true" : "false");
  (void)fprintf(fp, "dmg_palette = [\"%06x\", \"%06x\", \"%06x\", \"%06x\"]\n",
                s->dmg_palette[0], s->dmg_palette[1], s->dmg_palette[2],
                s->dmg_palette[3]);
  (void)fprintf(fp, "\n[audio]\n");
  (void)fprintf(fp, "volume = %.2f\n", (double)s->volume);
  (void)fprintf(fp, "mute = %s\n", (int)s->mute ? "true" : "false");
  (void)fprintf(fp, "\n[controls]\n");
  (void)fprintf(fp, "a = ");
  write_str(fp, s->key_a);
  (void)fprintf(fp, "\nb = ");
  write_str(fp, s->key_b);
  (void)fprintf(fp, "\nstart = ");
  write_str(fp, s->key_start);
  (void)fprintf(fp, "\nselect = ");
  write_str(fp, s->key_select);
  (void)fprintf(fp, "\nup = ");
  write_str(fp, s->key_up);
  (void)fprintf(fp, "\ndown = ");
  write_str(fp, s->key_down);
  (void)fprintf(fp, "\nleft = ");
  write_str(fp, s->key_left);
  (void)fprintf(fp, "\nright = ");
  write_str(fp, s->key_right);
  (void)fprintf(fp, "\npause = ");
  write_str(fp, s->key_pause);
  (void)fprintf(fp, "\nscreenshot = ");
  write_str(fp, s->key_screenshot);
  (void)fputc('\n', fp);
  (void)fprintf(fp, "\n[paths]\n");
  (void)fprintf(fp, "last_rom = ");
  write_str(fp, s->last_rom);
  (void)fputc('\n', fp);
  (void)fprintf(fp, "\n[ui]\n");
  (void)fprintf(fp, "recent_roms = [");
  for (int i = 0; i < s->recent_rom_count; i++) {
    if (i > 0) {
      (void)fprintf(fp, ", ");
    }
    write_str(fp, s->recent_roms[i]);
  }
  (void)fprintf(fp, "]\n");
  (void)fclose(fp);
}

void settings_load(struct Settings* s) {
  settings_defaults(s);
  mkdir_config_dir();
  char path[1024];
  config_path(path, sizeof(path));
  FILE* fp = fopen(path, "r");
  if (!fp) {
    settings_save(s);
    return;
  }
  toml_result_t res = toml_parse_file(fp);
  (void)fclose(fp);
  if (!res.ok) {
    toml_free(res);
    return;
  }
  toml_datum_t root = res.toptab;
  toml_datum_t tbl;
  toml_datum_t v;
  tbl = toml_get(root, "display");
  if (tbl.type == TOML_TABLE) {
    v = toml_get(tbl, "scale");
    if (v.type == TOML_INT64) {
      s->scale = (int)v.u.int64;
    }
    v = toml_get(tbl, "fullscreen");
    if (v.type == TOML_BOOLEAN) {
      s->fullscreen = v.u.boolean;
    }
    v = toml_get(tbl, "dmg_palette");
    if (v.type == TOML_ARRAY && v.u.arr.size == 4) {
      for (int i = 0; i < 4; i++) {
        if (v.u.arr.elem[i].type == TOML_STRING) {
          s->dmg_palette[i] = (uint32_t)strtoul(v.u.arr.elem[i].u.s, NULL, 16);
        }
      }
    }
  }
  tbl = toml_get(root, "audio");
  if (tbl.type == TOML_TABLE) {
    v = toml_get(tbl, "volume");
    if (v.type == TOML_FP64) {
      s->volume = (float)v.u.fp64;
    }
    v = toml_get(tbl, "mute");
    if (v.type == TOML_BOOLEAN) {
      s->mute = v.u.boolean;
    }
  }
  tbl = toml_get(root, "controls");
  if (tbl.type == TOML_TABLE) {
    load_key(tbl, "a", s->key_a, sizeof(s->key_a));
    load_key(tbl, "b", s->key_b, sizeof(s->key_b));
    load_key(tbl, "start", s->key_start, sizeof(s->key_start));
    load_key(tbl, "select", s->key_select, sizeof(s->key_select));
    load_key(tbl, "up", s->key_up, sizeof(s->key_up));
    load_key(tbl, "down", s->key_down, sizeof(s->key_down));
    load_key(tbl, "left", s->key_left, sizeof(s->key_left));
    load_key(tbl, "right", s->key_right, sizeof(s->key_right));
    load_key(tbl, "pause", s->key_pause, sizeof(s->key_pause));
    load_key(tbl, "screenshot", s->key_screenshot, sizeof(s->key_screenshot));
  }
  tbl = toml_get(root, "paths");
  if (tbl.type == TOML_TABLE) {
    v = toml_get(tbl, "last_rom");
    if (v.type == TOML_STRING) {
      (void)snprintf(s->last_rom, sizeof(s->last_rom), "%s", v.u.s);
    }
  }
  tbl = toml_get(root, "ui");
  if (tbl.type == TOML_TABLE) {
    v = toml_get(tbl, "recent_roms");
    if (v.type == TOML_ARRAY) {
      int n = v.u.arr.size < 10 ? v.u.arr.size : 10;
      s->recent_rom_count = n;
      for (int i = 0; i < n; i++) {
        if (v.u.arr.elem[i].type == TOML_STRING) {
          (void)snprintf(s->recent_roms[i], sizeof(s->recent_roms[i]), "%s",
                         v.u.arr.elem[i].u.s);
        }
      }
    }
  }
  toml_free(res);
}

void settings_remove_recent_rom(struct Settings* s, const char* path) {
  for (int i = 0; i < s->recent_rom_count; i++) {
    if (strcmp(s->recent_roms[i], path) == 0) {
      for (int j = i; j < s->recent_rom_count - 1; j++) {
        memcpy(s->recent_roms[j], s->recent_roms[j + 1], 512);
      }
      s->recent_rom_count--;
      return;
    }
  }
}

void settings_add_recent_rom(struct Settings* s, const char* path) {
  for (int i = 0; i < s->recent_rom_count; i++) {
    if (strcmp(s->recent_roms[i], path) == 0) {
      for (int j = i; j < s->recent_rom_count - 1; j++) {
        memcpy(s->recent_roms[j], s->recent_roms[j + 1], 512);
      }
      s->recent_rom_count--;
      break;
    }
  }
  if (s->recent_rom_count < 10) {
    s->recent_rom_count++;
  }
  for (int i = s->recent_rom_count - 1; i > 0; i--) {
    memcpy(s->recent_roms[i], s->recent_roms[i - 1], 512);
  }
  (void)snprintf(s->recent_roms[0], 512, "%s", path);
}
