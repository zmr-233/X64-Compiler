CC=gcc
CCFLAGS=-std=c11 -g -fno-common -static
CCFLAGS+=-Wall -MMD -MP -I./

ROOT:=$(shell pwd)
SUBDIR:=$(ROOT)
OUTPUT:=$(ROOT)/output
TESDIR:=$(ROOT)/test
DEPDIR:=$(ROOT)/output

INCS:=$(foreach dir, $(SUBDIR), -I$(dir))
SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c, $(OUTPUT)/%.o, $(filter %.c,$(SRCS)))
DEPS:= $(patsubst $(OUTPUT)/%.o,$(DEPDIR)/%.d,$(OBJS))

TEST:=$(ROOT)/test.sh
TESTAR:=$(TESDIR)/tmp
TARGET:=$(OUTPUT)/main.elf

# 是否使用x64/rvcc平台
PLAT?=x64
ifeq ($(PLAT),x64)
	CCFLAGS+=-D PLATFORM_X64
else
	CCFLAGS+=-D PLATFORM_RISCV
endif


test: $(TARGET)
	@echo "\e[38;2;139;253;216mTest Target ......\e[38;2;241;250;140m"
	@mkdir -p $(TESDIR)
	@chmod 707 $(TEST)
	@chmod 707 $(TARGET)
	$(TEST) $(TARGET) $(TESTAR) $(PLAT)
	@echo "\e[38;2;139;253;216m\n...... Test Passed ......\e[38;2;241;250;140m"

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "\e[38;2;139;253;216mMaking Target ......\e[38;2;241;250;140m"
	$(CC) $(CCFLAGS) $^ -o $@ 

$(OUTPUT)/%.o : %.c FORCE
	@echo "\e[38;2;139;253;216mMaking output ......\e[38;2;241;250;140m"
	@mkdir -p $(OUTPUT)
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@ 

.PHONY: all clean test

clean:
	@echo "\e[38;2;255;0;0mDo make clean\e[0m"
	rm -rf $(OUTPUT) $(DEPDIR) $(TESDIR)

FORCE:

-include $(DEPS)