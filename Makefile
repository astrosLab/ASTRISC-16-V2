CC = g++
CFLAGS = -Wall

SRC = src/main.cpp src/emu/emulator.cpp
TARGET = build/astrisc-16-v2

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: all
	./$(TARGET)

clean:
	rm -rf build

