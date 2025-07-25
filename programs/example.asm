.org 0x0000

#define ERROR 0x0

hello_world: .ascii "Hello, World!", 0
test: .byte 0b10111001
test1: .byte 'A'

start:
    ldi R0, -42 // testing 123
    nop
    jmp start
    halt

