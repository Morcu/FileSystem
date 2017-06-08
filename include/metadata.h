/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */

#define MAX_FICHEROS 64
#define MAX_FILE_NAME 32
#define MAX_FILE_SIZE 2048  
#define BLOCK_SIZE 2048
#define MIN_DISK 51200
#define MAX_DISK 102400 
#define DISK "disk.dat"
#define BLOCK_SUPERBLOCK 0 
#define BLOCK_INODOS 1


typedef struct {
    unsigned int numMagico; /* Número mágico del superbloque: 0x000D5500 */
 //   unsigned int numBloquesMapaInodos; /* Número de bloques del mapa inodos */
 //   unsigned int numBloquesMapaDatos; /* Número de bloques del mapa datos */
    unsigned int numInodos; /* Número de inodos en el dispositivo */
    unsigned int primerInodo; /* Número bloque del 1º inodo del disp. (inodo raíz) */
    unsigned int numBloquesDatos; /* Número de bloques de datos en el disp. */
    unsigned int primerBloqueDatos; /* Número de bloque del 1º bloque de datos */
    long tamDispositivo; /* Tamaño total del disp. (en bytes) */
//    char relleno[PADDING_SB]; /* Campo de relleno (para completar un bloque) */
} TipoSuperbloque;

typedef struct {
//unsigned int tipo; /* T_FICHERO o T_DIRECTORIO */
char nombre[MAX_FILE_NAME]; /* Nombre del fichero/ directorio asociado */
//unsigned int inodosContenidos[200]; /* tipo==dir: lista de los inodos del directorio */
//unsigned int tamanyo; /* Tamaño actual del fichero en bytes */
unsigned int bloqueDirecto; /* Número del bloque directo */
//unsigned int bloqueIndirecto; /* Número del bloque indirecto */
//char relleno[PADDING_INODO]; /* Campo relleno para llenar un bloque */
} TipoInodoDisco;

typedef struct{

    long posicion;
    int abierto; //Si esta abieto 1, si esta cerrado 0

}inodos_X;