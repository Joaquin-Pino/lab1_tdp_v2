// solver.cpp
#include "solver.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

Solver::Solver(Tablero* t)
    : tablero(t), openSet(nullptr), closedSet(nullptr), vecinosTemp(nullptr),
      nodosGenerados(0), nodosVisitados(0), aplicarPodaRedundante(false) {
    int maxDim  = (t->getW() > t->getH()) ? t->getW() : t->getH();
    // En el peor caso, cada pieza puede salir (1 opción) o moverse en 4 direcciones
    // hasta el borde del tablero (maxDim pasos). Multiplicamos por el número de piezas.
    maxVecinos = t->getNumPiezas() * (4 * maxDim + 1);
    openSet = new MinHeap(1024);
    closedSet = new TablaHash(100003); // primo grande para distribuir bien los hashes iniciales
    vecinosTemp = new Estado*[maxVecinos];

    // Poda de movimientos redundantes: deshacer el movimiento previo de la misma pieza
    // siempre lleva al estado anterior, SALVO cuando hay compuertas (los movimientos de portal
    // saltan más de 1 celda, por lo que mover en dirección opuesta no regresa al mismo estado)
    // o cuando hay elementos dinámicos con paso >= 2.
    aplicarPodaRedundante = true;
    if (t->getNumCompuertas() > 0) {
        aplicarPodaRedundante = false;
    }
    if (aplicarPodaRedundante) {
        Salida* salidas = t->getSalidas();
        for (int i = 0; i < t->getNumSalidas(); i++) {
            if (salidas[i].getLi() != salidas[i].getLf() && salidas[i].getPaso() >= 2) {
                aplicarPodaRedundante = false;
                break;
            }
        }
    }
}

Solver::~Solver() {
    // Vaciar openSet: los estados que quedaron sin expandir deben liberarse aquí
    // porque el MinHeap no es dueño de ellos.
    while (!openSet->estaVacio()) {
        Estado* e = openSet->pop();
        delete e;
    }
    // El closedSet SÍ es dueño de sus estados; liberarEstados() los borra todos.
    closedSet->liberarEstados();
    delete openSet;
    delete closedSet;
    delete[] vecinosTemp;
}

void Solver::prepararVecino(Estado* vecino, Estado* actual, char mov[10]) const {
    // Actualizar el color de cada compuerta según el stepUsed del vecino
    // (puede haber cambiado si el nuevo step cruzó un múltiplo del `paso`).
    for (int i = 0; i < tablero->getNumCompuertas(); i++)
        vecino->actualizarCompuerta(i, tablero->calcularColorCompuerta(i, *vecino));

    // Actualizar el largo de cada salida por la misma razón.
    for (int i = 0; i < tablero->getNumSalidas(); i++)
        vecino->actualizarSalida(i, tablero->calcularLargoSalida(i, *vecino));

    // Calcular h y f para que el heap pueda ordenar este vecino correctamente.
    int hv = calcularHeuristica(*vecino);
    vecino->setH(hv);
    vecino->setF(vecino->getStepUsed() + hv);

    // Registrar el estado padre para poder reconstruir el camino al llegar a la meta.
    vecino->setParent(actual);
    vecino->setMovimiento(mov);
}

