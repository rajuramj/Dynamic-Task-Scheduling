#**
#    Makefile
#	 Compile the static scheduling case of task thread framework

#    @author Raju Ram
#    @version 1.0 04/07/18
#

#Compiler flags
CXX = g++
CXXFLAGS1 = -pthread -std=c++11 -Wall -Wextra -Wshadow #-Werror
#-fopenmp -DNDEBUG -Werror -pipe 


# gdb debug switch
GDB_DBG = 0

ifeq ($(GDB_DBG),1)
	#-v -da -Q to generate compiler time files-> helpful in debug for example segfault 
	#CXXFLAGS1 += -g -v -da -Q -Og
	CXXFLAGS1 += -g -O0
else
	CXXFLAGS1 += -O3
endif


TARGET1 = staticDecomposition


OBJS = $(TARGET1).o
EXEC = exec

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS1) $(TARGET1).o  -o $(EXEC)

$(TARGET1).o: $(TARGET1).cpp
	$(CXX) -c $(CXXFLAGS1) $(TARGET1).cpp  -o $(TARGET1).o
	
clean:
	@$(RM) -rf *.o ./src/*.o $(EXEC) 
