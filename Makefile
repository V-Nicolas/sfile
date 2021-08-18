
GCC=			/usr/bin/gcc

EXEC=			sfile

S=				./src/

CSOURCE=		$(S)sfile.c

OBJS=			$(CSOURCE:.c=.o)

CFLAGS=			-O2 -I src/ -W -Wall -Wextra \
				-pedantic -Wpedantic -std=c11 \
				-Wbad-function-cast \
				-Wcast-align \
				-Wcast-qual \
				-Wconversion  \
				-Wdate-time \
				-Wfloat-equal \
				-Wformat=2 \
				-Winit-self \
				-Wnested-externs \
				-Wnull-dereference \
				-Wold-style-definition \
				-Wpointer-arith \
				-Wshadow \
				-Wstack-protector \
				-Wstrict-prototypes \
				-Wswitch-default \
				-Wwrite-strings  \
				-Wmissing-prototypes \
				-Wformat-security \
				-fstack-protector-strong \
				-fPIE \
				-D_FORTIFY_SOURCE=2 -D_XOPEN_SOURCE=700 -DNDEBUG

ifeq ($(MACOS),yes)
  CFLAGS += 	-DMACOS
else
  CFLAGS += 	-Wduplicated-cond \
  				-Wformat-signedness \
  				-Wjump-misses-init \
  				-Wlogical-op \
  				-Wnormalized \
  				-Wsuggest-attribute=format \
  				-Wtrampolines \
  				-pie
endif

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
