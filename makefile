
CC = @g++
LD = @g++
CFLAGS = -c -Wall -Wextra -Wwrite-strings
IFLAGS = -I/StdFuncs
LFLAGS = -lStdFuncs -lauto

ifdef DEBUG
	OBJ = Debug
	CFLAGS += -ggdb -D_DEBUG
else
	OBJ = Release
	CFLAGS += -O2
endif

LFLAGS += -L/StdFuncs/$(OBJ)

EXECUTABLE = $(OBJ)/ls

OBJECTS = $(OBJ)/ls.o

All: $(OBJ) $(EXECUTABLE)

$(OBJ):
	@MakeDir $(OBJ)

$(EXECUTABLE): $(OBJECTS)
	@echo Linking $@...
	$(LD) $(OBJECTS) $(LFLAGS) -o $@.debug
	@strip -R.comment $@.debug -o $@

$(OBJ)/%.o: %.cpp
	@echo Compiling $<...
	$(CC) $(CFLAGS) $(IFLAGS) -o $(OBJ)/$*.o $<

clean:
	@Delete $(OBJ) all quiet
