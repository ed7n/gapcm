# Generative Makefile for C (GMFC) u0r0 by Brendon, 05/05/2023.
#
# To be only included by a parent makefile, it defines the `clean` phony target
# that removes object and dependency files from `OUTPUT`.
#
# Copy the first three variables to the parent makefile, and change its copy to
# preference.
#
# Based on [https://makefiletutorial.com/#makefile-cookbook] with hardening tips
# from Airbus SecLab, Debian, and Red Hat.

# Compiler base and security warning options.
GMFC_CFLAGS := -g -std=c2x -Wall -Wextra -Wpedantic -Warray-bounds=2 \
		-Wcast-qual -Wformat-overflow=2 -Wformat-truncation=2 -Wformat=2 \
		-Wnull-dereference -Wshift-overflow=2 -Wstack-protector \
		-Wstringop-overflow=4 -Wtrampolines -Wvla
# Compiler hardening options.
GMFC_CFLAGS += -fPIE -fsanitize-undefined-trap-on-error -fsanitize=undefined \
		-fstack-clash-protection -fstack-protector-strong
# Preprocessor hardening options.
GMFC_CPPFLAGS := -D_FORTIFY_SOURCE=3
# Linker hardening options.
GMFC_LDFLAGS := -Wl,-z,relro,-z,now
# Output directory.
OUTPUT ?= build
# Source directory.
SOURCE ?= src

# `-fstack-protector-strong` requires `libssp`.
mingw-w64: GMFC_CFLAGS := -fPIE -fsanitize-undefined-trap-on-error \
		-fsanitize=undefined -fstack-clash-protection
mingw-w64: GMFC_LDFLAGS :=
mingw-w64 release: GMFC_CFLAGS += -O2

# Object files.
objects := $(patsubst %.c, %.o, $(shell find ${SOURCE} -iname '*.c' -printf \
		'${OUTPUT}/%P '))
# Generated makefiles.
depends := $(patsubst %.o, %.d, ${objects})
# Source files.
sources := $(shell find ${SOURCE} -type d -printf '%p ')
# Output directories.
outputs := $(patsubst ${SOURCE}%, ${OUTPUT}%, ${sources})

.PHONY: clean

clean:
	${RM} $(foreach output, ${outputs}, $(addprefix ${output}/*., d o))

${objects}: | ${OUTPUT}

${OUTPUT}:
	mkdir -p ${outputs}

${OUTPUT}/%.o: ${SOURCE}/%.c
	${CC} ${CFLAGS} ${GMFC_CFLAGS} ${CPPFLAGS} ${GMFC_CPPFLAGS} -c -MMD -o $@ $<

-include ${depends}
