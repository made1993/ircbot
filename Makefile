# # FILE: Makefile

# #=============================================================
# # Programming environment and options
# #=============================================================


# # Compiler
        CC = gcc

# # Compiling options
        CFLAGS = -Wall -Wextra -std=gnu99 -pedantic -g -O3 -Wno-unused-parameter #-Wno-unused-result
        LIBS = -lpthread -lncurses

# # Source code folder
        DIR = $(shell basename "$(CURDIR)")
        SRCDIR = bot
        SOURCES = $(wildcard $(SRCDIR)/*.c)
        OBJS = $(SOURCES:.c=.o)
        INCLUDES = $(SRCDIR)/funciones.h 
        TARFLAG = -cvzf
        TARNAME = bot.tgz
# # Variables
        SEP = "\n------------------------------------------------------------------\n\n------------------------------------------------------------------\n"
        AUTHORS = $(SEP) "Authors: "$(SEP)

# #=============================================================
# # Executables
# #=============================================================

        EXEC = ircbot 
         
# #=============================================================
# # Targets
# #=============================================================

all: clean $(EXEC)
	mkdir ~/logs

# #*************************************************************
# # Executable files
# #*************************************************************

ircbot: $(OBJS)
	@echo -e "Building" $@...
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS)



# #*************************************************************
# # Object files (.o)
# #*************************************************************

.c.o: 
	@echo "Compiling" $<...
	$(CC) $(CFLAGS) -c $< $(LIBS) -o $@

# #=============================================================
# # Create a compressed folder with the project
# #=============================================================

compress: clean
	cd ..; tar $(TARFLAG) $(TARNAME) $(DIR); cd -


# #=============================================================
# # Install/Uninstall
# #=============================================================

install: all
	cp ircbot /usr/bin/

uninstall: clean
	rm -rf /usr/bin/ircbot

# #=============================================================
# # Clean .o and executable files
# #=============================================================

clean:	
	rm -rf *.c~ *~ $(SRCDIR)/*.o $(SRCDIR)/*.c~ $(SRCDIR)/*.h~ $(SRCDIR)/*.o~ $(EXEC) $(OBJS)
	rm -rf ~/logs


# #=============================================================
# # Generate documentation
# #=============================================================


# #=============================================================
# # Display authors
# #=============================================================

authors:
	@echo -e $(AUTHORS)
