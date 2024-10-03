#include "highlight.h"
#include <time.h>

char *C_FMATCH[] = {".c", ".h", ".cpp", ".hpp", NULL};
char *C_KEYWORDS[] = {
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|", "size_t",
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case", "extern", "default",
  NULL
};
char *C_PREPROC[] = {
  "include", "define", "undef", "ifdef", "if", "else", "elif", "endif",
  "error", "pragma", NULL
};

char *JS_FMATCH[] = {".js", ".jsx", ".mjs", ".cjs", ".ts", ".tsx", NULL};
char *JS_KEYWORDS[] = {
  "var|", "let|", "const|", "boolean|", "number|", "string|", "undefined|", "null|", "BigInt|", "Symbol|",
  "async", "await", "break", "case", "catch", "class", "continue", "debugger", "default", "delete", 
  "do", "else", "export", "extends", "finally", "for", "function", "if", "import", "in", "instanceof", 
  "new", "return", "super", "switch", "this", "throw", "try", "typeof", "void", "while", "with", "yield", 
  NULL
};

char *JSON_FMATCH[] = {".json", NULL};
char *JSON_KEYWORDS[] = {
  "true", "false", "null",
  NULL
};

char *SHELL_FMATCH[] = {".sh", ".bash", NULL};
char *SHELL_KEYWORDS[] = {
  "if", "fi", "else", "elif", "for", "do", "done", "while", "until", "case", "esac", "break", "continue", 
  "eval", "exec", "exit", "export", "function", "getopts", "let", "local", "read", "return", "set", 
  "shift", "test", "then", "times", "trap", "umask", "unset", 
  NULL
};

char *RUST_FMATCH[]={".rs", NULL};
char *RUST_KEYWORDS[] = {
  "bool|", "char|", "i8|", "i16|", "i32|", "i64|", "i128|", "u8|", "u16|", "u32|", "u64|", "u128|", 
  "f32|", "f64|", "str|",
  "as", "async", "await", "break", "const", "continue", "crate", "else", "enum", "extern", "false", 
  "fn", "for", "if", "impl", "in", "let", "loop", "match", "mod", "move", "mut", "pub", "ref", "return", 
  "self", "static", "struct", "super", "trait", "true", "type", "unsafe", "use", "where", "while", 
  NULL
};

char *PYTHON_FMATCH[] = {".py", NULL};
char *PYTHON_KEYWORDS[] = {
  "int|", "float|", "bool|", "str|", "list|", "dict|", "set|", "tuple|", "None|", "True|", "False|",
  "and", "as", "assert", "async", "await", "break", "class", "continue", "def", "del", 
  "elif", "else", "except", "finally", "for", "from", "global", "if", "import", "in", "is", 
  "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try", "while", "with", "yield", 
  NULL
};

char *ODIN_FMATCH[] = {".odin", NULL};
char *ODIN_KEYWORDS[] = {
  "int|", "float|", "bool|", "string|", "rune|", "any|", 
  "package", "import", "if", "else", "for", "when", "switch", "defer", "proc", "struct", "enum", 
  "map", "break", "continue", "return", "type", "true", "false", 
  NULL
};

char *ZIG_FMATCH[] = {".zig", NULL};
char *ZIG_KEYWORDS[] = {
  "i8|", "i16|", "i32|", "i64|", "i128|", "u8|", "u16|", "u32|", "u64|", "u128|", 
  "f16|", "f32|", "f64|", "bool|", "void|", "noreturn|",
  "fn", "pub", "const", "var", "if", "else", "while", "for", "switch", "comptime", "struct", 
  "enum", "break", "continue", "return", "defer", "true", "false", 
  NULL
};

char *CSHARP_FMATCH[] = {".cs", NULL};
char *CSHARP_KEYWORDS[] = {
  "int|", "float|", "double|", "decimal|", "bool|", "char|", "string|", "object|", 
  "abstract", "as", "base", "break", "case", "catch", "checked", "class", "const", "continue", 
  "default", "delegate", "do", "else", "enum", "event", "explicit", "extern", "false", "finally", 
  "fixed", "for", "foreach", "goto", "if", "implicit", "in", "interface", "internal", "is", 
  "lock", "namespace", "new", "null", "operator", "out", "override", "params", "private", 
  "protected", "public", "readonly", "ref", "return", "sealed", "sizeof", "stackalloc", 
  "static", "struct", "switch", "this", "throw", "true", "try", "typeof", "unchecked", 
  "unsafe", "using", "virtual", "void", "volatile", "while", 
  NULL
};

char *DART_FMATCH[] = {".dart", NULL};
char *DART_KEYWORDS[] = {
  "int|", "double|", "bool|", "String|", "num|", "void|",
  "abstract", "as", "assert", "async", "await", "break", "case", "catch", "class", "const", 
  "continue", "default", "deferred", "do", "dynamic", "else", "enum", "export", "extends", 
  "extension", "external", "factory", "false", "final", "finally", "for", "Function", "get", 
  "hide", "if", "implements", "import", "in", "interface", "is", "late", "library", "mixin", 
  "new", "null", "on", "operator", "part", "required", "rethrow", "return", "set", "show", 
  "static", "super", "switch", "sync", "this", "throw", "true", "try", "typedef", "var", 
  "void", "while", "with", "yield",
  NULL
};

