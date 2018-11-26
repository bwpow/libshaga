GPP=g++
AR=ar
RM=rm
CP=cp
MKDIR=@mkdir -p

#GLIBC_INSTALL=/opt/rh/glibc-2.25

DESTINCLUDE=/usr/local/include/
DESTLIB=/usr/local/lib/

ST_LIBS= \
	-lrt \
	-lmbedcrypto
#	-L "$(GLIBC_INSTALL)/lib" \
#	-Wl,--rpath="$(GLIBC_INSTALL)/lib" \
#	-Wl,--dynamic-linker="$(GLIBC_INSTALL)/lib/ld-linux-x86-64.so.2" \

ST_CPPFLAGS= \
	-pipe \
	-Wall \
	-Wextra \
	-O3 \
	-std=c++14 \
	-march=native
#	-I "$(GLIBC_INSTALL)/include"

MT_LIBS=-pthread $(ST_LIBS)
MT_CPPFLAGS=-pthread $(ST_CPPFLAGS)

LIBDIR = lib
OBJDIR = obj
BINDIR = bin

BINEXT = bin

include Makefile.in
