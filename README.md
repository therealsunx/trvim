# **trvim** - terminal text editor
<p align="center">
    <img src="trvim.png" />
</p>

## Table of Contents
- [Overview](#overview)
- [Key Features](#key-features)
- [Syntax Highlighting](#syntax-highlighting)
  - [Supported Programming Languages](#supported-programming-languages)
  - [Syntax Flags](#syntax-flags)
- [Getting Started](#getting-started)
  - [Installation](#installation)
- [Customization & Test](#customization--test)
- [License](#license)


### **Overview**
**trvim** - is a text editor built in C, it is fast and is similiar in operation to vim. It uses the power of non-canonical terminal to provide quick, robust and easy operation. It supports features like syntax highlighting, auto-indentation, cross-platform use, zero-mouse-need movement bindings, etc.

Install and use it. Installation method and User Guide is provided below.
___Enjoy !!___


## **Key Features**
Here are the key features supported by this editor.
| Feature                        | Description                                                                                             |
|---------------------------------|---------------------------------------------------------------------------------------------------------|
| **Syntax Highlighting**         | Color-coded syntax for multiple programming languages to enhance code readability and debugging.         |
| **Cross-platform Support**      | Fully functional across major platforms (Linux, Windows, macOS).                                         |
| **Lightweight & Fast**          | Optimized for speed and minimal memory usage, even when handling large files.                           |
| **Multiple Language Support**   | Supports a wide range of programming languages with easy language switching.                             |
| **Faster Navigation**           | Has easy to use keybinds (similiar to vim), with zero-mouse requirement, allowing quicker navigation.   |


## **Syntax Highlighting**
The editor's syntax highlighting engine is designed to provide clear, readable code with visually distinct colors for different parts of your codes like keywords, punctuations, comments, strings, etc.

Syntax highlighting is done by built-in highlight database, so no external plugin is used for it. It only has minimal required highlighting required for any language, which prevents the editor from being littered with errors, warnings and whatnot.


### **Supported Programming Languages**
The editor supports syntax highlighting for the following languages:

| Language        | Extension(s)        |
|-----------------|---------------------|
| **C/C++**       | `.c`, `.cpp`, `.h`, `.hpp`  |
| **JavaScript**  | `.js`, `.mjs`, `.cjs`, `.ts`, `.jsx`, `.tsx` |
| **Python**      | `.py`               |
| **Rust**        | `.rs`               |
| **Shell Script**| `.sh`, `.bash`      |
| **JSON**        | `.json`             |
| **Odin**        | `.odin`             |
| **Zig**         | `.zig`              |
| **C#**          | `.cs`               |
| **Go**          | `.go`               |
| **Java**        | `.java`             |
| **Dart**        | `.dart`             |
| **Go**          | `.kt`, `.kts`       |


### **Syntax Flags**
Syntax highlighting is controlled via several flags that allow for customizable styling:

| Flag                 | Description                                                                |
|----------------------|----------------------------------------------------------------------------|
| `HL_NUMBERS`         | Highlights numeric literals (e.g., `123`, `0xFF`).                          |
| `HL_PREPROC`         | Highlights preprocessor-like directives (e.g., `#include`, `import`).       |
| `HL_KEYWORD`         | Highlights language keywords (e.g., `for`, `if`, `while`).                  |
| `HL_PUNCTUATION`     | Highlights punctuation and symbols (e.g., `{`, `}`, `;`).                   |
| `HL_COMMENT`         | Highlights single-line and multi-line comments.                             |
| `HL_STRING`          | Highlights string literals.                                                 |

_If you want to customize your own version of **trvim**, then you can make use of this information to set up highlighting rules in `highlight.c`. Otherwise ignore this information._
_The colors for syntax highlighting are 8bit ANSI color codes. So if you want to customize colors follow this [wikipedia article on ANSI codes.](https://en.wikipedia.org/wiki/ANSI_escape_code)_


## **Getting Started**

### **Setup & Installation**
1. Clone the repository to your local machine:
    ```bash
    git clone https://github.com/therealsunx/trvim
    ```
2. Make a build directory and navigate to it. This is where build files will go:   
    ```bash
    mkdir build
    cd trvim
    ```
3. Make sure you have cmake installed on your system:
    ```bash
    cmake --version
    ```
    _If you don't have it, then install it from [here](https://cmake.org/download/)._
4. Run cmake in project directory form build directory.
    ```bash
    cmake ..
    ```
    _This will have completed the setup phase. You can modify code without any issues._
5. In order to build the project, you can run `make`.
    If you want to ___install it in your system___, to access it from anywhere, then run following:
    ```bash
    make install
    ```
3. Launch the editor from your terminal:
    ```bash
    trvim filename.c
    #or
    trvim
    ```

__Follow this [User Guide](https://github.com/therealsunx/trvim/blob/main/UserGuide.md)__

___Use it just like you use vim.___
