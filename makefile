# Makefile for compiling the sources of Bathymetric surface tools project
# See header file for more information

# Author: Topi Filppula

SHELL = /bin/sh

# Use GCC compiler, define paths:
CC = gcc
OBJECT_DIR = obj/
BIN_DIR = bin/

# Flags with debugging helpers:
FLAGS = -O3 -march=native -Wall -Wextra -Wfloat-equal -Werror -std=gnu11
LIBS = -lgdal

# Clean:
RM = rm -f
RMDIR = rmdir

# Make new directories:
$(shell mkdir -p $(OBJECT_DIR))
$(shell mkdir -p $(BIN_DIR))

all: surfacetools

surfacetools: main.o rolling_coin_smoothing.o laplacian_smoothing.o inputandmemory.o fileoutput.o infoprinters.o cli.o focalmaxfilter.o offset.o
	$(CC) $(FLAGS) $(LIBS) $(OBJECT_DIR)*.o -o $(BIN_DIR)surfacetools

%.o: %.c
	$(CC) -c $(FLAGS) $< -o $(OBJECT_DIR)$@

clean:
	$(RM) $(OBJECT_DIR)*.o $(BIN_DIR)surfacetools
	$(RMDIR) $(OBJECT_DIR) $(BIN_DIR)
