
CC = @g++
LD = @g++
CFLAGS = -c -fno-asynchronous-unwind-tables -fno-exceptions -Wall -Wextra -Wwrite-strings
IFLAGS = -I../StdFuncs
LFLAGS = -fno-asynchronous-unwind-tables -fno-exceptions
LIBS = -lStdFuncs

ifdef DEBUG
	OBJ = Debug
	CFLAGS += -ggdb -D_DEBUG
else
	OBJ = Release
	CFLAGS += -O2
endif

UNAME = $(shell uname)

ifeq ($(UNAME), AmigaOS)

LIBS += -lauto

endif

LFLAGS += -L../StdFuncs/$(OBJ)

EXECUTABLE = $(OBJ)/ls

OBJECTS = $(OBJ)/ls.o

All: $(OBJ) $(EXECUTABLE)

$(OBJ):
	@mkdir $(OBJ)

$(EXECUTABLE): $(OBJECTS) ../StdFuncs/$(OBJ)/libStdFuncs.a
	@echo Linking $@...
	$(LD) $(OBJECTS) $(LFLAGS) $(LIBS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(OBJ)/%.o: %.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

clean:
	@rm -fr $(OBJ)
