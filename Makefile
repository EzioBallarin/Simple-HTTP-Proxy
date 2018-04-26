#
# Makefile for proxy
#

CC=gcc
CFLAGS=-g -Wall
LDFLAGS=
BINS=proxy

.PHONY: all clean

all: $(BINS)

proxy: proxy.o http_support.o 
proxy.o: src/proxy.c src/http_support.h
http_support.o: src/http_support.c

$(BINS):
	$(CC) $^ -o $@

%.o:
	$(CC) $< -c $(CFLAGS) -o $@

clean:
	rm -f *.o $(BINS)

