CC=g++
LDFLAGS=-L.
LIBNAME=TripEnergySO
MAINFILE=main.cpp
EXECUTABLE=exec

exec:
	$(CC) -static $(MAINFILE) $(LDFLAGS) -l$(LIBNAME) -o $(EXECUTABLE)

clean:
	rm  $(EXECUTABLE)
