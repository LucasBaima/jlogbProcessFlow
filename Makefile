CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = processflow

OBJECTS = main.o processflow.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

main.o: main.c processflow.h
	$(CC) $(CFLAGS) -c main.c

processflow.o: processflow.c processflow.h
	$(CC) $(CFLAGS) -c processflow.c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)