int Solver::generarVecinos(Estado* actual) {
    int   count    = 0;
    int   numPiezas = tablero->getNumPiezas();
    int   w         = tablero->getW();
    Pieza* piezas   = tablero->getPiezas();

    // Tablas paralelas de las 4 direcciones para evitar repetir el switch en cada iteración.
    direccion dirs[4] = {ARRIBA, ABAJO, IZQUIERDA, DERECHA};
    int dx[4] = { 0,  0, -1,  1};
    int  dy[4] = {-1,  1,  0,  0};
    char dirChar[4] = {'U', 'D', 'L', 'R'};

    for (int id = 0; id < numPiezas; id++) {
        if (actual->piezaYaSalio(id)) continue; // pieza ya fuera del tablero, ignorar
        Pieza& pieza = piezas[id];

        if (tablero->piezaPuedeSalir(id, *actual)) {
            // Si la pieza puede salir ahora, generamos solo ese vecino y pasamos a la siguiente.
            // No generamos también movimientos: sacar es siempre preferible a seguir moviendo.
            Estado* vecino = actual->clonarYSacar(id, pieza, w);

            char mov[10];
            snprintf(mov, 10, "S%d", pieza.getId()); // formato de salida: "S<idExterno>"
            prepararVecino(vecino, actual, mov);

            vecinosTemp[count++] = vecino;
            if (count >= maxVecinos) return count; // protección contra overflow del buffer
            break;
            continue;
        }

        // Intentar mover la pieza 1 celda en cada dirección (o portal si hay compuerta).
        for (int d = 0; d < 4; d++) {
            if (esMovimientoRedundante(id, dirChar[d], actual)) continue;

            int moveX = dx[d], moveY = dy[d];
            if (!tablero->piezaPuedeMoverse(id, dirs[d], *actual)) {
                // Intentar portal a través de compuerta
                int dxSalto, dySalto;
                if (!tablero->piezaPuedeCruzarCompuerta(id, dirs[d], *actual, dxSalto, dySalto)) continue;
                moveX = dxSalto;
                moveY = dySalto;
            }

            Estado* vecino = actual->clonarYMover(id, moveX, moveY, pieza, w);

            char mov[10];
            snprintf(mov, 10, "%c%d,1", dirChar[d], pieza.getId());
            prepararVecino(vecino, actual, mov);

            vecinosTemp[count++] = vecino;
            if (count >= maxVecinos) return count;
        }
    }
    return count;
}

Estado** Solver::reconstruirCamino(Estado* final) {
    // Primera pasada: contar la cantidad de estados en la cadena parent.
    int numPasos = 0;
    Estado* actual = final;
    while (actual != nullptr) {
        numPasos++;
        actual = actual->getParent();
    }

    // Asignar el arreglo con un slot extra para el centinela nullptr al final.
    Estado** camino = new Estado*[numPasos + 1];

    // Segunda pasada: copiar los estados en orden cronológico (del inicial al final).
    // Seguimos la cadena de nuevo pero llenamos el arreglo de atrás hacia adelante.
    actual = final;
    for (int i = numPasos - 1; i >= 0; i--) {
        camino[i] = new Estado(*actual);    // copiar para que el camino sea independiente
        camino[i]->setParent(nullptr);      // romper la cadena en la copia
        actual = actual->getParent();
    }

    camino[numPasos] = nullptr; // centinela que indica el fin del arreglo
    return camino;
}

