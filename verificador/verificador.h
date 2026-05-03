#pragma once
#include "../tablero/tablero.h"
#include "../estado/estado.h"

// Valida una secuencia de movimientos ingresada por el usuario contra el estado inicial.
// Reutiliza los mismos métodos del Tablero y Estado que usa el Solver,
// por lo que cualquier estado válido para el Verificador también lo es para el Solver.
//
// El Verificador no toma ownership del Tablero.
class Verificador {
private:
    Tablero* tablero;

    // Lee el siguiente token de la secuencia a partir de la posición `idx`.
    // Un token es "S<id>" (salida) o "<dir><id>,<dist>" (movimiento).
    // Avanza `idx` al carácter siguiente al token leído.
    // Devuelve false si el formato es inválido o si se llegó al final de la cadena.
    bool parsearToken(const char* str, int& idx,
                      char& dir, int& pieza, int& dist, bool& esSalida);

    // Aplica `dist` pasos de 1 celda en dirección `dir` para la pieza con id externo `pieza`.
    // Valida cada paso con Tablero::piezaPuedeMoverse y actualiza los elementos dinámicos.
    // Devuelve false si algún paso es inválido.
    bool aplicarMovimiento(Estado* estado, char dir, int pieza, int dist);

    // Saca la pieza con id externo `pieza` del tablero.
    // Valida con Tablero::piezaPuedeSalir antes de aplicar.
    // Devuelve false si la pieza no puede salir en ese estado.
    bool aplicarSalida(Estado* estado, int pieza);

public:
    Verificador(Tablero* t);
    Verificador(const Verificador&) = delete;
    Verificador& operator=(const Verificador&) = delete;
    ~Verificador();

    // Simula la secuencia completa sobre una copia del estado inicial.
    // Devuelve true solo si todas las piezas salieron y stepUsed <= stepLimit.
    bool verificarSolucion(const char* secuencia, const Estado* estadoInicial);
};
