#pragma once

enum settingsflags {
  REL_LINENUM = 0x1,
  EXPAND_TABS = 0x2,
};

typedef struct {
  int scrollpadding, tabwidth;
  int flags;
} settingsType;
