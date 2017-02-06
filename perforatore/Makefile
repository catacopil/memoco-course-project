CXX = g++
CXXFLAGS = -g -Wall -O -std=c++11
LDADD =

CPX_INCDIR  = /opt/ibm/ILOG/CPLEX_Studio126/cplex/include
CPX_LDPATH  = /opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic
CPX_LDADD = -lcplex -lm -pthread

OBJ = perforatore.o soluzione.o istanza.o punto.o 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(CPX_INCDIR) -c $^ -o $@

main: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o perforatore -L$(CPX_LDPATH) $(CPX_LDADD)

istanze: creaIstanze.o punto.o
	$(CXX) $(CXXFLAGS) -o istanze creaIstanze.o punto.o

clean:
	rm -rf $(OBJ) perforatore istanza.o soluzione.o

.PHONY: clean
