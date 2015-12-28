# program version
VERSION = 0.1
# program name
PROG_NAME = dwm_status

# Customize below to fit your needs.

# paths

# includes and libs
INCS =
LIBS = -lX11

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -g -std=c99 -pedantic -Wall ${INCS} ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = gcc
LD = ${CC}
