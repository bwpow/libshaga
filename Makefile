GPP=g++
AR=ar
RM=rm
CP=cp
MKDIR=@mkdir -p
LIBS=-pthread -lrt -lmbedcrypto
CPPFLAGS=-pthread -pipe -Wall -Wextra -std=c++14

DESTINCLUDE=/usr/local/include/
DESTLIB=/usr/local/lib/

LIBDIR = lib
OBJDIR = obj
BINDIR = bin

BINEXT = bin

include Makefile.in
