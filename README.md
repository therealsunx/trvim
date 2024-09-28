# **trvim** - terminal text editor
<p align="center">
    <img src="trvim.png" />
</p>

### **Overview**
**trvim** - is a text editor built in C, it is fast and is similiar in operation to vim. It uses the power of non-canonical terminal to provide quick, robust and easy operation. It supports features like syntax highlighting, auto-indentation, cross-platform use, zero-mouse-need movement bindings, etc.
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

### **Installation**
1. Clone the repository to your local machine:
    ```bash
    git clone https://github.com/therealsunx/trvim
    ```
2. Run the provided bash script to compile and install the editor to system via: 
    ```bash
    cd trvim
    ./install.sh
    ```
3. Launch the editor from your terminal:
    ```bash
    trvim filename.c
    #or
    trvim
    ```

## **Customization & Test**
1. Clone the repository to your local machine:
    ```bash
    git clone https://github.com/therealsunx/trvim
    ```
2. Set up the project for editing using provided setup shell script:
    ```bash
    ./setup.sh
    ```
3. Everything is set up for development. If you want to build and test it without installing into your system, then do this:
    ```bash
    cd build
    make
    ```
4. This will build the app for you. Run it:
    ```bash
    ./trvim filename
    #or
    ./trvim
    ```

___Use it just like you use vim.___


## **License**
This project is licensed under whatever it is used in `LICENSE` file. I copied it from my another project. Ignore any mistakes. See the `LICENSE` file for more information.


## Features-List(for you) : TODO board (for me)
___Completed features are marked with =
Undergoing features are marked with .
I still have to make a good looking table instead of list for this section. SO, ignore inconsistencies like a good programmer.___

- = movement
- = editing
- = scroll
- = prompted incremental search
- = open & save file
- = buffer status bar
- = editor command & msg bar
- = normal editing
- = syntax higlighting
    - = functions
    - = preprocessor directives
    - = comments
    - = strings
    - = numbers
    - = data members
    - = punctuations
    - = keywords
    - = escape characters in string
    - = 8bit colors instead of 4 bits
    - = default highlights in any type
- = absolute and relative line number
- = status msg bar
    - = mode indicator
    - = command buffer
- . vim operation modes
    - = normal mode
        - = command parsing
        - = mode switching
            - = insert mode
                - = i : cursor on left
                - = a : cursor on right
                - = I : cursor to starting character of line
                - = A : cursor to end of line
                - = o : add new line below and go to it
                - = O : add new line below but stay on current one
            - = select mode
                - = v : normal selection mode
                - = V : line selectio= mode
        - = '/' key find string
            - arrow up/down key during search to go to next/previous one
        - = f : find character after cursor
        - = F : find character before cursor
        - = r : replace character with next one
        - = $ : go to end of line
        - = 0 : go to start of line
        - = : : internal command prompt
            - = :{n} : go to line number n
            - = :w : save the current buffer into the file
            - = :wa : save all buffers into the file
            - = :q : quit current buffer
            - = :qa : quit all buffers
            - = :wq : save and quit current buffer
            - = :wqa : save and quit all buffers
            - = force quit buffers using ! at end of quit command
                - e.g. :q! force quits current buffer
        - = word navigation
            - = w : next word
            - = W : next word (ignore punctuations)
            - = b : previous word
            - = B : previous word (ignore punctuations)
            - = e : end of word
            - = E : end of word ignore punctutations
        - = paragraph navigation
            - = { : previous paragraph end
            - = } : next paragraph end
        - = absolute line jumps : {n}J : jump to line 'n'
        - = quick page navigation : fast cursor jumps for efficient work
            - = G : go to  last line of file
            - = H : move cursor towards beginning of visible page
            - = L : move cursor towards end of visible page
						- = _ : move cursor to first character in the line
        - = movement keybinds
            - h : left
            - j : down
            - k : up
            - l : right
        - = relative jumps
            - {n}(h|j|k|l) = n times (h|j|k|l)
            - e.g. 12j = jump down 12 lines
        - = command replication
            - type some number and then command to replicate it
            - e.g. relative jumps (as described above)
            - e.g. {n}w go to nth next word : 3w = go to 3rd word next to cursor 
            - e.g. {n}f{char} goes to nth next {char} character in current line
                - 3fc : go to 3rd 'c' away from cursor
            - e.g. {n}$ go to end of nth line below cursor
    - = insert mode
        - = auto indentation
        - = cursor range increase
        - = thin cursor
        - = arrow based movements
    - = visual/selection mode
				- = visual mode : select character/word-wise
				- = visual-line mode : select lines
        - = continous selection
        - = highlight selection
        - = navigation : same as normal mode(but selects items along the way) except some special keybinds
            - = Esc or Ctrl_C : switch to normal mode
            - = o or O : move cursor to different end of selection
            - = v : switch between visual and visual line mode
        - = d or x : delete selection
        - = c : cut selection (delete & switch to insert mode)
        - = r {character} : replace selection with following character
- . bufferize the editor
    - = views for buffers
    - . split {n} views in single screen
    - . keybinds for opening & editing views & buffers inside editor
        - . :e {filename} opens the file in buffer and shows in current view
        - . TAB goes to next buffer in the current view
        - . CTRL-{h, l} navigates to {left, right} views
        - . save and quit buffers and views (yet to think of bindings)

- . multiline/block comments (i hate it... may not even implement it... just use // on each line instead of /**/ sh*t)
