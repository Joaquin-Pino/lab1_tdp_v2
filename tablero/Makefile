CXX = g++
CXXFLAGS = -c -g -Wall -Wextra

PIEZA     = ../pieza
SALIDA    = ../salida
COMPUERTA = ../compuerta
ESTADO    = ../estado
PARSER    = ../parser

all: testTablero

pieza.o: $(PIEZA)/pieza.cpp $(PIEZA)/pieza.h
	$(CXX) $(CXXFLAGS) $(PIEZA)/pieza.cpp -o pieza.o

salida.o: $(SALIDA)/salida.cpp $(SALIDA)/salida.h
	$(CXX) $(CXXFLAGS) $(SALIDA)/salida.cpp -o salida.o

compuerta.o: $(COMPUERTA)/compuerta.cpp $(COMPUERTA)/compuerta.h
	$(CXX) $(CXXFLAGS) $(COMPUERTA)/compuerta.cpp -o compuerta.o

estado.o: $(ESTADO)/estado.cpp $(ESTADO)/estado.h
	$(CXX) $(CXXFLAGS) $(ESTADO)/estado.cpp -o estado.o

tablero.o: tablero.cpp tablero.h
	$(CXX) $(CXXFLAGS) tablero.cpp -o tablero.o

testTablero.o: testTablero.cpp
	$(CXX) $(CXXFLAGS) testTablero.cpp -o testTablero.o

testTablero: pieza.o salida.o compuerta.o estado.o tablero.o testTablero.o
	$(CXX) pieza.o salida.o compuerta.o estado.o tablero.o testTablero.o -o testTablero

valgrind: testTablero
	valgrind --leak-check=full --track-origins=yes ./testTablero

parser.o: $(PARSER)/parser.cpp $(PARSER)/parser.h
	$(CXX) $(CXXFLAGS) $(PARSER)/parser.cpp -o parser.o
	
testParser.o: $(PARSER)/testParser.cpp
	$(CXX) $(CXXFLAGS) $(PARSER)/testParser.cpp -o testParser.o

testParser: pieza.o salida.o compuerta.o estado.o tablero.o parser.o testParser.o
	$(CXX) pieza.o salida.o compuerta.o estado.o tablero.o parser.o testParser.o -o testParser

clean:
	rm -f *.o testTablero testParser
