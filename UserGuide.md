# User Guide
If you're new to __trvim__ then this is the perfect place for you to start. Let's go ...

## Opening Editor

__Prerequisites__: You should have trvim installed on your system.
 
There are plenty of ways to open the editor. Let's go through them one by one:
- Open the terminal or command prompt on your machine.

- You can open editor on any directory simply by goint to that directory and running trvim.
	```bash
	cd /path/to/directory
	trvim
	```

- Or you can open a specific file by following the `trvim` command with filename
	```bash
	trvim filename.ext
	```
	_It will create the file if it is not already there._

## Inside the Editor

Now that you are inside the editor, let's talk about what you're seeing.

- If you have opened a file that exists, then you should see the contents of the file. If your file extension is recognized by the system, then you should have syntax highlighted for it.

- If you have empty file open or no file open at all, you will see a editor entry message at the middle of the screen.

_Go ahead and open a file_

- __LINE COLUMN__
On the left side of the screen, you can see that there is a column showing some numbers, what do they represent?
    - Only the current line you are on has absolute line number
    - All of other lines have line numbers relative to the current line.
    - _Why this?_: It is because it makes it easier for navigation.
    - Let's take an example:
        - Let's say, You want to go to line and edit something.
        - It is not optimal to press arrow key multiple times to go to that line when you can just do it at once.
        - If you know that destination is 15 lines above you, then simply typing `15` and `arrow-up` should take you there.
        - Just so it happens, __trvim__ supports such features.
        - `15k` or `15↑` will take you 15 lines above. `20j` or `20↓` will take you 20 lines down.

- __FILE INFO__
    - There is a bar on the bottom of each view, showing the information about file.
    - First off there is, filename.
    - Alongside it, there is a filetype indicator and dirty indicator. (_Dirty indicator indicates that changes has not been saved._)
    - On the right side, there is cursor information.

- __COMMAND BAR__
    - On the bottom of the editor, there is a big bar which is used for taking inputs, entering special commands, etc.
    - It is also used to display some important messages

You can see that the cursor is `block` when you open the file. This is for better visibility of cursor while moving around the editor. This means you are currently in normal mode.
When you switch to `INSERT` mode, then cursor becomes thin.

### What are modes?

trvim - functions in 3 modes:
- __Normal Mode__: In this mode, anything you type is executed as a editor command (like movement, jumping, switching views, saving and opening files, etc)
- __Insert Mode__: In this mode, you can actually write on the file.
- __Visual Mode__: In this mode, you can select the text in the file. You can perform operations on selection, like deleting, replacing, copying, pasting over it, etc.
	There are 2 sub-modes in Visual Mode, `Visual`(character-wise selection) and `Visual-Line`(line-wise selection).

## Actually Using it

__Now let's use it in actual development.__

We will implement a `list` data structure in C.

- _Open you terminal._
- Make a directory, name it anything you like and jump into that directory.

- Now, let's open _main.c_ file in trvim to get started.
    ```trvim main.c```
    _You can see that filename is written in the bottom-left corner. It also shows file type is `[c/c++]`_

