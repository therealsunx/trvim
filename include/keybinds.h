#pragma once

#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKeys {

  CTRL_C = CTRL_KEY('c'),
  ESCAPE = '\x1b',

  CTRL_A = CTRL_KEY('a'),
  CTRL_F = CTRL_KEY('f'),

  RETURN = '\r',
  CTRL_X = CTRL_KEY('x'),
  CTRL_Z = CTRL_KEY('z'),

  BACKSPACE = 127,
  CTRL_H = CTRL_KEY('h'),

  ARROW_LEFT=1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,

  HOME_KEY,
  END_KEY,
  DEL_KEY,

  PAGE_UP,
  PAGE_DOWN,
};
