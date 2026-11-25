" Vim syntax file
" Language:     Atmos
" Origin:       atmos-lang/etc (atmos.vim, 2025), updated to the
"               lexer of this compiler (src/lexer.c)
"
" To use it, copy this file to ~/.vim/syntax/atmos.vim and add to
" ~/.vimrc (or to ~/.vim/ftdetect/atmos.vim):
"
"     autocmd BufRead,BufNewFile *.atm setfiletype atmos

if exists("b:current_syntax")
    finish
endif

" identifiers: letters, digits, and _
syn iskeyword @,48-57,_

" keywords (KEYS in src/lexer.c)
syn keyword atmosStatement  abort await break catch context defer do else
syn keyword atmosStatement  emit escape func if ifs in include loop match
syn keyword atmosStatement  mute on par pin return set skip solo spawn stage
syn keyword atmosStatement  task tasks test thread throw until val var
syn keyword atmosStatement  watching where with while
syn keyword atmosConstant   false nil true

" implicit names
syn keyword atmosIdentifier it pub

" globals of the library
syn keyword atmosFunction   assert next print tonumber tostring type
syn keyword atmosFunction   X atmos io iter math os string vector

" tags: :x  :T.A.x
syn match   atmosTag        ':[A-Za-z0-9_][A-Za-z0-9_.]*'

" numbers: 10  10.5  0xFF  1e10, and clocks: 5s  300ms  2h30min  1.5day
syn match   atmosNumber     '\<\d[A-Za-z0-9_]*\(\.\d[A-Za-z0-9_]*\)\='
syn match   atmosNumber     '\<\d\+\(\.\d\+\)\=[eE][+-]\=\d\+\>'
syn match   atmosNumber     '\<0[xX][0-9A-Fa-f]\+\(\.[0-9A-Fa-f]\+\)\=\([pP][+-]\=\d\+\)\=\>'

" operators (OPS in src/lexer.c)
syn match   atmosOperator   '[-+*/%<>=|&?!#~^]\+'

" strings '...' "..." and natives `...`: one delimiter is single-line, three
" or more are multi-line, closed by the same number of delimiters
syn region  atmosString     start=+\z('''\+\)+ end=+\z1+ contains=@Spell
syn region  atmosString     start=+\z("""\+\)+ end=+\z1+ contains=@Spell
syn region  atmosString     start=+'\(''\)\@!+ skip=+\\\\\|\\'+ end=+'+ oneline contains=atmosEscape
syn region  atmosString     start=+"\(""\)\@!+ skip=+\\\\\|\\"+ end=+"+ oneline contains=atmosEscape
syn match   atmosString     +''+
syn match   atmosString     +""+
syn match   atmosEscape     +\\.+ contained
syn region  atmosNative     start=+\z(```\+\)+ end=+\z1+
syn region  atmosNative     start=+`\(``\)\@!+ end=+`+ oneline
syn match   atmosNative     +``+

" comments: ;; to the end of the line, and ;;; ... ;;; (three or more,
" closed by the same number of semicolons)
syn keyword atmosTodo       TODO FIXME XXX contained
syn match   atmosComment    ';;.*$' contains=atmosTodo,@Spell
syn region  atmosComment    start=+\z(;;;\+\)+ end=+\z1+ contains=atmosTodo,@Spell

hi def link atmosStatement  Statement
hi def link atmosConstant   Constant
hi def link atmosIdentifier Identifier
hi def link atmosFunction   Function
hi def link atmosTag        Type
hi def link atmosNumber     Number
hi def link atmosOperator   Operator
hi def link atmosString     String
hi def link atmosEscape     SpecialChar
hi def link atmosNative     PreProc
hi def link atmosComment    Comment
hi def link atmosTodo       Todo

let b:current_syntax = "atmos"
