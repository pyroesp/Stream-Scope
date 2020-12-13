
CXX = gcc

EXE = stream_scope
VISA_INC = "C:/Program Files/IVI Foundation/VISA/Win64/Include/"
VISA_LIB = "C:/Program Files/IVI Foundation/VISA/Win64/Lib_x64/msc/"
SOURCES = main.c
SOURCES += gfx.c scope.c DS1052E.c
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

CXXFLAGS = 
CXXFLAGS += -g -Wall -Wformat
CXXFLAGS += -I$(VISA_INC)
CXXFLAGS += -L$(VISA_LIB)
LIBS =
LIBS += -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
LIBS += -lvisa32


%.o:%.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
