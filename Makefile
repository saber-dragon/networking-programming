CC=gcc
CXX=g++
CFLAGS=-g
RM=rm -rf


C_SRCs=$(wildcard *.c)
HEADERs=$(wildcard *.h)
C_OBJs=$(patsubst %.c,%.o,$(C_SRCs))

all: $(C_OBJs)

%.o: %.c $(HEADERs)
	$(CC) $< -o $@

.PHONY: clean

clean:
	$(RM) $(C_OBJs)

