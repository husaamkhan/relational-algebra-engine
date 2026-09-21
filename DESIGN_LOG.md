## September 15 - 18
-   Research + learning on grammar, lexers, parsers, etc.
-   Working on GRAMMAR.md

## September 19
-   Basic project setup

## September 20
-   Getting started on the lexer
-   Imported a common library of mine that I am using for basic logging, utility functions,
    Arena definition, etc.
-   Implementation for tokens with single-character lexemes (LPAREN - EQUAL)
-   Unit tests
    -   Using ThrowTheSwitch's Unity Test Framework for the UTs
    -   Spent time on finding ways to replace UT boilerplate to make writing the UTs faster since I'm short on time
-   Found some issues that I will address in the next session
    -   Issues in my arena implementation
        -   I use malloc and realloc, allocating a small piece of memory and expanding with realloc when needed.
            I foresee potential hard-to-replicate bugs with using realloc in cases where the arena is moved to
            a different place in the memory. Doesn't matter for the lexer since I wouldn't have pointers to random
            tokens throughout the arena, but it could cause problems later in the parser. I will change this to
            mmap/munmap later, and maybe at a later date implement some wrapper that allows this to be cross-platform
        -   Used AI for writing some of the code but found that it is not making smart considerations on how it removes
            tokens from the Arena in error cases. It is deleting tokens with arena->used -= sizeof(Token) but that doesnt
            take into account any extra space caused by alignment upwards
    -   Issues in my lexer implementation
        -   Used AI to write out some of the lexer but it does not realize that the grammar is not enforcing tokens to be
            separated by ' '. Because of this it is making the error handling skip to the next word (loop until the next ' ')
            when an unrecognized character is found, but this is could lead to tokens being skipped

## September 21
-   Continuing work on lexer. Addressing issues that I identified yesterday.
    -   Improved arena implementation, but I'll come write the wrapper around mmap later so as to not waste time
    -   Used AI to generate documentation for the code
    -   Lexer improvements before moving forward
        -   Preventing possible issues with peek by changing it to not consume characters
        -   Created a separate function advance() that advances the lexer forward
        -   Fixed possibly buggy handling for !=, <=, and >=
    -   Moving forward with lexer
        -   Added identifier and keyword recognition

