CC=g++
CFLAGS=-O0 -c -Wall -ggdb 
LDFLAGS=
SOURCES=rmemptyxmlpaths.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=RmEmptyXMLPaths

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)

