# therealtxt
I am attempting to make a text editor, that is similar in use as vim. Let's see how far it goes.

## what is expected?
A comprehensive text editor with syntax highlighting and few autocompletions for general text editing.

## Features
- _completed features are marked with =
- _undergoing features are marked with .

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
- . status msg bar
    - = mode indicator
    - . command buffer
- . vim operation modes
    - . normal mode
        - . command parsing
        - . number+command:replication
        - . absolute and relative line jumps
        - = movement keybinds (h,j,k,l)
        - = '/' key find string
        - . f : find character after cursor
        - . F : find character before cursor
        - = word navigation
            - = w : next word
            - = W : next word (ignore punctuations)
            - = b : previous word
            - = B : previous word (ignore punctuations)
            - = e : end of word
            - = E : end of word ignore punctutations
        - . paragraph navigation
            - . { : previous paragraph end
            - . } : next paragraph end
    - . insert mode
        - . auto indentation
        - = cursor range increase
        - = thin cursor
        - = arrow based movements
- . bufferize the editor
    - . multiple buffers
    - . buffer navigation

