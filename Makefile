# Define the compiler
CC = gcc

# Define compiler flags
CFLAGS = -c -g

# Define the output executable
OUTPUT = main.exe

# Define the source files
SRC = main.c otpc.c

# Define the object files (same names as source files but with .o extension)
OBJ = $(SRC:.c=.o)

# Default target
all: $(OUTPUT)

# Rule to build the executable
$(OUTPUT): $(OBJ)
	$(CC) -o $@ $^

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) $<

# Clean up
clean:
	rm -f $(OBJ) $(OUTPUT) *.dat

