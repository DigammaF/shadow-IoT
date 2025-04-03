CC = gcc
CFLAGS = -Iinclude -Wall -Werror -g
SRC = src/data.c src/session.c
OBJ = $(SRC:src/%.c=obj/%.o)
AR = ar
ARFLAGS = rcs
TARGET = liblantern.a

all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj $(TARGET)

.PHONY: all clean