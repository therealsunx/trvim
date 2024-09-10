#define CTRL_KEY(k) ((k) & 0x1f)

enum editorKeys {

  CTRl_Z = CTRL_KEY('z'),

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
