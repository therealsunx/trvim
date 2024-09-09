TARGET = therealtxt
FLAGS = -Wall -Wextra -pedantic -std=c99
SRC=$(wildcard **/*.c)

all: $(SRC)
	$(CC) -o $(TARGET) $^ $(FLAGS)