Estado** Solver::resolver(Estado* estadoInicial) {
    nodosGenerados = 1; // el estado inicial cuenta como el primer nodo generado
    nodosVisitados = 0;

    // Poda inicial: si la cota admisible ya supera el stepLimit, no hay solución posible.
    bool hayCompuertas = tablero->getNumCompuertas() > 0;
    if (cotaInferiorAdmisible(*estadoInicial) > tablero->getStepLimit()) {
        std::cout << "Nodos generados: " << nodosGenerados
                  << " | Nodos visitados: " << nodosVisitados << std::endl;
        delete estadoInicial;
        return nullptr;
    }

    // Inicializar el estado inicial con su heurística y encolarlo.
    int h = calcularHeuristica(*estadoInicial);
    estadoInicial->setH(h);
    estadoInicial->setF(estadoInicial->getStepUsed() + h);
    estadoInicial->setParent(nullptr);
    openSet->push(estadoInicial);

    while (!openSet->estaVacio()) {
        Estado* actual = openSet->pop(); // extraer el estado con menor f

        // Si ya está en closedSet, fue procesado con un camino más corto anteriormente.
        // Descartar este duplicado.
        if (closedSet->existe(actual)) {
            delete actual;
            continue;
        }

        nodosVisitados++;

        // Verificar si llegamos a la meta (todas las piezas salidas).
        if (actual->jugoTerminado(tablero->getNumPiezas())) {
            Estado** camino = reconstruirCamino(actual);
            delete actual; // el estado meta ya fue copiado en reconstruirCamino
            std::cout << "Nodos generados: " << nodosGenerados
                      << " | Nodos visitados: " << nodosVisitados << std::endl;
            return camino;
        }

        // Poda por presupuesto: si ya usamos todos los steps disponibles,
        // este estado no puede generar vecinos válidos.
        if (actual->getStepUsed() >= tablero->getStepLimit()) {
            delete actual;
            continue;
        }

        // Insertar en closedSet antes de expandir para que los vecinos que generen
        // el mismo estado no lo inserten en openSet si ya quedó marcado como explorado.
        closedSet->insertar(actual);

        // Margen para la poda por f con la heurística informada: tolera la sobreestimación
        // del término bloqueos/2. Esta poda es mucho más agresiva que la admisible cuando
        // hay piezas que se bloquean entre sí (caso típico de mapas densos como xd.txt).
        int margen = tablero->getNumPiezas() / 2 + 1;

        int numVecinos = generarVecinos(actual);
        nodosGenerados += numVecinos;
        for (int i = 0; i < numVecinos; i++) {
            // Poda por g: el vecino ya excedió el presupuesto.
            if (vecinosTemp[i]->getStepUsed() > tablero->getStepLimit()) {
                delete vecinosTemp[i];
                continue;
            }
            // Poda por f con margen (heurística informada): poda agresiva pero no estrictamente
            // segura — puede descartar caminos válidos si la informada sobreestima más que el margen.
            if (vecinosTemp[i]->getF() > tablero->getStepLimit() + margen) {
                delete vecinosTemp[i];
                continue;
            }
            // Poda por f admisible (solo si no hay compuertas: con portales la cota puede
            // sobreestimar y descartar estados alcanzables con un camino más corto).
            if (!hayCompuertas && vecinosTemp[i]->getStepUsed()
                + cotaInferiorAdmisible(*vecinosTemp[i]) > tablero->getStepLimit()) {
                delete vecinosTemp[i];
                continue;
            }
            // Solo encolar si no está ya en closedSet.
            if (!closedSet->existe(vecinosTemp[i]))
                openSet->push(vecinosTemp[i]);
            else
                delete vecinosTemp[i];
        }

        // Liberar la matriz de ocupación del estado ya insertado en closedSet.
        // Esto ahorra cientos de bytes por estado en el closed set, que puede tener millones.
        // El estado sigue siendo útil para la reconstrucción del camino vía parent.
        actual->eliminarOcupacion();
    }

    std::cout << "Nodos generados: " << nodosGenerados
              << " | Nodos visitados: " << nodosVisitados << std::endl;
    return nullptr; // openSet vacío sin encontrar la meta: sin solución
}

