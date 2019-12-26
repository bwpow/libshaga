GPP ?= g++
AR ?= ar
RM ?= rm
CP ?= cp
MKDIR ?= @mkdir -p

DESTINCLUDE ?= /usr/local/include/
DESTLIB ?= /usr/local/lib/

ifdef SHAGA_SANITY
	SANITY = -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer
else
	SANITY =
endif

ST_LIBS = \
	$(SANITY) \
	-pie \
	-lrt \
	-lmbedcrypto

ST_CPPFLAGS = \
	-pipe \
	-fPIE \
	-Wall \
	-Wextra \
	-Wshadow \
	-fstack-protector-strong \
	-O3 \
	-std=c++17 \
	-march=native \
	$(SANITY)


MT_LIBS = -pthread $(ST_LIBS)
MT_CPPFLAGS = -pthread $(ST_CPPFLAGS)

LIBDIR = lib
OBJDIR = obj
BINDIR = bin

BINEXT = bin

include Makefile.in
