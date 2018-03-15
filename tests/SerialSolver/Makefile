CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wshadow -Werror -O3


TARGET = serialSolver
OBJS = $(TARGET).o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(TARGET).o: $(TARGET).cpp
	$(CXX) -c $(CXXFLAGS) $(TARGET).cpp

clean:
	@$(RM) -rf *.o $(TARGET)
