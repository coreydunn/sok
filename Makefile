CC=      cc
CFLAGS=  -Wfatal-errors $(shell pkgconf --cflags ncurses)
LDFLAGS= -no-pie -s $(shell pkgconf --libs ncurses)
OBJS=     src/config.h maps/incmaps.o src/main.o src/inc.o

all: sok cre/cre
sok: src/sok
	[ ! -e $@ ] && ln -s $^ || true
src/sok: $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
src/inc.o: src/inc.asm
	nasm -I src -felf64 $^
maps/incmaps.o: maps/incmaps.asm
	nasm -I maps -felf64 $^
maps/incmaps.asm:
	./maps/makeasm.sh maps maps/maps.h > $@
cre/cre: cre/main.c
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
clean:
	$(RM) *.o sok cre/cre
	$(RM) src/*.o src/sok
	$(RM) maps/incmaps.asm maps/incmaps.o maps/maps.h
.PHONY: all clean
