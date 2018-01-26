CXX = g++
CXXFLAGS1 = -pthread -std=c++11 -Wall -Wextra -Wshadow -O3 
CXXFLAGS2 = -std=c++11 -Wall -Wextra -Wshadow -O3
#-fopenmp -DNDEBUG -Werror -pipe 

TARGET1 = main
TARGET2 = task
TARGET3 = util
TARGETG = grid


#all: $(TARGET1) $(TARGET2)

OBJS = $(TARGET3).o $(TARGET2).o $(TARGETG).o  $(TARGET1).o
EXEC = task_parallel_jacobi

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS1) $(OBJS) -o $(EXEC)

$(TARGET1).o: $(TARGET1).cpp
	$(CXX) -c $(CXXFLAGS1) $(TARGET1).cpp

$(TARGET2).o: $(TARGET2).cpp
	$(CXX) -c $(CXXFLAGS2) $(TARGET2).cpp

$(TARGET3).o: $(TARGET3).cpp
	$(CXX) -c $(CXXFLAGS2) $(TARGET3).cpp

$(TARGETG).o: $(TARGETG).cpp
	$(CXX) -c $(CXXFLAGS2) $(TARGETG).cpp

clean:
	@$(RM) -rf *.o $(EXEC) 
