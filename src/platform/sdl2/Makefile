SRCS=main.cpp \
     ../../libs/stb_vorbis/stb_vorbis.c \
     ../../libs/minimp3/minimp3.cpp \
     ../../libs/tinf/tinflate.c
CC=g++

SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

CFLAGS+=-D_SDL2_OPENGL -DSDL2_SW -std=c++11 -O3 -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -D__SDL2__ $(SDL_CFLAGS) -Wall
LDFLAGS+=-framework OpenGL -lm -lpthread $(SDL_LDFLAGS)

INCLUDES+=-I../../

openlara :
	$(CC) $(INCLUDES) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $@

install : openlara
	install -d $(DESTDIR)$(PREFIX)/bin/
	install openlara $(DESTDIR)$(PREFIX)/bin/OpenLara

uninstall :
	rm -f $(DESTDIR)$(PREFIX)/bin/OpenLara

clean:
	rm -f OpenLara

