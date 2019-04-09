GPP ?= g++
AR ?= ar
RM ?= rm
CP ?= cp
MKDIR ?= @mkdir -p

DESTINCLUDE ?= /usr/local/include/
DESTLIB ?= /usr/local/lib/

ST_LIBS = \
	-lrt \
	-lmbedcrypto

ST_CPPFLAGS = \
	-pipe \
	-fPIC \
	-Wall \
	-Wextra \
	-O3 \
	-std=c++17 \
	-march=native

MT_LIBS = -pthread $(ST_LIBS)
MT_CPPFLAGS = -pthread $(ST_CPPFLAGS)

LIBDIR = lib
OBJDIR = obj
BINDIR = bin

BINEXT = bin

include Makefile.in
