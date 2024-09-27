#pragma once

#define DEF_SETTINGS { 8, 2, REL_LINENUM}

enum settingsflags {
  REL_LINENUM = 0x1,
  EXPAND_TABS = 0x2,
};

typedef struct {
  int scrollpadding, tabwidth;
  int flags;
} settings_t;