int Solver::calcularHeuristica(const Estado& estado) const {
    int total = 0;
    int W = tablero->getW();
    int H = tablero->getH();

    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (estado.piezaYaSalio(i)) continue;

        coordenada pos = estado.getPosPiezas()[i];
        Pieza& pieza = tablero->getPiezas()[i];
        int pw = pieza.getAncho();
        int ph = pieza.getAlto();

        int mejorCosto = -1;

        for (int j = 0; j < tablero->getNumSalidas(); j++) {
            Salida& salida = tablero->getSalidas()[j];
            
            if (salida.getColor() != pieza.getColor()) continue;
            if (!tablero->piezaPodriaSalir(pieza, salida)) continue;

            coordenada ps = salida.getPos();
            
            // 1. Distancia Base Mejorada (Incorporando el Span LF de la salida)
            // Usamos la lógica de la cota admisible para obtener una base geométrica 
            // perfecta desde el bounding box de la pieza hacia el span de la salida.
            int L = (salida.getLf() > salida.getLi()) ? salida.getLf() : salida.getLi();
            int perpCost = 0, parCost = 0;

            if (salida.getEsHorizontal()) {
                int adjRow = (ps.y == 0) ? 1 : ((ps.y == H - 1) ? H - 2 : ps.y);
                
                if (pos.y > adjRow) perpCost = pos.y - adjRow;
                else if (pos.y + ph - 1 < adjRow) perpCost = adjRow - (pos.y + ph - 1);

                if (pos.x < ps.x) parCost = ps.x - pos.x;
                else if (pos.x + pw - 1 > ps.x + L - 1) parCost = (pos.x + pw - 1) - (ps.x + L - 1);
            } else {
                int adjCol = (ps.x == 0) ? 1 : ((ps.x == W - 1) ? W - 2 : ps.x);
                
                if (pos.x > adjCol) perpCost = pos.x - adjCol;
                else if (pos.x + pw - 1 < adjCol) perpCost = adjCol - (pos.x + pw - 1);

                if (pos.y < ps.y) parCost = ps.y - pos.y;
                else if (pos.y + ph - 1 > ps.y + L - 1) parCost = (pos.y + ph - 1) - (ps.y + L - 1);
            }

            int distBase = perpCost + parCost;

            // 2. Conflicto Lineal Adaptado (Penalización Informada)
            int bloqueos = contarBloqueos(i, pos, ps, estado);
            int penalizacion = bloqueos / 2;

            // Detección de "Conflicto en Meta" (Linear Conflict): 
            // Si la pieza ya está en la fila/columna adyacente a la salida (distBase == 0) 
            // pero el raycast detecta obstáculos (bloqueos > 0), estamos ante un deadlock local.
            // Sumar +2 es matemáticamente seguro porque despejar el bloqueo cuesta >= 1
            // y avanzar la pieza actual a la meta cuesta >= 1.
            if (distBase == 0 && bloqueos > 0) {
                penalizacion += 2; 
            }

            int costo = distBase + penalizacion;

            if (mejorCosto == -1 || costo < mejorCosto) {
                mejorCosto = costo;
            }
        }

        if (mejorCosto == -1) mejorCosto = 0;
        total += mejorCosto;
    }
    return total;
}

