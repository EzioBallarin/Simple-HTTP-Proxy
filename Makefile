#
# Makefile for proxy
#

CC=gcc
CFLAGS=-g -Wall
LDFLAGS=
BINS=proxy

.PHONY: all clean clean_tests

all: $(BINS)

proxy: proxy.o http_support.o 
proxy.o: src/proxy.c src/http_support.h
http_support.o: src/http_support.c

$(BINS):
	$(CC) $^ -o $@

%.o:
	$(CC) $< -ggdb -c $(CFLAGS) -o $@

clean:
	rm -rf *.o $(BINS) test/proxy_return/*

clean_tests:
	rm -rf test/proxy_return/*