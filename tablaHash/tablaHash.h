// tablahash.h
#pragma once
#include "../estado/estado.h"

// Tabla hash con encadenamiento separado (listas enlazadas por bucket).
// Actúa como el closed set de A*: almacena los estados ya explorados
// para evitar procesar el mismo estado dos veces.
//
// Regla de ownership: esta tabla ES dueña de los Estado* que almacena.
// Hay que llamar liberarEstados() antes de destruirla si se quiere liberar
// la memoria de los estados; el destructor solo libera los nodos de la lista.
//
// No es copiable porque gestiona memoria heap con semántica de dueño.
class TablaHash {
private:
    struct Nodo {
        Estado* estado;
        Nodo*   siguiente;
        Nodo(Estado* e, Nodo* sig) : estado(e), siguiente(sig) {}
    };

    Nodo** tabla;    // arreglo de punteros a lista encadenada, uno por bucket
    int capacidad;   // cantidad de buckets
    int tamano;      // cantidad de estados almacenados

    // Calcula el índice del bucket para el estado dado.
    unsigned int calcularHash(const Estado* e) const;
    // Igual pero con capacidad explícita (usado durante rehash).
    unsigned int calcularHashConCapacidad(const Estado* e, int cap) const;

    // Compara dos estados por contenido (posiciones, piezasSalidas, compuertas, salidas).
    // No compara stepUsed: dos estados con la misma configuración pero diferente costo
    // se consideran el mismo nodo del grafo; el closed set guarda el primero encontrado.
    bool sonIguales(const Estado* a, const Estado* b) const;

    // Duplica la capacidad de la tabla y redistribuye todos los nodos.
    // Se llama automáticamente cuando el factor de carga supera 0.75.
    void rehash();

public:
    TablaHash(int capacidad);
    TablaHash(const TablaHash& otro) = delete;
    TablaHash& operator=(const TablaHash& otro) = delete;
    ~TablaHash();

    void insertar(Estado* e);
    bool existe(const Estado* e) const;
    int  getTamano() const;

    // Libera todos los Estado* almacenados y vacía la tabla.
    // Debe llamarse antes de destruir la tabla si los estados no fueron liberados externamente.
    void liberarEstados();
};
