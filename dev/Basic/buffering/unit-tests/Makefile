# Copyright Singapore-MIT Alliance for Research and Technology

INCLUDES = -I. -I..
CXXFLAGS = -g -Wall -Wextra -fmessage-length=0 $(INCLUDES)

OBJS = BufferedUnitTests.o main.o

MODULE_OBJS_DIR = ../../Debug/buffering
MODULE_OBJS = $(MODULE_OBJS_DIR)/BufferedDataManager.o

LIBS = -lcppunit

TARGET = BufferedUnitTests

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(MODULE_OBJS) $(LIBS)

all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *~

BufferedUnitTests.o: \
    BufferedUnitTests.hpp \
    BufferedUnitTests.cpp \
    ../BufferedDataManager.hpp \
    ../Buffered.hpp \
    ../Buffered_uint32.hpp \
    ../Vector2D.hpp

main.o: main.cpp

$(MODULE_OBJS_DIR)/BufferedDataManager.o : ../BufferedDataManager.hpp ../BufferedDataManager.cpp
