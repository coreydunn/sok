; set ft=nasm

section .data
global includefile, includefilesize

includefile:
incbin "file.tbz"
includefilesize:
