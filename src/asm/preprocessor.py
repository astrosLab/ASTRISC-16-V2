def is_validname(name: str):
    for char in name:
        if not (char.isalpha() or char == '_'):
            return False
    return True

def is_validvalue(value: str):
    if value.startswith('-'):
        for char in value[1:]:
            if not char.isdigit():
                return False
        return True

    if value.startswith("0b"):
        for char in value[2:]:
            if not char in list("01"):
                return False
        return True

    if value.startswith("0x"):
        for char in value[2:]:
            if not char in list("0123456789ABCDEFabcdef"):
                return False
        return True

    if value[0].isdigit():
        for char in value:
            if not char.isdigit():
                return False
        return True

    for char in value:
        if not (char.isalpha() or char == '_'):
            return False
    return True

def replace(program: str):
    preprocessed = []

    definitions = {}

    for line in program.split('\n'):
        clean_line = line.strip().split(' ')

        if clean_line[0] != ".equ":
            preprocessed.append(line)
        else:
            stripped_line = line.strip()
            split_line = stripped_line[5:].split(',')
            if len(split_line) >= 3:
                return {
                    "status": "error",
                    "value": "Definition '" + line + "' has too many parameters (expected 2)."
                }
            elif len(split_line) == 1:
                return {
                    "status": "error",
                    "value": "Definition '" + line + "' doesn't have enough parameters (expected 2)."
                }

            split_names = [name for name in split_line[0].split(' ') if name]
            split_values = [value for value in split_line[1].split(' ') if value]
            if len(split_names) > 1:
                return {
                    "status": "error",
                    "value": "Definition '" + stripped_line + "' has too many names (expected 1)."
                }
            elif len(split_names) < 1:
                return {
                    "status": "error",
                    "value": "Definition '" + stripped_line + "' has no name (expected 1)."
                }

            if len(split_values) > 1:
                return {
                    "status": "error",
                    "value": "Definition '" + stripped_line + "' has too many values (expected 1)."
                }
            elif len(split_values) < 1:
                return {
                    "status": "error",
                    "value": "Definition '" + stripped_line + "' has no value (expected 1)."
                }

            name = split_names[0].replace(' ', '')
            value = split_values[0].replace(' ', '')

            if is_validname(name) == False:
                return {
                    "status": "error",
                    "value": "Name in definition '" + stripped_line + "' has invalid characters."
                }

            if is_validvalue(value) == False:
                return {
                    "status": "error",
                    "value": "Value in definition '" + stripped_line + "' has invalid characters."
                }

            definitions[name] = value

    print(definitions)

    return {
        "status": "success",
        "value": preprocessed
    }
