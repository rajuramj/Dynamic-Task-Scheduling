

#Compiler flags
CXX = g++
CXXFLAGS1 = -pthread -std=c++11 -Wall -Wextra -Wshadow #-Werror
CXXFLAGS2 = -std=c++11 -Wall -Wextra -Wshadow #-Werror
#-fopenmp -DNDEBUG -Werror -pipe 


# gdb debug switch
GDB_DBG = 0

ifeq ($(GDB_DBG),1)
	#-v -da -Q to generate compiler time files-> helpful in debug for example segfault 
	#CXXFLAGS1 += -g -v -da -Q -Og
	CXXFLAGS1 += -g -O0
	CXXFLAGS2 += -g -O0
else
	CXXFLAGS1 += -O3
        CXXFLAGS2 += -O3
endif


TARGET1 = main
TARGET2 = task
TARGET3 = util
TARGETG = grid



OBJS = $(TARGET3).o $(TARGETG).o $(TARGET2).o  $(TARGET1).o
EXEC = hpc_task_parallel_jacobi_socket0

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS1) ./src/$(TARGET3).o  ./src/$(TARGETG).o ./src/$(TARGET2).o  ./src/$(TARGET1).o  -o $(EXEC)

$(TARGET1).o: ./src/$(TARGET1).cpp
	$(CXX) -c $(CXXFLAGS1) ./src/$(TARGET1).cpp -I ./include/ -o ./src/$(TARGET1).o

$(TARGET2).o: ./src/$(TARGET2).cpp
	$(CXX) -c $(CXXFLAGS2) ./src/$(TARGET2).cpp  -I ./include/ -o  ./src/$(TARGET2).o

$(TARGET3).o: ./src/$(TARGET3).cpp
	$(CXX) -c $(CXXFLAGS2) ./src/$(TARGET3).cpp -I ./include/ -o ./src/$(TARGET3).o 

$(TARGETG).o: ./src/$(TARGETG).cpp
	$(CXX) -c $(CXXFLAGS2) ./src/$(TARGETG).cpp  -I ./include/ -o ./src/$(TARGETG).o

clean:
	@$(RM) -rf *.o ./src/*.o $(EXEC) 
