PROJECT = winalttab
INCDIR  = -I./lua-5.1.4/src
LIBS    = ./lua-5.1.4/src/lua51.dll

CC      = gcc
CFLAGS  = -Os -Wall -c
LFLAGS  =  -shared
SHLIBSUFFIX = .dll

all:  $(PROJECT)$(SHLIBSUFFIX)

$(PROJECT)$(SHLIBSUFFIX): $(PROJECT).o
	$(CC) -o $@ $(LFLAGS) $^ $(LIBS) -lpsapi
	strip $(PROJECT)$(SHLIBSUFFIX)

$(PROJECT).o:  $(PROJECT).c

.c.o:
	$(CC) $(CFLAGS) $(INCDIR) -c $<
clean:
	rm -f *.o *~ $(PROJECT)$(SHLIBSUFFIX)