char *JAVA_FMATCH[] = {".java", NULL};
char *JAVA_KEYWORDS[] = {
  "int|", "float|", "double|", "boolean|", "char|", "byte|", "short|", "long|", "void|",
  "abstract", "assert", "break", "case", "catch", "class", "const", "continue", "default", 
  "do", "else", "enum", "extends", "final", "finally", "for", "goto", "if", "implements", 
  "import", "instanceof", "interface", "native", "new", "null", "package", "private", 
  "protected", "public", "return", "static", "strictfp", "super", "switch", "synchronized", 
  "this", "throw", "throws", "transient", "try", "volatile", "while", "true", "false",
  NULL
};

char *KOTLIN_FMATCH[] = {".kt", ".kts", NULL};
char *KOTLIN_KEYWORDS[] = {
  "Int|", "Double|", "Float|", "Boolean|", "Char|", "Byte|", "Short|", "Long|", "String|", "Unit|",
  "abstract", "annotation", "as", "break", "by", "catch", "class", "companion", "const", 
  "constructor", "continue", "crossinline", "data", "delegate", "do", "else", "enum", "expect", 
  "external", "false", "final", "finally", "for", "fun", "get", "if", "import", "in", 
  "infix", "init", "inline", "inner", "interface", "internal", "is", "lateinit", "noinline", 
  "null", "object", "open", "operator", "out", "override", "package", "private", "protected", 
  "public", "reified", "return", "sealed", "set", "super", "suspend", "tailrec", "this", 
  "throw", "true", "try", "typealias", "typeof", "val", "var", "vararg", "when", "where", 
  "while",
  NULL
};

syntaxhl HLDB[] ={
  {
    "c/c++",
    C_FMATCH,
    C_KEYWORDS,
    C_PREPROC,
    '#',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_PUNCTUATION|HL_STRING|HL_COMMENT|HL_PREPROC
  },
  {
    "js",
    JS_FMATCH,
    JS_KEYWORDS,
    NULL,
    '\0',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_PUNCTUATION|HL_STRING|HL_COMMENT
  },
  {
    "json",
    JSON_FMATCH,
    JSON_KEYWORDS,
    NULL,
    '\0',
    NULL,
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION
  },
  {
    "shell",
    SHELL_FMATCH,
    SHELL_KEYWORDS,
    NULL,
    '\0',
    "#",
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION|HL_COMMENT
  },
  {
    "rust",
    RUST_FMATCH,
    RUST_KEYWORDS,
    NULL,
    '\0',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_PUNCTUATION|HL_STRING|HL_COMMENT
  },
  {
    "python",
    PYTHON_FMATCH,
    PYTHON_KEYWORDS,
    NULL,
    '\0',
    "#",
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION|HL_COMMENT
  },
  {
    "odin",
    ODIN_FMATCH,
    ODIN_KEYWORDS,
    NULL,
    '\0',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION|HL_COMMENT
  },
  {
    "zig",
    ZIG_FMATCH,
    ZIG_KEYWORDS,
    NULL,
    '\0',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION|HL_COMMENT
  },
  {
    "csharp",
    CSHARP_FMATCH,
    CSHARP_KEYWORDS,
    NULL,
    '\0',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION|HL_COMMENT
  },
  {
    "java",
    JAVA_FMATCH,
    JAVA_KEYWORDS,
    NULL,
    '\0',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION|HL_COMMENT
  },
  {
    "dart",
    DART_FMATCH,
    DART_KEYWORDS,
    NULL,
    '\0',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION|HL_COMMENT
  },
  {
    "kotlin",
    KOTLIN_FMATCH,
    KOTLIN_KEYWORDS,
    NULL,
    '\0',
    "//",
    HL_NUMBERS|HL_KEYWORD|HL_STRING|HL_PUNCTUATION|HL_COMMENT
  },
};

int HLDB_SIZE = sizeof(HLDB)/sizeof(HLDB[0]);

int hlTokentoColorIndex(unsigned char hl) {
  switch (hl) {
    case TK_NUMBER:
      return 206; //95;
    case TK_COMMENT:
      return 244; //37;
    case TK_KEYWORD1:
      return 202; //91;
    case TK_KEYWORD2:
      return 220; //93;
    case TK_KEYWORD3:
      return 142; //92;
    case TK_KEYWORD4:
      return 64; //94;
    case TK_PREPROC:
      return 71; //36;
    case TK_PUNCTUATION:
      return 166; //33;
    case TK_STRING:
      return 106; //32;
    case TK_MATCH:
      return 227; //43;
    case TK_NORMAL:
    default:
      return 250; //39;
  }
}
