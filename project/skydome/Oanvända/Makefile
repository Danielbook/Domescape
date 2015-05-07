
CC= g++
CFLAGS= -g -Wall -ansi -pedantic

TARG= demo
OBJS= opengl.o obj.o bugs.o sky.o earth.o scene.o light.o \
	camera.o entity.o image.o data.o conf.o demo.o app.o main.o

LIBS= -lode -ljpeg -lpng -lz -lm
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

INCDIR= -I$(HOME)/include
LIBDIR= -L$(HOME)/lib

ifeq ($(shell uname), Darwin)
	CFLAGS += $(shell /sw/bin/sdl-config --cflags) -I/sw/include
        LIBS   += $(shell /sw/bin/sdl-config --libs)
else
        CFLAGS += $(shell sdl-config --cflags)
        LIBS   += $(shell sdl-config --libs) -lGLU -lGL
endif

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $< > /dev/null

%.d : %.cpp
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $< > /dev/null

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(INCDIR) -c $<

%.o : %.cpp Makefile
	$(CC) $(CFLAGS) $(INCDIR) -c $<

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	rm -f $(TARG) $(OBJS) $(DEPS)

#------------------------------------------------------------------------------

-include $(DEPS)
