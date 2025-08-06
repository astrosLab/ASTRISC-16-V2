"""
1 BEGIN: beginning of program
1 DIRECTIVE: .file, .org, .ascii, .equ, .byte, .word
1 LABEL: start, hello_world, MOV, ADD, JMP, CALL, R0, R1, R2, etc.
1 COLON: :
1 NUMBER_DEC: 1, -1, etc.
1 NUMBER_BIN: 0b1101, 0b0001011, etc.
1 NUMBER_HEX: 0xFF, 0x5849, etc.
1 STRING: "Hello, World!", 'A', etc.
1 COMMA: ,
1 COMMENT (IGNORE): // testing 123
1 EOL: \n
"""

def is_varchar(character: str, special_char_list: list[str] = []):
    return character.isalpha() or character == "_" or character in special_char_list

def tokenize(program: str):
    tokens = [("BEGIN", '')]

    char_index = 0

    while char_index < len(program):
        char = program[char_index]

        # End Of Line
        if char == '\n':
            char_index += 1
            tokens.append(("EOL", '\n'))
            continue

        if char.isspace():
            char_index += 1
            continue

        # Comments
        if char == '/':
            if program[char_index + 1] == '/':
                while char_index < len(program) and program[char_index] != '\n':
                    char_index += 1
                tokens.append(("EOL", '\n'))
                continue

        # Directives
        if char == ".":
            start = char_index
            while char_index < len(program) and is_varchar(program[char_index], ['.']):
                char_index += 1
            directive = program[start:char_index]
            tokens.append(("DIRECTIVE", directive))
            continue

        # Strings/Chars
        if char == '"':
            start = char_index
            char_index += 1
            in_string = True
            while in_string == True:
                if char_index >= len(program):
                    raise ValueError("Unfinished String")
                if program[char_index] == '"':
                    in_string = False
                else:
                    char_index += 1
            char_index += 1
            string = program[start+1:char_index-1]
            tokens.append(("STRING", string))
            continue

        if char == "'":
            start = char_index
            char_index += 1
            in_string = True
            while in_string == True:
                if char_index >= len(program):
                    raise ValueError("Unfinished String")
                if program[char_index] == "'":
                    in_string = False
                else:
                    char_index += 1
            char_index += 1
            string = program[start+1:char_index-1]
            tokens.append(("STRING", string))
            continue

        # Labels/Variables/Registers
        if char.isalpha():
            start = char_index
            while char_index < len(program) and is_varchar(program[char_index], list("0123456789")):
                char_index += 1
            label = program[start:char_index]
            tokens.append(("LABEL", label))
            continue

        # Colons
        if char == ':':
            char_index += 1
            tokens.append(("COLON", ':'))
            continue

        # Negative Decimal Numbers
        if char == "-":
            start = char_index
            char_index += 1
            while char_index < len(program) and program[char_index] in list("0123456879"):
                char_index += 1
            number = program[start:char_index]
            tokens.append(("NUMBER_DEC", number))
            continue

        # Numbers (Decimal only positive)
        if char.isdigit():
            # Bin
            if char == '0' and (program[char_index + 1] == 'b'):
                start = char_index
                char_index += 2
                while char_index < len(program) and program[char_index] in list("01"):
                    char_index += 1
                number = program[start:char_index]
                tokens.append(("NUMBER_BIN", number))
                continue
            # Hex
            elif char == '0' and (program[char_index + 1] == 'x'):
                start = char_index
                char_index += 2
                while char_index < len(program) and program[char_index] in list("0123456789ABCDEFabcdef"):
                    char_index += 1
                number = program[start:char_index]
                tokens.append(("NUMBER_HEX", number))
                continue
            # Decimal
            else:
                start = char_index
                while char_index < len(program) and program[char_index].isdigit():
                    char_index += 1
                number = program[start:char_index]
                tokens.append(("NUMBER_DEC", number))
                continue

        if char == ',':
            char_index += 1
            tokens.append(("COMMA", ','))
            continue

        char_index += 1

    """
    line = []
    for token in tokens:
        line.append(token)
        if (token[0] == "EOL"):
            print(line)
            line = []
    """

    return tokens
