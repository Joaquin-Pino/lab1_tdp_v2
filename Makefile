CXX = g++
CXXFLAGS = -c -O2 -Wall -Wextra

PIEZA       = pieza
SALIDA      = salida
COMPUERTA   = compuerta
ESTADO      = estado
MINHEAP     = minheap
TABLAHASH   = tablaHash
TABLERO     = tablero
PARSER      = parser
SOLVER      = solver
IMPRESORA   = impresora
VERIFICADOR = verificador

all: testPieza testSalida testCompuerta testEstado testMinHeap testTablaHash testTablero testParser testSolver testImpresora testVerificador main

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

verificador.o: $(VERIFICADOR)/verificador.cpp $(VERIFICADOR)/verificador.h
	$(CXX) $(CXXFLAGS) $(VERIFICADOR)/verificador.cpp -o verificador.o

main.o: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o main.o

# ── objetos de test ──────────────────────────────────

testPieza.o: $(PIEZA)/testPieza.cpp
	$(CXX) $(CXXFLAGS) $(PIEZA)/testPieza.cpp -o testPieza.o

testSalida.o: $(SALIDA)/testSalida.cpp
	$(CXX) $(CXXFLAGS) $(SALIDA)/testSalida.cpp -o testSalida.o

testCompuerta.o: $(COMPUERTA)/testCompuerta.cpp
	$(CXX) $(CXXFLAGS) $(COMPUERTA)/testCompuerta.cpp -o testCompuerta.o

testEstado.o: $(ESTADO)/testEstado.cpp
	$(CXX) $(CXXFLAGS) $(ESTADO)/testEstado.cpp -o testEstado.o

testMinHeap.o: $(MINHEAP)/testMinHeap.cpp
	$(CXX) $(CXXFLAGS) $(MINHEAP)/testMinHeap.cpp -o testMinHeap.o

testTablaHash.o: $(TABLAHASH)/testTablaHash.cpp
	$(CXX) $(CXXFLAGS) $(TABLAHASH)/testTablaHash.cpp -o testTablaHash.o

testTablero.o: $(TABLERO)/testTablero.cpp
	$(CXX) $(CXXFLAGS) $(TABLERO)/testTablero.cpp -o testTablero.o

testParser.o: $(PARSER)/testParser.cpp
	$(CXX) $(CXXFLAGS) $(PARSER)/testParser.cpp -o testParser.o

testSolver.o: $(SOLVER)/testSolver.cpp
	$(CXX) $(CXXFLAGS) $(SOLVER)/testSolver.cpp -o testSolver.o

testImpresora.o: $(IMPRESORA)/testImpresora.cpp
	$(CXX) $(CXXFLAGS) $(IMPRESORA)/testImpresora.cpp -o testImpresora.o

testVerificador.o: $(VERIFICADOR)/testVerificador.cpp
	$(CXX) $(CXXFLAGS) $(VERIFICADOR)/testVerificador.cpp -o testVerificador.o

# ── colección de .o compartida ──────────────────────

OBJS = pieza.o salida.o compuerta.o estado.o minheap.o tablaHash.o tablero.o parser.o solver.o impresora.o verificador.o

# ── binarios de test ─────────────────────────────────

testPieza: pieza.o testPieza.o
	$(CXX) pieza.o testPieza.o -o testPieza

testSalida: salida.o pieza.o testSalida.o
	$(CXX) salida.o pieza.o testSalida.o -o testSalida

testCompuerta: compuerta.o pieza.o testCompuerta.o
	$(CXX) compuerta.o pieza.o testCompuerta.o -o testCompuerta

testEstado: estado.o pieza.o testEstado.o
	$(CXX) estado.o pieza.o testEstado.o -o testEstado

testMinHeap: pieza.o salida.o compuerta.o estado.o minheap.o testMinHeap.o
	$(CXX) pieza.o salida.o compuerta.o estado.o minheap.o testMinHeap.o -o testMinHeap

testTablaHash: pieza.o salida.o compuerta.o estado.o tablaHash.o testTablaHash.o
	$(CXX) pieza.o salida.o compuerta.o estado.o tablaHash.o testTablaHash.o -o testTablaHash

testTablero: $(OBJS) testTablero.o
	$(CXX) $(OBJS) testTablero.o -o testTablero

testParser: $(OBJS) testParser.o
	$(CXX) $(OBJS) testParser.o -o testParser

testSolver: $(OBJS) testSolver.o
	$(CXX) $(OBJS) testSolver.o -o testSolver

testImpresora: $(OBJS) testImpresora.o
	$(CXX) $(OBJS) testImpresora.o -o testImpresora

testVerificador: $(OBJS) testVerificador.o
	$(CXX) $(OBJS) testVerificador.o -o testVerificador

main: $(OBJS) main.o
	$(CXX) $(OBJS) main.o -o main

# ── valgrind ─────────────────────────────────────────

valgrind_pieza: testPieza
	valgrind --leak-check=full --track-origins=yes ./testPieza

valgrind_salida: testSalida
	valgrind --leak-check=full --track-origins=yes ./testSalida

valgrind_compuerta: testCompuerta
	valgrind --leak-check=full --track-origins=yes ./testCompuerta

valgrind_estado: testEstado
	valgrind --leak-check=full --track-origins=yes ./testEstado

valgrind_minheap: testMinHeap
	valgrind --leak-check=full --track-origins=yes ./testMinHeap

valgrind_tablahash: testTablaHash
	valgrind --leak-check=full --track-origins=yes ./testTablaHash

valgrind_tablero: testTablero
	valgrind --leak-check=full --track-origins=yes ./testTablero

valgrind_parser: testParser
	valgrind --leak-check=full --track-origins=yes ./testParser

valgrind_solver: testSolver
	valgrind --leak-check=full --track-origins=yes ./testSolver

valgrind_impresora: testImpresora
	valgrind --leak-check=full --track-origins=yes ./testImpresora

valgrind_verificador: testVerificador
	valgrind --leak-check=full --track-origins=yes ./testVerificador

valgrind_main: main
	valgrind --leak-check=full --track-origins=yes ./main

clean:
	rm -f *.o testPieza testSalida testCompuerta testEstado testMinHeap testTablaHash testTablero testParser testSolver testImpresora testVerificador main
