import sys
import os
import lexer
import preprocessor

def replace_tokens(definitions: dict[str, str], tokens: list[tuple[str]]):
    new_tokens = []

    for token in tokens:
        if token[0] == "LABEL":
            if token[1] in definitions.keys():
                new_tokens.append((token[0], definitions[token[1]]))
            else:
                new_tokens.append(token)
        else:
            new_tokens.append(token)
            
    return new_tokens

def print_error(message: str):
    print("\033[31m" + "error: " + "\033[0m" + message)

def main():
    if len(sys.argv) > 2 or len(sys.argv) < 2:
        print_error("Expected 1 file input, got " + str(len(sys.argv) - 1) + ".")
        return

    program_path = __file__.split("src/asm/assembler.py")[0] + "programs/" + sys.argv[1]

    if os.path.isfile(program_path) == False:
        print_error("File " + sys.argv[1] + " not found in programs/.")
        return

    program = ""
    with open(program_path, "r") as program_file:
        program = program_file.read()

    # Preprocess, get definitions and preprocessed program
    pp_out = preprocessor.replace(program)
    preprocessed = pp_out["value"]["preprocessed"]
    definitions = pp_out["value"]["definitions"]

    if pp_out["status"] == "error":
        print_error(processed["value"])

    program_tokens = lexer.tokenize(preprocessed)

    new_tokens = replace_tokens(definitions, program_tokens)

if __name__ == "__main__":
    main()
