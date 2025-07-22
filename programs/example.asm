.org 0x0000

hello_world: .ascii "Hello, World!", 0

start:
    ldi R0, 42 // testing 123
    nop
    jmp start
    halt

