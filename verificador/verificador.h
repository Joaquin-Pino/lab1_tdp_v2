#pragma once
#include "../tablero/tablero.h"
#include "../estado/estado.h"

class Verificador {
private:
    Tablero* tablero;

    bool parsearToken(const char* str, int& idx,
                      char& dir, int& pieza, int& dist, bool& esSalida);
    bool aplicarMovimiento(Estado* estado, char dir, int pieza, int dist);
    bool aplicarSalida(Estado* estado, int pieza);

public:
    Verificador(Tablero* t);
    Verificador(const Verificador&) = delete;
    Verificador& operator=(const Verificador&) = delete;
    ~Verificador();

    bool verificarSolucion(const char* secuencia, const Estado* estadoInicial);
};