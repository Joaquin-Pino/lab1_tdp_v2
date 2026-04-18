#include <iostream>
#include "compuerta.h"


int main() {
    // aunque trivial, test de todo lo de compuerta
    Compuerta c1(1, {0, 0}, 10, true, 1, 2, 1);
    
    std::cout << "Compuerta c1: " << std::endl;
    std::cout << "Posición: (" << c1.getPos().x << ", " << c1.getPos().y << ")" << std::endl;
    std::cout << "Tamaño: " << c1.getTamano() << std::endl;
    std::cout << "Es vertical: " << (c1.getEsVertical() ? "Sí" : "No") << std::endl;
    std::cout << "Color inicial: " << c1.getCi() << std::endl;
    std::cout << "Color final: " << c1.getCf() << std::endl;
    std::cout << "Paso: " << c1.getPaso() << std::endl; 

    // prueba de aceptaBloque
    std::cout << "Acepta bloque (color 1, tamaño 5, color actual 1): " << (c1.aceptaBloque(1, 5, 1) ? "Sí" : "No") << std::endl;
    std::cout << "Acepta bloque (color 1, tamaño 5, color actual 2): " << (c1.aceptaBloque(1, 5, 2) ? "Sí" : "No") << std::endl;
    std::cout << "Acepta bloque (color 2, tamaño 5, color actual 1): " << (c1.aceptaBloque(2, 5, 1) ? "Sí" : "No") << std::endl;
    


    return 0;
}