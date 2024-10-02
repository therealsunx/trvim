# Cheat Sheet
The cheatsheet for the editor - trvim - are listed below. Be fluent and be productive. Enjoy !

## Movement and Navigation

### Basic Movement
- Arrow movements available
- `h` : Move left
- `j` : Move down
- `k` : Move up
- `l` : Move right
- `{n}*` : Move `n` times in the specified direction (e.g., `12j` moves down 12 lines)

### Line Navigation
- `$` : Go to the end of the line
    - `{n}$` : Go to end of nth line below
- `0` : Go to the start of the line
- `_` : Move cursor to the first character in the line

### Word Navigation
- `w` : Move to the beginning of the next word
- `W` : Move to the next word, ignoring punctuation
- `b` : Move to the beginning of the previous word
- `B` : Move to the previous word, ignoring punctuation
- `e` : Move to the end of the current word
- `E` : Move to the end of the word, ignoring punctuation
    - `{n}*` : Executes the above commands `n` times

### Paragraph Navigation
- `{` : Move to the beginning of the previous paragraph
- `}` : Move to the end of the next paragraph
    - `{n}*` : Executes the above commands n times

### Quick Page Navigation
- `G` : Move to the last line of the file
- `H` : Move towards top of the visible page
- `L` : Move towards bottom of the visible page
- `{n}J` : Jump to line `n`

## Mode Switching

### Normal Mode (Default)
- **Insert Mode**
  - `i` : Switch to insert mode
  - `a` : Switch to insert mode & move cursor to right
  - `I` : Switch to insert mode &  move cursor at the start of the line
  - `A` : Switch to insert mode & move cursor to the end of the line
  - `o` : Add a new line below and switch to insert mode
  - `O` : Add a new line above and switch to insert mode

- **Select Mode**
  - `v` : Switch to visual mode (select character-wise)
  - `V` : Switch to visual line mode (select lines)

## Insert Mode
- `Esc` or `Ctrl+C` : Return to normal mode
- **Auto Indentation** and **Arrow-Based Movements** available

## Visual/Selection Mode
In this mode you can do almost everything as same as normal mode, but it will select as you go.

- **Basic Selection**
  - `v` : Enter visual mode (character/word selection)
  - `V` : Enter visual line mode (select full lines)
  
- **Movement in Visual Mode**
  - Use the same navigation as normal mode, selecting text as you go
  - `o` : Move cursor to the other end of the selection
  - `Esc` or `Ctrl+C` : Return to normal mode

- **Actions**
  - `d` : Delete the selection
  - `c` : Cut selection and enter insert mode
  - `y` : yank/copy the selected text into system clipboard
  - `x` : cut the selection into system clipboard (copy and delete selection)
  - `p` : paste the clipboard content over selected text
  - `r{char}` : Replace selection with the specified `{char}`
  - `C` : Comment the current selection
  - `v` : Switch between Visual and Visual-line mode

## Search, Find & Replace
- `/` : Enter search mode (type to search and navigate with arrow keys)
- `f{char}` : Find the next occurrence of `{char}` after the cursor in the current line
    - `{n}f{char}` : Find the `n`th next occurrence of `{char}` in the current line
- `F{char}` : Find the next occurrence of `{char}` before the cursor in the current line
    - `{n}F{char}` : Find the `n`th previous occurrence of `{char}` in the current line
- `r{char}` : Replace current character with `{char}`
    - `{n}r{char}` : Replace next`n` characters with `{char}`

## Programmer's Commands
- `y` : Copies the selected text into system clipboard
- `p` : Pastes clipboard content after the cursor
- `P` : Pastes clipboard content before the cursor

## Command Mode
- `:` : Enter command prompt (for advanced operations)
  - `:{n}` : Go to line number `n`
  - `:w` : Save the current buffer to the file
  - `:wa` : Save all buffers to files
  - `:q` : Quit the current buffer
  - `:qa` : Quit all buffers
  - `:wq` : Save and quit the current buffer
  - `:wqa` : Save and quit all buffers
  - Append `!` to force quit without saving (e.g., `:q!`, `:qa!`)
  - `:e {filename}` : Open `{filename}` in a buffer and display in the current view
  - `:vs` : Split the screen vertically

## Buffer and View Management

### Buffer Commands
- `:e {filename}` : Open `{filename}` in a buffer and display in the current view
- `b` : Switch to previous buffer in the view
  - `{n}b` : Switch to `{n}`th previous buffer in the view
- `n` : Switch to next buffer in the view
  - `{n}n` : Switch to `{n}`th next buffer in the view

### Split View
- `:vs` : Split the screen vertically
- `]` : Move to view right of current one
- `[` : Move to view right of current one
- `,` : Decrease the size of the current view (by 20 units)
  - `{n},` : Decrease the width by `n` units
- `.` : Increase the size of the current view (by 20 units)
  - `{n}.` : Increase the width by `n` units

