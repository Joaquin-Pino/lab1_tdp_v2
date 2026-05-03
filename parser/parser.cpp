// parser.cpp
#include "parser.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

Parser::Parser(const char* nombreArchivo)
    : archivo(nullptr), w(0), h(0), stepLimit(0),
      numPiezas(0), numSalidas(0), numCompuertas(0),
      matriz(nullptr), piezas(nullptr), salidas(nullptr), compuertas(nullptr) {

    lineaActual[0] = '\0';
    archivo = fopen(nombreArchivo, "r");
    if (!archivo)
        std::cerr << "Error: no se pudo abrir el archivo " << nombreArchivo << std::endl;
}

// El destructor libera la memoria de los arreglos, pero no de los objetos individuales,
Parser::~Parser() {
    if (archivo) fclose(archivo);
    // construirTablero() pone estos punteros en nullptr al transferirlos al Tablero,
    // así que delete[] nullptr es no-op y no hay doble liberación.
    delete[] matriz;
    delete[] piezas;
    delete[] salidas;
    delete[] compuertas;
}

// Lee una línea del archivo y la almacena en lineaActual, eliminando el salto de línea.
// Retorna true si se leyó una línea, false si se llegó al final del archivo
bool Parser::leerLinea() {
    if (!archivo) return false;

    // fgets incluye el salto de línea, así que lo eliminamos para facilitar el procesamiento.
    // Si la línea es demasiado larga, se truncará, pero asumimos que 256 caracteres son suficientes para nuestras líneas de configuración.
    if (fgets(lineaActual, 256, archivo) == nullptr) {
        return false;
    }
    // eliminar salto de línea al final
    int len = strlen(lineaActual);
    if (len > 0 && lineaActual[len-1] == '\n')
        lineaActual[len-1] = '\0';
    return true;
}

bool Parser::esSeccion(const char* nombre) {
    return strncmp(lineaActual, nombre, strlen(nombre)) == 0;
}

// Cada función de parseo de sección asume que la línea actual es el encabezado de esa sección
// y lee líneas hasta encontrar el siguiente encabezado (que "devuelve" con tieneLinea Pendiente).
void Parser::parsearMeta() {
    // Lee líneas clave=valor hasta encontrar un encabezado de sección o el final del archivo.
    while (leerLinea()) {

        if (lineaActual[0] == '[') {
            // Encontramos el encabezado de la siguiente sección.
            // Lo "devolvemos" para que construirTablero() lo despache correctamente.
            tieneLineaPendiente = true;
            return;
        }
        // Procesar  meta, esperando formato clave=valor. Ignorar líneas mal formateadas.
        char clave[64], valor[64];
        if (sscanf(lineaActual, "%s = %s", clave, valor) != 2) continue;
        if (strcmp(clave, "WIDTH") == 0) w = atoi(valor);
        if (strcmp(clave, "HEIGHT") == 0) h = atoi(valor);
        if (strcmp(clave, "STEP_LIMIT") == 0) stepLimit = atoi(valor);
    }
}


void Parser::parsearWall() {
    // Asignar la matriz de celdas con el tamaño correcto. El parser de paredes asume que w y h ya fueron leídos en META.
    matriz = new celda[w * h];

    // inicializar todo como VACIA
    for (int i = 0; i < w * h; i++)
        matriz[i] = {VACIA, -1};

    int fila = 0;
    // Leer líneas de la sección WALL, marcando las celdas con '#' como PARED. Detenerse al encontrar el siguiente encabezado de sección.
    while (fila < h && leerLinea()) {
        if (lineaActual[0] == '[') return;  // siguiente sección

        // Marcar celdas con '#' como PARED. El formato es una línea de texto con w caracteres, donde '#' indica pared.
        for (int j = 0; j < w && lineaActual[j] != '\0'; j++) {
            if (lineaActual[j] == '#')
                matriz[fila * w + j] = {PARED, -1};
        }
        fila++;
    }
}

