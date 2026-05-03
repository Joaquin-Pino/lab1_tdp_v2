#pragma once
#include "../common/coordenada.h"

// Representa una compuerta dinámica del tablero.
// Su color activo oscila entre Ci y Cf cada `paso` pasos, calculado por Tablero::calcularColorCompuerta.
// El solver almacena el color actual en Estado::colorCompuertas para no recalcularlo en cada consulta.
class Compuerta{
private:
    int id;
    coordenada pos;
    short tamano;
    bool esVertical;
    short Ci; // color inicial
    short Cf; // color final
    short paso; // cada cuántos steps cambia de color (Ci → Cf → Ci → ...)

public:
    Compuerta();
    Compuerta(int id, coordenada pos, short tamano, bool esVertical, short Ci, short Cf, short paso);
    ~Compuerta();

    int getId() const;
    coordenada getPos() const;
    short getTamano() const;
    bool getEsVertical() const;
    short getCi() const;
    short getCf() const;
    short getPaso() const;

    // Determina si una pieza puede cruzar la compuerta en el estado actual.
    // Requiere que tanto el color como el tamaño sean compatibles:
    // una pieza del color correcto pero más ancha que la apertura tampoco puede pasar.
    bool aceptaBloque(int colorBloque, int tamanoBloque, int colorActual) const;

    // Devuelve la posición de la i-ésima celda de la compuerta,
    // avanzando en la dirección perpendicular a su orientación.
    coordenada getCeldaPos(int i) const;
};