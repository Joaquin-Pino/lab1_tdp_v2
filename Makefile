CXX = g++
CXXFLAGS = -c -g -Wall -Wextra

PIEZA     = pieza
SALIDA    = salida
COMPUERTA = compuerta
ESTADO    = estado
MINHEAP   = minheap
TABLAHASH = tablaHash
TABLERO   = tablero
PARSER    = parser
SOLVER    = solver
IMPRESORA = impresora

all: testMinHeap testTablaHash testSolver main

pieza.o: $(PIEZA)/pieza.cpp $(PIEZA)/pieza.h
	$(CXX) $(CXXFLAGS) $(PIEZA)/pieza.cpp -o pieza.o

salida.o: $(SALIDA)/salida.cpp $(SALIDA)/salida.h
	$(CXX) $(CXXFLAGS) $(SALIDA)/salida.cpp -o salida.o

compuerta.o: $(COMPUERTA)/compuerta.cpp $(COMPUERTA)/compuerta.h
	$(CXX) $(CXXFLAGS) $(COMPUERTA)/compuerta.cpp -o compuerta.o

estado.o: $(ESTADO)/estado.cpp $(ESTADO)/estado.h
	$(CXX) $(CXXFLAGS) $(ESTADO)/estado.cpp -o estado.o

minheap.o: $(MINHEAP)/minheap.cpp $(MINHEAP)/minheap.h
	$(CXX) $(CXXFLAGS) $(MINHEAP)/minheap.cpp -o minheap.o

tablaHash.o: $(TABLAHASH)/tablaHash.cpp $(TABLAHASH)/tablaHash.h
	$(CXX) $(CXXFLAGS) $(TABLAHASH)/tablaHash.cpp -o tablaHash.o

tablero.o: $(TABLERO)/tablero.cpp $(TABLERO)/tablero.h
	$(CXX) $(CXXFLAGS) $(TABLERO)/tablero.cpp -o tablero.o

parser.o: $(PARSER)/parser.cpp $(PARSER)/parser.h
	$(CXX) $(CXXFLAGS) $(PARSER)/parser.cpp -o parser.o

solver.o: $(SOLVER)/solver.cpp $(SOLVER)/solver.h
	$(CXX) $(CXXFLAGS) $(SOLVER)/solver.cpp -o solver.o

impresora.o: $(IMPRESORA)/impresora.cpp $(IMPRESORA)/impresora.h
	$(CXX) $(CXXFLAGS) $(IMPRESORA)/impresora.cpp -o impresora.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o main.o

testMinHeap.o: $(MINHEAP)/testMinHeap.cpp
	$(CXX) $(CXXFLAGS) $(MINHEAP)/testMinHeap.cpp -o testMinHeap.o

testTablaHash.o: $(TABLAHASH)/testTablaHash.cpp
	$(CXX) $(CXXFLAGS) $(TABLAHASH)/testTablaHash.cpp -o testTablaHash.o

testSolver.o: $(SOLVER)/testSolver.cpp
	$(CXX) $(CXXFLAGS) $(SOLVER)/testSolver.cpp -o testSolver.o

OBJS = pieza.o salida.o compuerta.o estado.o minheap.o tablaHash.o \
       tablero.o parser.o solver.o impresora.o

testMinHeap: pieza.o salida.o compuerta.o estado.o minheap.o testMinHeap.o
	$(CXX) pieza.o salida.o compuerta.o estado.o minheap.o testMinHeap.o -o testMinHeap

testTablaHash: pieza.o salida.o compuerta.o estado.o tablaHash.o testTablaHash.o
	$(CXX) pieza.o salida.o compuerta.o estado.o tablaHash.o testTablaHash.o -o testTablaHash

testSolver: $(OBJS) testSolver.o
	$(CXX) $(OBJS) testSolver.o -o testSolver

main: $(OBJS) main.o
	$(CXX) $(OBJS) main.o -o main

valgrind_minheap: testMinHeap
	valgrind --leak-check=full --track-origins=yes ./testMinHeap

valgrind_tablahash: testTablaHash
	valgrind --leak-check=full --track-origins=yes ./testTablaHash

valgrind_solver: testSolver
	valgrind --leak-check=full --track-origins=yes ./testSolver

valgrind_main: main
	valgrind --leak-check=full --track-origins=yes ./main

clean:
	rm -f *.o testMinHeap testTablaHash testSolver main