- You should see an empty file. Let's quickly write a program to write "hey! world".
    - Press `i` to switch to insert mode and type in the program as usual.
    - After that press `Esc` or `CTRL-C` to go back to normal mode.

    - _Since we have not saved the file after editing, you can see that there is dirty-indicator `[+]` alongside filename._
    - Type `:` to go to master-command mode. (_You can `Esc` your way out of it too_).
        - You can type a lots of commands in this mode. Look at the [Cheatsheet](https://github.com/therealsunx/trvim/blob/main/CheatSheet.md) for commands.
        - For now, type `:w`, which save the file in which we are currently in.
    - _Also there is cursor position on lower right hand side in the format `{cursorY}/{num_of_lines}, {cursorX}`._

- Since, we are making our own data structure, it is mandatory to have separate header file for it.
    - `:e list.h` and enter to create this file
    - Whoa!! Our `main.c` disappeared.. Not really, it is opened in background. To go to it, simply press `<` to switch to it.
    - What if I want to view both of those files side by side. trvim got it. Type `:vs` to split the window vertically.
    - Both of the views have same file? Press `<` or `>` like before to previous or next one.
    - You can jump between views by using `[` and `]` keys for left and right side jump.
    - Use this information to go to `list.h`. Now you can write our own code, or yank someone's code. So, make yourself comfortable.

    ```c
    #pragma once

    typedef struct {
        int *vals;
        size_t _size;
        unsigned int length;
    } list_i;

    void initlist(list_i *list);
    void listPush(list_i *list, int val);
    int listPop(list_i *list);
    void listInsert(list_i *list, unsigned int index, int val);
    int listRemoveAt(list_i *list, unsigned int index);
    ```
    
    - Copy this code, and go to `list.h` file and paste this code with `p`.
    - For now, we don't need 2 functions `listPush` and `listPop`, so in order to delete them do:
        - Go to line containing `listPush` by searching for it:
            - `/list` and press `DOWN-ARROW` without pressing `ENTER` key, to go to next `list` until you reach destination 
            or
            - `/listPush` and press enter
            _Notice that searched text is highlighted._
        - Press `Vjd` to go to `VISUAL-LINE` mode and go downwards and delete.
        - Or, You could have just `2D` to delete 2 lines.
        - _If you use `x` or `X` instead of `d` or `D`, it will copy the text to clipboard before deleting it.

- Now, create `list.c` file similiarly, and write the implementations for the above definitions in it. Make sure to include `list.h` in `list.c`
    - If you want to speed up the things, then do following steps,
        - `:0` to go to first line
        - `VG` to select from start to finish
        - `y` to copy selection to clipboard
        - Go to chatgpt and tell it to write implementation for the functions only.
        - Copy that code and go to `list.c` and press `p` to paste it.

- Now, you have code for list data structure ready. Implement it in `main.c` by including `list.h`.
- After writing everything, you're gonna want to test it. So, save everything by `:wa` command and `:qa` to quit all the files. Alternatively, you can `:wqa` to do it all at once.
- Now, compile and run using `gcc main.c list.c -o main && ./main`. _change command according to your OS_.

This much enough to get familiar with the editor's operation. There are also plenty of commands and keybinds to favour your development. It's like you think of doing a thing, and there's a keybind for that.


## Movement keybinds a.k.a motions

The keybindings for movements are what makes __trvim__ good to use. Let's take on some examples of use-case scenerio:

1. Let's suppose, we have following code:
    ```c
    void listInsert(list_i *list, unsigned int index);
    int listRemoveAt(list_i *list, unsigned int index, int val);
    ```
    You can see that the parameters are accidently swapped. Let's say you are currently at `*` in the first line. To fix this, we can do following:
    - go to `(` : `F(` : finds the first occurance of `(` before cursor
    - select everything upto `)` : `vf)` : `v` switches to visual mode, `f)` jumps to `)`, everything is selected on the way
    - cut the selection : `x` : cuts into system clipboard
    - jump down : `j`
    - go to `(` : `f(`
    - select everything upto `)` : `vf)` : `v` switches to visual mode, `f)` jumps to `)`, everything is selected on the way
    - paste previously cut text over current selection, and also copy current selection : `p` or `P`
    - jump upto previous line : `k` :: (_now the cursor should be on `;` character_)
    - paste the text before the cursor : `P`

2. Let's suppose you have this code:
    ```c
    char *peekFile(char *filename, int lineCount){
        *file = fopen(filename, "r")
        char *contents = getLines(file, lineCoun);
        return contents;
    }
    ```
    Just by looking at the code, you can see that there are plenty of errors and possible hazards. Let's go and fix'em.
    Let's say we are at `(` of first line.
    - missing type of `file`
        - `jI` to jump down and switch to insert mode as well as move cursor before first character.
        - Let's write `FILE ` there and `ESC` to switch back to normal mode.
    - missing `;` at the end
        - `A` to go to end as well as switching to insert mode. Now add `;` there and `ESC` to switch to normal mode.
    - continue only if file is opened successfully.
        - `o` to add new line below as well as switch to insert mode
        - Add this check there:
            ```c
            if(!file) return NULL;
            ```
    - wrong spelling of `lineCount` in 3rd line
        - `/line` to go to that word quickly
        - `ea` to go to end of that word and switch to insert mode with cursor just after that word.
        - Now, just add that missing `t` and go back to normal mode.
        