int Solver::cotaInferiorAdmisible(const Estado& estado) const {
    // Cota inferior estrictamente admisible. Para cada pieza activa, calcula la mínima
    // cantidad de celdas necesarias para llevarla a una posición desde la cual pueda salir,
    // considerando que:
    //   - La salida absorbe a la pieza (último paso es gratis), por eso se cuenta hasta la
    //     fila/columna ADYACENTE a la salida, no hasta la celda de la salida.
    //   - El span de la salida (ancho LF) permite que la pieza se alinee en cualquier punto
    //     dentro de él, no solo en la celda de inicio.
    int total = 0;
    int W = tablero->getW();
    int H = tablero->getH();

    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (estado.piezaYaSalio(i)) continue;

        coordenada pos  = estado.getPosPiezas()[i];
        Pieza& pieza    = tablero->getPiezas()[i];
        int pw          = pieza.getAncho();
        int ph          = pieza.getAlto();
        int mejorCosto  = -1;

        for (int j = 0; j < tablero->getNumSalidas(); j++) {
            Salida& salida = tablero->getSalidas()[j];

            if (salida.getColor() != pieza.getColor()) continue;
            if (!tablero->piezaPodriaSalir(pieza, salida)) continue;

            coordenada ps = salida.getPos();

            // Usamos LF (largo máximo): da el span más optimista, lo que mantiene la cota admisible.
            int L = (salida.getLf() > salida.getLi()) ? salida.getLf() : salida.getLi();

            int perpCost = 0;
            int parCost  = 0;

            if (salida.getEsHorizontal()) {
                // Salida horizontal: span [ps.x..ps.x+L-1] en fila ps.y (en algún borde).
                // Fila adyacente desde la cual la pieza puede salir.
                int adjRow;
                if (ps.y == 0) {
                    adjRow = 1;
                } else if (ps.y == H - 1) {
                    adjRow = H - 2;
                } else {
                    adjRow = ps.y; // caso degenerado, no debería pasar
                }

                // Perpendicular: distancia del borde superior/inferior de la pieza a adjRow.
                if (pos.y > adjRow) {
                    perpCost = pos.y - adjRow;
                } else if (pos.y + ph - 1 < adjRow) {
                    perpCost = adjRow - (pos.y + ph - 1);
                }

                // Paralelo: alinear el bbox horizontalmente dentro del span.
                if (pos.x < ps.x) {
                    parCost = ps.x - pos.x;
                } else if (pos.x + pw - 1 > ps.x + L - 1) {
                    parCost = (pos.x + pw - 1) - (ps.x + L - 1);
                }

            } else {
                // Salida vertical: span [ps.y..ps.y+L-1] en columna ps.x (en algún borde).
                int adjCol;
                if (ps.x == 0) {
                    adjCol = 1;
                } else if (ps.x == W - 1) {
                    adjCol = W - 2;
                } else {
                    adjCol = ps.x;
                }

                // Perpendicular: distancia del borde izquierdo/derecho de la pieza a adjCol.
                if (pos.x > adjCol) {
                    perpCost = pos.x - adjCol;
                } else if (pos.x + pw - 1 < adjCol) {
                    perpCost = adjCol - (pos.x + pw - 1);
                }

                // Paralelo: alinear el bbox verticalmente dentro del span.
                if (pos.y < ps.y) {
                    parCost = ps.y - pos.y;
                } else if (pos.y + ph - 1 > ps.y + L - 1) {
                    parCost = (pos.y + ph - 1) - (ps.y + L - 1);
                }
            }

            int dist = perpCost + parCost;
            if (mejorCosto == -1 || dist < mejorCosto) {
                mejorCosto = dist;
            }
        }

        if (mejorCosto == -1) mejorCosto = 0;
        total += mejorCosto;
    }
    return total;
}

bool Solver::esMovimientoRedundante(int idPieza, char dir, const Estado* actual) const {
    // Si el mapa tiene compuertas o salidas oscilantes, no podar: a veces es necesario
    // "perder" un step para que cambie un color de compuerta o un largo de salida.
    if (!aplicarPodaRedundante) return false;

    const char* mov = actual->getMovimiento();
    if (mov[0] == '\0' || mov[0] == 'S') return false; // sin movimiento previo, o el previo fue una salida

    // Formato del movimiento previo: "<dir><idExterno>,<dist>" (ej: "R3,1", "U5,1").
    char prevDir = mov[0];
    int  prevId  = atoi(mov + 1);

    // Si la pieza previa no es la actual, no hay redundancia: mover otra pieza no deshace nada.
    int extId = tablero->getPiezas()[idPieza].getId();
    if (prevId != extId) return false;

    // dir es opuesta a prevDir => moverse hacia el lado contrario regresa al estado anterior.
    if ((prevDir == 'U' && dir == 'D') || (prevDir == 'D' && dir == 'U') ||
        (prevDir == 'L' && dir == 'R') || (prevDir == 'R' && dir == 'L'))
        return true;

    return false;
}

int Solver::contarBloqueos(int idPieza, coordenada pos,
                             const Estado& estado) const {
    // Versión sin salida explícita: encontrar primero la salida más cercana por Manhattan
    // y delegar en la versión con salida.
    Pieza& pieza = tablero->getPiezas()[idPieza];
    coordenada mejorSalida = {0, 0};
    int mejorDist = -1;
    for (int j = 0; j < tablero->getNumSalidas(); j++) {
        Salida& salida = tablero->getSalidas()[j];
        
        if (salida.getColor() != pieza.getColor()) continue;
        if (!tablero->piezaPodriaSalir(pieza, salida)) continue;
        
        int d = abs(pos.x - salida.getPos().x) + abs(pos.y - salida.getPos().y);
        if (mejorDist == -1 || d < mejorDist) {
            mejorDist  = d;
            mejorSalida = salida.getPos();
        }
    }
    if (mejorDist <= 0) return 0;
    return contarBloqueos(idPieza, pos, mejorSalida, estado);
}

