CXX = g++
CXXFLAGS = -g -Wall -O -std=c++11
LDADD =

CPX_INCDIR  = /opt/ibm/ILOG/CPLEX_Studio126/cplex/include
CPX_LDPATH  = /opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic
CPX_LDADD = -lcplex -lm -pthread

OBJ = mixIstanza.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(CPX_INCDIR) -c $^ -o $@

leggiIstanze: mixIstanza.o punto.o
	$(CXX) $(CXXFLAGS) -o leggi mixIstanza.o punto.o

creaIstanze: creaIstanze.o punto.o
	$(CXX) $(CXXFLAGS) -o crea creaIstanze.o punto.o

clean:
	rm -rf $(OBJ)

.PHONY: clean
