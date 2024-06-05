CC=gcc
CCFLAGS=-std=c11 -g -fno-common -static

TARGET=main
SRCS=main.c
OBJS=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CCFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

test: all
	./test.sh

clean:
	rm -f $(OBJS) $(TARGET) tmp* *~

.PHONY: all clean test