CXX = g++
CXXFLAGS = -DDEBUG=1 -I.

LIBS =

TARGETS = main

all: $(TARGETS)

main: main.o $(LIBS)
	$(CXX) -o main main.o $(LIBS)

clean:	tidy
	-rm -f $(TARGETS) main.o $(LIBS)

tidy:
	-rm -f *~
