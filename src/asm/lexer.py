"""
1 DIRECTIVE: .file, .org, .ascii, .equ, .byte, .word
1 LABEL: start, hello_world, MOV, ADD, JMP, CALL, R0, R1, R2, etc.
1 COLON: :
1 NUMBER: 1, -1, 0b1101, 0b0001011, 0xFF, 0x5849, etc.
1 STRING: "Hello, World!", 'A', etc.
1 COMMA: ,
1 COMMENT (IGNORE): // testing 123
1 PREPROCESSOR: #define
1 EOL: \n
"""

def is_varchar(character: str, special_char_list: list[str] = []):
    return character.isalpha() or character == "_" or character in special_char_list

def scan(program: str):
    tokens = []

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

        # Preprocessors
        if char == "#":
            start = char_index
            while char_index < len(program) and is_varchar(program[char_index], ['#']):
                char_index += 1
            preprocessor = program[start:char_index]
            tokens.append(("PREPROCESSOR", preprocessor))
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
            string = program[start:char_index]
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
            string = program[start:char_index]
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
            while char_index < len(program) and program[char_index] in list("-0123456879"):
                char_index += 1
            number = program[start:char_index]
            tokens.append(("NUMBER", number))
            continue

        # Numbers (Decimal only positive)
        if char.isdigit():
            # Hex/Bin
            if char == '0' and (program[char_index + 1] in ["x", "b"]):
                start = char_index
                while char_index < len(program) and program[char_index] in list("012345679xb"):
                    char_index += 1
                number = program[start:char_index]
                tokens.append(("NUMBER", number))
                continue
            # Decimal
            else:
                start = char_index
                while char_index < len(program) and program[char_index].isdigit():
                    char_index += 1
                number = program[start:char_index]
                tokens.append(("NUMBER", number))
                continue

        if char == ',':
            char_index += 1
            tokens.append(("COMMA", ','))
            continue

        char_index += 1

    print(tokens)

    return tokens