int Solver::contarBloqueos(int idPieza, coordenada pos, coordenada posSalida,
                             const Estado& estado) const {
    int w = tablero->getW();
    short* ocupacion = estado.getOcupacion();
    celda* matriz = tablero->getMatriz();

    // Camino 1: ir primero en horizontal hasta posSalida.x, luego en vertical hasta posSalida.y.
    int bloqueosHorizPrimero = 0;
    {
        // Tramo horizontal: avanzar desde pos.x hacia posSalida.x a lo largo de la fila pos.y.
        int xIni   = pos.x < posSalida.x ? pos.x + 1 : pos.x - 1;
        int xFin   = posSalida.x;
        int paso   = pos.x < posSalida.x ? 1 : -1;
        short ultimoId = idPieza; // para no contar la misma pieza bloqueante más de una vez
        for (int x = xIni; x != xFin + paso; x += paso) {
            if (x < 0 || x >= w) break;
            short ocup = ocupacion[pos.y * w + x];
            // Contar solo piezas distintas de la pieza actual y del último bloqueante detectado.
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosHorizPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza; // resetear al encontrar celda libre
            }
            celda& c = matriz[pos.y * w + x];
            if (c.tipo == PARED) { bloqueosHorizPrimero += 2; break; } // pared infranqueable
        }
        // Tramo vertical: avanzar desde pos.y hacia posSalida.y a lo largo de la columna posSalida.x.
        int yIni  = pos.y < posSalida.y ? pos.y + 1 : pos.y - 1;
        int yFin  = posSalida.y;
        int pasoY = pos.y < posSalida.y ? 1 : -1;
        ultimoId  = idPieza;
        for (int y = yIni; y != yFin + pasoY; y += pasoY) {
            if (y < 0 || y >= tablero->getH()) break;
            short ocup = ocupacion[y * w + posSalida.x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosHorizPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[y * w + posSalida.x];
            if (c.tipo == PARED) { bloqueosHorizPrimero += 2; break; }
        }
    }

    // Camino 2: ir primero en vertical hasta posSalida.y, luego en horizontal hasta posSalida.x.
    int bloqueosVertPrimero = 0;
    {
        // Tramo vertical: avanzar desde pos.y hacia posSalida.y a lo largo de la columna pos.x.
        int yIni  = pos.y < posSalida.y ? pos.y + 1 : pos.y - 1;
        int yFin  = posSalida.y;
        int pasoY = pos.y < posSalida.y ? 1 : -1;
        short ultimoId = idPieza;
        for (int y = yIni; y != yFin + pasoY; y += pasoY) {
            if (y < 0 || y >= tablero->getH()) break;
            short ocup = ocupacion[y * w + pos.x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosVertPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[y * w + pos.x];
            if (c.tipo == PARED) { bloqueosVertPrimero += 2; break; }
        }
        // Tramo horizontal: avanzar desde pos.x hacia posSalida.x a lo largo de la fila posSalida.y.
        int xIni = pos.x < posSalida.x ? pos.x + 1 : pos.x - 1;
        int xFin = posSalida.x;
        int paso = pos.x < posSalida.x ? 1 : -1;
        ultimoId = idPieza;
        for (int x = xIni; x != xFin + paso; x += paso) {
            if (x < 0 || x >= w) break;
            short ocup = ocupacion[posSalida.y * w + x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosVertPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[posSalida.y * w + x];
            if (c.tipo == PARED) { bloqueosVertPrimero += 2; break; }
        }
    }

    // Devolver el camino con menos bloqueos: tomar el recorrido más despejado
    // para no sobreestimar más de lo necesario.
    return bloqueosHorizPrimero < bloqueosVertPrimero ? bloqueosHorizPrimero : bloqueosVertPrimero;
}
