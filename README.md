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
| **Screen Split**                | Has split-screen view supported        |


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
| **Kotlin**      | `.kt`, `.kts`       |


## **Getting Started**

### **Setup & Installation**
1. Clone the repository to your local machine:
    ```bash
    git clone https://github.com/therealsunx/trvim
    ```

2. Make sure you have dependencies `cmake` & `make` installed on your system:
    1. For Linux, extra dependency xclip is required to interact with system clipboard.
        ```bash
        sudo apt install cmake make xclip
        ```
    2. For MacOs
        ```bash
        brew install cmake make
        ```
    3. For Windows
        - __Cmake__ : Download from [here](https://cmake.org/download/)._
        - __Make__ : Download from [here](https://gnuwin32.sourceforge.net/packages/make.htm).

3. Make a build directory and navigate to it. This is where build files will go:   
    ```bash
    mkdir build
    cd build
    ```

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

__Now Follow this [User Guide](https://github.com/therealsunx/trvim/blob/main/UserGuide.md)__

__Get the Cheat Sheet [here](https://github.com/therealsunx/trvim/blob/main/CheatSheet.md).__

___Use it just like you use vim.___
