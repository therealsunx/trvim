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
        - = command parsing
        - = '/' key find string
        - = f : find character after cursor
        - = F : find character before cursor
        - = r : replace character with next one
        - = $ : go to end of line
        - = 0 : go to start of line
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
    - . insert mode
        - . auto indentation
        - = cursor range increase
        - = thin cursor
        - = arrow based movements
    - . visual mode/selection mode
        - . character wise selection
        - . line selection
        - . delete selection
- . bufferize the editor
    - . multiple buffers
    - . buffer navigation

