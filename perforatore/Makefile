CXX = g++
CXXFLAGS = -g -Wall -O -std=c++11
LDADD =

CPX_INCDIR  = /opt/ibm/ILOG/CPLEX_Studio126/cplex/include
CPX_LDPATH  = /opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic
CPX_LDADD = -lcplex -lm -pthread

OBJ = perforatore.o TwoOptSolver.o Miosolver.o Nearsolver.o CPLEXsolver.o solver.o soluzione.o istanza.o segmento.o punto.o 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(CPX_INCDIR) -c $^ -o $@

main: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o perforatore -L$(CPX_LDPATH) $(CPX_LDADD)

istanze: creaIstanze.o punto.o
	$(CXX) $(CXXFLAGS) -o istanze creaIstanze.o punto.o

clean:
	rm -rf $(OBJ) perforatore punto.o segmento.o istanza.o soluzione.o CPLEXsolver.o Nearsolver.o Miosolver.o TwoOptSolver.o

.PHONY: clean
