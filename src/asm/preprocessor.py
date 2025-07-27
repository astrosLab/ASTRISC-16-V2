def scan(program_tokens: str):
    definitions = {}

    token_index = 0
    while token_index < len(program_tokens):
        token = program_tokens[token_index]

        if token[0] == "DIRECTIVE" and token[1] == ".equ":
            label = program_tokens[token_index + 1][1]
            replacement = program_tokens[token_index + 3][1]
            definitions[label] = replacement
            token_index += 5

        token_index += 1

    print(definitions)

    new_tokens = []
    for token in program_tokens:
        if token[1] in definitions.keys() and token[0] == "LABEL":
            new_tokens.append((token[0], definitions[token[1]]))
        else:
            new_tokens.append(token)

    line = []
    for token in new_tokens:
        line.append(token)
        if (token[0] == "EOL"):
            print(line)
            line = []
