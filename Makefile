# GAPCM: Makefile

# Compiler executable.
#CC := clang
# Compiler options for development use.
CFLAGS := -fsanitize=address,leak
# Output directory.
OUTPUT := build
# Source directory.
SOURCE := src

all: gamdec gamenc gaminfo
check: gamtest

mingw-w64: CC := x86_64-w64-mingw32-gcc
mingw-w64: LDLIBS := -l ws2_32
mingw-w64 release: CFLAGS :=
mingw-w64 release: all

.SECONDEXPANSION:

gamdec gamenc gaminfo gamtest:: $(foreach object, $$@ gapcm/gapcm gam \
		$(foreach object, application strings strtonum, \
		common/${object}), ${OUTPUT}/${object}.o)
	${CC} ${CFLAGS} ${GMFC_CFLAGS} ${CPPFLAGS} ${GMFC_CPPFLAGS} ${GMFC_LDFLAGS} \
			-o $@ $^ ${LDLIBS}
gamtest::
		./$@

include ${SOURCE}/GMFC.mk
