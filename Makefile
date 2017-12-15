# ISA 2017/2018
# Project Měření ztrátovosti a RTT
# Author: xpacak01, Gabriela Pacáková
# File: Makefile
# Description: Makefile for testovac

comp=gcc
cbin=testovac
#FILES=$(wildcard *.c)
FILES=socket.c memops.c testovac.c argparse.c icmp.c udp.c utils.c

flags=-g -Wall -Wextra -pedantic -pthread -std=gnu99

all:
	$(comp) $(flags) $(FILES) -o $(cbin) -lm

clean:
	rm $(cbin)
