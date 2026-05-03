#include <iostream>
#include <cstring>
#include "tablero/tablero.h"
#include "impresora/impresora.h"
#include "solver/solver.h"
#include "parser/parser.h"
#include "verificador/verificador.h"

void menuPrincipal() {
    std::cout << "\n=== COLOR BLOCK JAM SOLVER ===" << std::endl;
    std::cout << "1. Cargar archivo de configuracion" << std::endl;
    std::cout << "2. Resolver" << std::endl;
    std::cout << "3. Mostrar tablero" << std::endl;
    std::cout << "4. Mostrar solucion paso a paso" << std::endl;
    std::cout << "5. Verificar secuencia de movimientos" << std::endl;
    std::cout << "6. Salir" << std::endl;
    std::cout << "Opcion: ";
}

int main() {
    Tablero* tablero       = nullptr;
    Estado*  estadoInicial = nullptr;
    Estado** solucion      = nullptr; // arreglo terminado en nullptr; cada elemento es un Estado*
    char     nombreArchivo[256];

    int opcion = 0;

    while (opcion != 6) {
        menuPrincipal();
        std::cin >> opcion; // TODO: validar input — si se ingresa un string, cin queda en mal estado

        switch (opcion) {
            case 1: {
                std::cout << "Ingrese el nombre del archivo: ";
                std::cin >> nombreArchivo;

                // Liberar los objetos del mapa anterior antes de cargar uno nuevo,
                // para no perder las referencias a la memoria ya asignada.
                delete tablero;
                delete estadoInicial;
                if (solucion) {
                    for (int i = 0; solucion[i] != nullptr; i++) delete solucion[i];
                    delete[] solucion;
                }
                tablero       = nullptr;
                estadoInicial = nullptr;
                solucion      = nullptr;

                Parser parser(nombreArchivo);
                tablero = parser.construirTablero();

                if (!tablero) {
                    std::cout << "Error: no se pudo cargar el archivo." << std::endl;
                    break;
                }

                estadoInicial = tablero->crearEstadoInicial();
                std::cout << "Tablero cargado correctamente." << std::endl;
                Impresora::imprimirEstado(*tablero, *estadoInicial);
                break;
            }

            case 2: {
                if (!tablero || !estadoInicial) {
                    std::cout << "Primero cargue un archivo de configuracion." << std::endl;
                    break;
                }

                // Liberar la solución anterior si existía.
                if (solucion) {
                    for (int i = 0; solucion[i] != nullptr; i++) delete solucion[i];
                    delete[] solucion;
                    solucion = nullptr;
                }

                std::cout << "Resolviendo..." << std::endl;

                clock_t inicio = clock();

                Solver solver(tablero);
                // El Solver toma ownership del estado que recibe; pasamos una copia
                // para preservar estadoInicial y poder seguir usándolo (mostrar, verificar).
                Estado* estadoCopia = new Estado(*estadoInicial);
                solucion = solver.resolver(estadoCopia);

                clock_t fin    = clock();
                double  tiempoMs = (double)(fin - inicio) / CLOCKS_PER_SEC * 1000.0;

                if (solucion) {
                    std::cout << "Solucion encontrada." << std::endl;
                    std::cout << "Tiempo de resolucion: " << tiempoMs << "[mseg]" << std::endl;
                    Impresora::imprimirSolucion(*tablero, solucion);
                } else {
                    std::cout << "Juego sin solucion." << std::endl;
                    std::cout << "Tiempo: " << tiempoMs << "[mseg]" << std::endl;
                }
                break;
            }

            case 3: {
                if (!tablero || !estadoInicial) {
                    std::cout << "Primero cargue un archivo de configuracion." << std::endl;
                    break;
                }
                Impresora::imprimirEstado(*tablero, *estadoInicial);
                break;
            }

            case 4: {
                if (!tablero || !estadoInicial) {
                    std::cout << "Primero cargue un archivo de configuracion." << std::endl;
                    break;
                }
                if (!solucion) {
                    std::cout << "Primero resuelva el tablero (opcion 2)." << std::endl;
                    break;
                }
                Impresora::imprimirSolucionPasoAPaso(*tablero, solucion);
                break;
            }

            case 5: {
                if (!tablero || !estadoInicial) {
                    std::cout << "Primero cargue un archivo de configuracion." << std::endl;
                    break;
                }
                std::cout << "Ingrese la secuencia de movimientos: ";
                char secuencia[4096];
                std::cin >> secuencia;

                Verificador verificador(tablero);
                bool valido = verificador.verificarSolucion(secuencia, estadoInicial);
                if (valido)
                    std::cout << "Secuencia valida." << std::endl;
                else
                    std::cout << "Secuencia invalida." << std::endl;
                break;
            }

            case 6: {
                std::cout << "Saliendo..." << std::endl;
                break;
            }

            default: {
                std::cout << "Opcion invalida." << std::endl;
                break;
            }
        }
    }

    // Liberar toda la memoria antes de terminar.
    delete tablero;
    delete estadoInicial;
    if (solucion) {
        for (int i = 0; solucion[i] != nullptr; i++) delete solucion[i];
        delete[] solucion;
    }

    return 0;
}
