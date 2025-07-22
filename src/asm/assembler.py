import sys
import os

def print_error(message: str):
    print("\033[31m" + "error: " + "\033[0m" + message)

def main():
    if len(sys.argv) > 2:
        print_error("Expected 1 file input, got " + str(len(sys.argv) - 1) + ".")
        return

    program_path = __file__.split("src/asm/assembler.py")[0] + "programs/" + sys.argv[1]

    if os.path.isfile(program_path) == False:
        print_error("File " + sys.argv[1] + " not found in programs/.")
        return

    program = ""
    with open(program_path, "r") as program_file:
        program = program_file.read()

    print(program)

if __name__ == "__main__":
    main()
