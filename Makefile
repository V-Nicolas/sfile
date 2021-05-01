
GCC=			/usr/bin/gcc

EXEC=			sfile

S=			./src/

CSOURCE=                $(S)sfile.c

OBJS=			$(CSOURCE:.c=.o)

CFLAGS=                 -O2 -I src/ -Wall -W
LDFLAGS=

all:			$(OBJS) $(EXEC)

$(EXEC):
			$(GCC) -o $@ $(OBJS) $(LDFLAGS)

.c.o:
			$(GCC) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

.PHONY: clean  distclean install uninstall

install:
			@echo "install $(EXEC) in /usr/bin/ ..."
			cp $(EXEC) /usr/bin

uninstall:
			@echo "delete $(EXEC) in /usr/bin/ ..."
			rm /usr/bin/$(EXEC)

clean:
			@rm ./src/*.o

distclean:
			@rm $(EXEC)
