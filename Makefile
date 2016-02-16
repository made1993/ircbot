CC = gcc

CFLAGS = -Wall -Wextra -std=gnu99 -pedantic -g -O3 -Wno-unused-parameter #-Wno-unused-result
LIBS = -lpthread -lncurses

DIR = $(shell basename "$(CURDIR)")
SRCDIR = bot
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SOURCES:.c=.o)
INCLUDES = $(SRCDIR)/bot.h
TARFLAG = -cvzf
TARNAME = bot.tgz

EXEC = ircbot

all: clean $(EXEC)
	mkdir ~/logs

ircbot: $(OBJS)
	@echo -e "Building" $@...
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS)

.c.o:
	@echo "Compiling" $<...
	$(CC) $(CFLAGS) -c $< $(LIBS) -o $@

compress: clean
	cd ..; tar $(TARFLAG) $(TARNAME) $(DIR); cd -

install: all
	cp ircbot /usr/bin/

uninstall: clean
	rm -rf /usr/bin/ircbot

clean:
	rm -rf *.c~ *~ $(SRCDIR)/*.o $(SRCDIR)/*.c~ $(SRCDIR)/*.h~ $(SRCDIR)/*.o~ $(EXEC) $(OBJS)
	rm -rf ~/logs
