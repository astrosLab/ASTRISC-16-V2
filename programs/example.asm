.org 0x0000

.equ stop, halt
stop .equ halt

hello_world: .ascii "Hello, World! //", 0
test: .byte 0b10111001
test1: .byte 'A'

.ascii "hello
world"

start:
    ldi R0, -42 // testing 123
    nop
    jmp start
    stop

