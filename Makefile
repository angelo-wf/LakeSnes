
CC = clang
CFLAGS = -O3 -Wall -Wextra -I ./snes -I ./zip

WINDRES = windres

execname = lakesnes
sdlflags = `sdl2-config --cflags --libs`

appname = LakeSnes.app
appexecname = lakesnes_app
appsdlflags = -framework SDL2 -F sdl2 -rpath @executable_path/../Frameworks

winexecname = lakesnes.exe

cfiles = snes/spc.c snes/dsp.c snes/apu.c snes/cpu.c snes/dma.c snes/ppu.c snes/cart.c snes/input.c snes/statehandler.c snes/snes.c snes/snes_other.c \
 zip/zip.c tracing.c main.c
hfiles = snes/spc.h snes/dsp.h snes/apu.h snes/cpu.h snes/dma.h snes/ppu.h snes/cart.h snes/input.h snes/statehandler.h snes/snes.h \
 zip/zip.h zip/miniz.h tracing.h

.PHONY: all clean

all: $(execname)

$(execname): $(cfiles) $(hfiles)
	$(CC) $(CFLAGS) -o $@ $(cfiles) $(sdlflags)

$(appexecname): $(cfiles) $(hfiles)
	$(CC) $(CFLAGS) -o $@ $(cfiles) $(appsdlflags) -D SDL2SUBDIR

$(appname): $(appexecname)
	rm -rf $(appname)
	mkdir -p $(appname)/Contents/MacOS
	mkdir -p $(appname)/Contents/Frameworks
	mkdir -p $(appname)/Contents/Resources
	cp -R sdl2/SDL2.framework $(appname)/Contents/Frameworks/
	cp $(appexecname) $(appname)/Contents/MacOS/$(appexecname)
	cp resources/appicon.icns $(appname)/Contents/Resources/
	cp resources/PkgInfo $(appname)/Contents/
	cp resources/Info.plist $(appname)/Contents/

$(winexecname): $(cfiles) $(hfiles)
	$(WINDRES) resources/win.rc -O coff -o win.res
	$(CC) $(CFLAGS) -o $@ $(cfiles) win.res $(sdlflags)

clean:
	rm -f $(execname) $(appexecname) $(winexecname) win.res
	rm -rf $(appname)