// 
void Parser::parsearBloques() {
    const int MAX_TEMP = 100;
    Pieza temp[MAX_TEMP];
    numPiezas = 0;

    while (leerLinea()) {
        if (lineaActual[0] == '[') {
            // El encabezado de la siguiente sección fue consumido por leerLinea().
            // Como no tenemos un mecanismo de "unget" de línea completa aquí,
            // retrocedemos el cursor del archivo para que construirTablero() la vea.
            fseek(archivo, -(long)(strlen(lineaActual)+1), SEEK_CUR);
            break;
        }
        // Ignorar líneas vacías o de solo salto de línea.
        if (lineaActual[0] == '\0') continue;

        // id, ancho, alto, posicion x e y
        int id, bw, bh, initX, initY;
        char colorChar; 

        // leemos el formato esperado: ID=n COLOR=c WIDTH=n HEIGHT=n INIT_X=n INIT_Y=n [GEOMETRY=...]
        if (sscanf(lineaActual, "%d COLOR=%c WIDTH=%d HEIGHT=%d INIT_X=%d INIT_Y=%d",
                   &id, &colorChar, &bw, &bh, &initX, &initY) != 6) continue;
        
        // creamos la geometria de la peiza
        bool* geom = new bool[bw * bh];
        char* ptr = strstr(lineaActual, "GEOMETRY=");
        // 
        if (ptr) {
            ptr += strlen("GEOMETRY=");
            for (int k = 0; k < bw * bh; k++) {
                while (*ptr == ' ') ptr++;
                geom[k] = (*ptr == '1');
                ptr++;
            }
        }

        
        coordenada pos = {initX, initY};
        temp[numPiezas] = Pieza(id, (short)bw, (short)bh, (int)colorChar, pos, geom); // agregamos la pieza al arreglo temporal
        numPiezas++;
    }

    // Copiar del arreglo temporal al arreglo final de piezas. Esto evita reallocs o un MAX_TEMP fijo en el arreglo final.
    piezas = new Pieza[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        piezas[i] = temp[i];
}

void Parser::parsearSalidas() {
    // Dos pasadas sobre la sección: primero contar para asignar el arreglo exacto,
    // luego leer de nuevo para construir los objetos. Esto evita reallocs o un MAX_TEMP fijo.
    long posInicio = ftell(archivo);
    numSalidas = 0;

    while (leerLinea()) {
        if (lineaActual[0] == '[') break;
        if (lineaActual[0] != '\0' && lineaActual[0] != '\n')
            numSalidas++;
    }

    // 
    fseek(archivo, posInicio, SEEK_SET);
    salidas = new Salida[numSalidas];
    int idx = 0;

    while (idx < numSalidas && leerLinea()) {
        if (lineaActual[0] == '[') return;

        char colorChar, orient;
        int x, y, li, lf, paso;

        // formato: COLOR=c X=x Y=y ORIENTATION=H,V LI=n LF=n STEP=n
        if (sscanf(lineaActual, "COLOR=%c X=%d Y=%d ORIENTATION=%c LI=%d LF=%d STEP=%d",
                   &colorChar, &x, &y, &orient, &li, &lf, &paso) != 7) continue;

        bool esHorizontal = (orient == 'H');
        coordenada pos = {x, y};
        salidas[idx] = Salida(idx, (int)colorChar, pos, esHorizontal,
                              (short)li, (short)lf, (short)paso);
        
        for (int k = 0; k < li; k++) {
            int fila    = esHorizontal ? y : y + k;  // ← correcto, crece hacia abajo
            int columna = esHorizontal ? x + k : x;
            if (fila >= 0 && fila < h && columna >= 0 && columna < w)
                matriz[fila * w + columna] = {SALIDA, idx};
        }
        idx++;
    }
}

void Parser::parsearCompuertas() {
    const int MAX_TEMP = 100;
    Compuerta temp[MAX_TEMP];
    numCompuertas = 0;

    // Similar a parsearSalidas(), hacemos dos pasadas para contar y luego leer. Esto evita reallocs o un MAX_TEMP fijo en el arreglo final.
    while (leerLinea()) {
        if (lineaActual[0] == '[') {
            tieneLineaPendiente = true;
            break;
        }
        if (lineaActual[0] == '\0') continue;

        char orient;
        int x, y, li, ci, cf, paso;
        short tamano = 1;

        // formato: COLOR=c X=x Y=y ORIENTATION=H,V LI=n CI=n CF=n STEP=n
        if (sscanf(lineaActual, "COLOR=%*c X=%d Y=%d ORIENTATION=%c LI=%d CI=%d CF=%d STEP=%d",
           &x, &y, &orient, &li, &ci, &cf, &paso) != 7) continue;
        
        bool esVertical = (orient == 'V');
        coordenada pos = {x, y};
        temp[numCompuertas] = Compuerta(numCompuertas, pos, tamano, esVertical,
                                        (short)ci, (short)cf, (short)paso);

        if (y >= 0 && y < h && x >= 0 && x < w)
            matriz[y * w + x] = {COMPUERTA, numCompuertas};

        numCompuertas++;
    }

    compuertas = new Compuerta[numCompuertas];
    for (int i = 0; i < numCompuertas; i++)
        compuertas[i] = temp[i];
}

// Construye un Tablero* a partir de los datos parseados. Transfiere ownership de los arreglos al Tablero, dejando los punteros del Parser en nullptr para evitar doble liberación.
Tablero* Parser::construirTablero() {
    if (!archivo) return nullptr;

    tieneLineaPendiente = false;

    while (tieneLineaPendiente || leerLinea()) {
        tieneLineaPendiente = false;

        if      (esSeccion("[META]"))   parsearMeta();
        else if (esSeccion("[WALL]"))   parsearWall();
        else if (esSeccion("[BLOCK]"))  parsearBloques();
        else if (esSeccion("[EXIT]"))   parsearSalidas();
        else if (esSeccion("[GATE]"))   parsearCompuertas();
    }

    Tablero* t = new Tablero(matriz, piezas, salidas, compuertas,
                              numPiezas, numSalidas, numCompuertas,
                              w, h, stepLimit);

    // Transferir ownership: el Tablero ahora es responsable de liberar estos arreglos.
    // Ponemos los punteros en nullptr para que el destructor del Parser no los libere dos veces.
    matriz     = nullptr;
    piezas     = nullptr;
    salidas    = nullptr;
    compuertas = nullptr;

    return t;
}