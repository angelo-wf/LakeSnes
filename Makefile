
CC = clang
# CFLAGS = -O0 -g -framework SDL2 -F . -I ./snes -I ./zip -rpath @executable_path
CFLAGS = -O3 -g -framework SDL2 -F . -I ./snes -I ./zip -rpath @executable_path
# CFLAGS = -O3 -flto -g -framework SDL2 -F . -I ./snes -I ./zip -rpath @executable_path

execname = lakesnes

cfiles = snes/spc.c snes/dsp.c snes/apu.c snes/cpu.c snes/dma.c snes/ppu.c snes/cart.c snes/input.c snes/snes.c snes/snes_other.c \
 zip/zip.c tracing.c main.c
hfiles = snes/spc.h snes/dsp.h snes/apu.h snes/cpu.h snes/dma.h snes/ppu.h snes/cart.h snes/input.h snes/snes.h \
 zip/zip.h zip/miniz.h tracing.h

.PHONY: clean

$(execname): $(cfiles) $(hfiles)
	$(CC) $(CFLAGS) -o $@ $(cfiles)

clean:
	rm -f $(execname)
