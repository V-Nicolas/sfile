
GCC=			/usr/bin/gcc

EXEC=			sfile

S=			./src/

CSOURCE=                $(S)sfile.c

OBJS=			$(CSOURCE:.c=.o)

CFLAGS=                 -O2 -I src/ -W -Wall -Wextra -pedantic -std=c99 -Wbad-function-cast \
			-Wcast-align -Wcast-qual -Wconversion -Wdate-time -Wduplicated-cond \
			-Wfloat-equal -Wformat=2 -Wformat-signedness -Winit-self \
			-Wjump-misses-init -Wlogical-op -Wnested-externs -Wnormalized \
			-Wnull-dereference -Wold-style-definition -Wpointer-arith -Wshadow \
			-Wstack-protector -Wstrict-prototypes -Wsuggest-attribute=format \
			-Wswitch-default -Wtrampolines -Wmissing-prototypes -Wformat-security \
			-D_FORTIFY_SOURCE=2 -D_XOPEN_SOURCE=700 -DNDEBUG

LDFLAGS=

all:			$(OBJS) $(EXEC)

$(EXEC):
			$(GCC) -o $@ $(OBJS) $(LDFLAGS)

.c.o:
			$(GCC) $(CFLAGS) $(LDFLAGS) -o $@ -c $<

.PHONY: clean  distclean install uninstall re alias

install:
			@echo "install $(EXEC) in /usr/bin/ ..."
			cp $(EXEC) /usr/bin

uninstall:
			@echo "delete $(EXEC) in /usr/bin/ ..."
			rm /usr/bin/$(EXEC)

alias:
			@echo "alias sack='sfile --ack'" >> $(HOME)/.bashrc

clean:
			@rm ./src/*.o

distclean:
			@rm $(EXEC)

re:
			-make clean
			-make distclean
			make
