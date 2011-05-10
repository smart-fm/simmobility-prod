CXX = g++
CXXFLAGS = -DDEBUG=1 -I.

OBJS = data_mgr.o

TARGETS = main

all: $(TARGETS)

main: main.o $(OBJS)
	$(CXX) -o main main.o $(OBJS)

clean:	tidy
	-rm -f $(TARGETS) main.o $(OBJS)

tidy:
	-rm -f *~
