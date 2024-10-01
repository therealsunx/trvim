# User Guide
Below are very simple to understand & intuitive commands for doing different things. The power does not come from individual commands. Its the combination of commands that makes your work faster.

## Movement and Navigation

### Basic Movement
- `h` : Move left
- `j` : Move down
- `k` : Move up
- `l` : Move right
- `{n}(h|j|k|l)` : Move `n` times in the specified direction (e.g., `12j` moves down 12 lines)

### Line Navigation
- `$` : Go to the end of the line
- `0` : Go to the start of the line
- `_` : Move cursor to the first character in the line

### Word Navigation
- `w` : Move to the beginning of the next word
- `W` : Move to the next word, ignoring punctuation
- `b` : Move to the beginning of the previous word
- `B` : Move to the previous word, ignoring punctuation
- `e` : Move to the end of the current word
- `E` : Move to the end of the word, ignoring punctuation

### Paragraph Navigation
- `{` : Move to the beginning of the previous paragraph
- `}` : Move to the end of the next paragraph

### Quick Page Navigation
- `G` : Move to the last line of the file
- `H` : Move towards the beginning of the visible page
- `L` : Move towards the end of the visible page

### Absolute Line Jumps
- `{n}J` : Jump to line `n`

### Relative Line Jumps
- `{n}$` : Go to the end of the `n`th line below the cursor

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
In this mode you can do almost everything as same as normal mode, but it will select asyou go.

- **Basic Selection**
  - `v` : Enter visual mode (character/word selection)
  - `V` : Enter visual line mode (select full lines)
  
- **Movement in Visual Mode**
  - Use the same navigation as normal mode, selecting text as you go
  - `o` or `O` : Move cursor to the other end of the selection
  - `Esc` or `Ctrl+C` : Return to normal mode

- **Actions**
  - `d` or `x` : Delete the selection
  - `c` : Cut selection and enter insert mode
  - `r {character}` : Replace selection with the specified character
  - `C` : Comment the current selection

## Search & Find
- `/` : Enter search mode (type to search and navigate with arrow keys)
- `f {char}` : Find the next occurrence of `{char}` after the cursor in the current line
- `F {char}` : Find the next occurrence of `{char}` before the cursor in the current line

## Command Mode
- `:` : Enter command prompt (for advanced operations)
  - `:{n}` : Go to line number `n`
  - `:w` : Save the current buffer to the file
  - `:wa` : Save all buffers to files
  - `:q` : Quit the current buffer
  - `:qa` : Quit all buffers
  - `:wq` : Save and quit the current buffer
  - `:wqa` : Save and quit all buffers
  - Append `!` to force quit/save (e.g., `:q!`, `:qa!`)

## Buffer and View Management

### Buffer Commands
- `:e {filename}` : Open `{filename}` in a buffer and display in the current view
- `p` / `n` : Switch to the previous/next buffer in the current view

### Split View
- Split the screen into `{n}` views
- `[` / `]` : Move to the left/right view
- `<` / `>` : Decrease/increase the size of the current view
  - `{n}>` : Increase the width by `n` units
  - `>` : Increase the width by a chunk (default: 20 units)

## Command Replication
- **Number Prefix**: Precede a command with a number to repeat it
  - `{n}w` : Move to the `n`th next word (e.g., `3w` moves to the third word ahead)
  - `{n}f{char}` : Go to the `n`th occurrence of `{char}` (e.g., `3fc` moves to the third `c` from the cursor)
  - `{n}$` : Go to the end of the `n`th line below

## Examples
Take a look at following examples along with guide toget hold of it.
- `ea` will go to end of word and start writing after that.
- `VC` will comment current line, `V2jC` will comment 3 lines
- `{V}C` will comment the paragraph. (breakdown: `{` moves to empy line before paragraph, `V` switches to selection/visual mode, `}` moves to next end of paragraph, and `C` will comment all selected lines)
- `wverc` will go to next word and replace it with c. It seems hard knowing this command but breaking it down will make you understand that it is not even a bit hard. `w` will go to start of next word, `v` will switch to selection/visual mode, `e` will go to end of word, (notice that the word will get highlighted now), `rc` will replace selection with c.
- similiarly, `wvec` will cut the next word and also switches to insert mode, so you can write something in its place instead.

Easy right !!
_(Note: you can enable such movement bindings on code editors like VS code by installing VIM extensions)_

