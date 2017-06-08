/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	01/03/2017
 */

#include "include/filesystem.h"		// Headers for the core functionality
#include "include/auxiliary.h"		// Headers for auxiliary functions
#include "include/metadata.h"		// Type and structure declaration of the file system
#include "include/crc.h"			// Headers for the CRC functionality
#include <stdlib.h>
#include <string.h>

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */

TipoSuperbloque *sbloques;
TipoInodoDisco *inodos;
char *i_map;
char *b_map ; 
inodos_X *inodos_x;

uint64_t *crcInfo;
uint64_t *crcSuperbInodos;


//Variable para comprobar que no esta montado
int montado = 0;

int mkFS(long deviceSize)
{

	sbloques = malloc(sizeof( TipoSuperbloque));
	sbloques->numMagico = 100330670;
	sbloques->numInodos = (deviceSize/BLOCK_SIZE)-2;
	sbloques->primerInodo = 0;

	if(deviceSize > MAX_FICHEROS*BLOCK_SIZE 
	||deviceSize > MAX_DISK
	|| deviceSize < MIN_DISK)
	{
		return -1;
	}
	sbloques->numBloquesDatos = (deviceSize/BLOCK_SIZE)-2;
	sbloques->primerBloqueDatos = 0;
	sbloques->tamDispositivo = deviceSize;


	i_map = (char *) malloc (sizeof(char)*(sbloques->numBloquesDatos)); 
	memset(i_map, '0', (sbloques->numBloquesDatos));
	
	b_map = (char *) malloc (sizeof(char)*(sbloques->numBloquesDatos)) ; 
	memset(b_map, '0', (sbloques->numBloquesDatos));

	inodos_x =  malloc (sizeof(inodos_X)*(sbloques->numBloquesDatos)) ; 
	bzero(inodos_x,sizeof(inodos_X)*(sbloques->numBloquesDatos));

	inodos =  malloc (sizeof(TipoInodoDisco)*(sbloques->numBloquesDatos)) ; 
	bzero(inodos,sizeof(TipoInodoDisco)*(sbloques->numBloquesDatos));

	crcInfo =  malloc (sizeof(uint64_t)*(sbloques->numBloquesDatos)) ; 
	bzero(crcInfo,sizeof(uint64_t)*(sbloques->numBloquesDatos));

	crcSuperbInodos =  malloc (sizeof(uint64_t)*2) ; 
	bzero(crcInfo,sizeof(uint64_t)*2);

		//Se genera el dispositivo virtual
	char llamada[] = {"./create_disk "};
					//Tamaño maximo del numero que puede aceptar
	char valor[sizeof(unsigned int)*8];
	
	//Se concatena elargumento a la llamada
	sprintf(valor, "%d", sbloques->numBloquesDatos);
	strcat(llamada, valor);

//printf("__Print %s\n",llamada);

	//Se llama a la funcion para generar el dispositivo con parametro numero de bloques de datos
	int resultado =	system(llamada);

//printf("__Print %d\n",resultado);
	//Se devuelve el resultado 
	if(resultado == 0){
		return 0;
	}else{
		return -1;
	}


}

/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */


 //Habria que comprobar si existe el dispositivo virtual, en caso de que exista, se deberia leer 
int mountFS(void)
{
	//Si ya ha sido montado no se vuelve a montar da error
	if(montado == 1){
		return -1;
	}

	//buffer para leer bloques
	char buffer1[BLOCK_SIZE];
	char buffer2[BLOCK_SIZE];


	//Limpiamos la informacion que pudiese haber
	bzero(&sbloques, sizeof(TipoSuperbloque));

	

	int result1 = bread(DISK, BLOCK_SUPERBLOCK, buffer1);
	//Si da error
	if(result1 < 0){
		return -1;
	}

	//Se pasa la informacion del bloque a las estructuras correspondientes
	memcpy(&sbloques, buffer1, sizeof(TipoSuperbloque));

	bzero(&inodos, sizeof(TipoInodoDisco) * sbloques->numBloquesDatos);
	bzero(&b_map, sizeof(sizeof(char *) * sbloques->numBloquesDatos));
	bzero(&i_map, sizeof(sizeof(char *) * sbloques->numBloquesDatos));
	bzero(&crcInfo, sizeof(sizeof(uint64_t) * sbloques->numBloquesDatos));
	bzero(&crcSuperbInodos, sizeof(sizeof(uint64_t) * 2));

	memcpy(&i_map, buffer1 + sizeof(TipoSuperbloque) , sizeof(char)*sbloques->numBloquesDatos);
	memcpy(&b_map,buffer1+ (sizeof(TipoSuperbloque)+sizeof(char) * sbloques->numBloquesDatos), sizeof(char) * sbloques->numBloquesDatos);
	memcpy(&crcInfo,buffer1+ (sizeof(TipoSuperbloque)+sizeof(char) * sbloques->numBloquesDatos) + sizeof(char) * sbloques->numBloquesDatos, sizeof(uint64_t) * sbloques->numBloquesDatos);
	memcpy(&crcSuperbInodos,buffer1+ (sizeof(TipoSuperbloque)+sizeof(char) * sbloques->numBloquesDatos) + sizeof(char) * sbloques->numBloquesDatos + sizeof(uint64_t) * sbloques->numBloquesDatos, sizeof(uint64_t) * 2);


	//Leer el segundo bloque 	
	int result2 = bread(DISK, BLOCK_INODOS, buffer2);
	
	//Si da error
	if(result2 < 0){
		return -1;
	}
	
	//Se pasa la informacion del bloque a la estructura correspondiente
	memcpy(&inodos, buffer2, sizeof(TipoInodoDisco)*sbloques->numBloquesDatos);
	

	int checkSystem = checkFS();
	if(checkSystem < 0){
		return -1;
	}

	montado = 1;
	return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
 
int unmountFS(void)
{
	char buffer[BLOCK_SIZE];
	char buffer2[BLOCK_SIZE];
	bzero(buffer,BLOCK_SIZE);
	bzero(buffer2,BLOCK_SIZE);	// Rellenamos de ceros el buffer

	int i=0;

	for(i = 0; i < sbloques->numBloquesDatos; i++){
		// Si esta abierto el fichero entonces -1, error
		if(inodos_x[i].abierto == 1){
			return -1;
		}
	}

	memcpy(buffer, &sbloques, sizeof(TipoSuperbloque));
	memcpy(buffer+sizeof(TipoSuperbloque), &i_map, sizeof(char) * sbloques->numBloquesDatos);
	memcpy(buffer+ (sizeof(TipoSuperbloque)+sizeof(char) * sbloques->numBloquesDatos), &b_map, sizeof(char) * sbloques->numBloquesDatos);
	
	memcpy(buffer +(sizeof(TipoSuperbloque)+sizeof(char) * sbloques->numBloquesDatos+sizeof(char) * sbloques->numBloquesDatos), &crcInfo,sizeof( uint64_t)* sbloques->numBloquesDatos);

memcpy(buffer2, &inodos, sizeof(TipoInodoDisco) * sbloques->numBloquesDatos);

	crcSuperbInodos[0] = CRC64((void*) buffer, BLOCK_SIZE);
	crcSuperbInodos[1] = CRC64((void*) buffer2, BLOCK_SIZE);

	memcpy(buffer +(sizeof(TipoSuperbloque)+sizeof(char) * sbloques->numBloquesDatos+sizeof(char) * sbloques->numBloquesDatos + sizeof( uint64_t)* sbloques->numBloquesDatos), &crcSuperbInodos,sizeof( uint64_t)*2);

	//Escribir el bloque 0 de disco en sbloques
	int result = bwrite(DISK, BLOCK_SUPERBLOCK, buffer);

		//Si da error
	if(result < 0){
		return -1;
	}

	
	
	int result2 = bwrite(DISK, BLOCK_INODOS, buffer2);

	//Si da error
	if(result2 < 0){
		return -1;
	}


	return 0;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{
	//No podemos tener ficheros con un nombre que supere los 32 bytes
	if(strlen(fileName) > MAX_FILE_NAME ){
		return -2;	

	}
	
	int i =0;
	// buscar i-nodo con nombre <fname>
	for ( i =0; i<sbloques->numInodos; i++)
	{
		//Si el nombre de fichero ya existe
		if (strcmp(inodos[i].nombre, fileName) == 0){
			return -1;
		}	
	}
	 i =0;
	// buscar un i-nodo libre
	for ( i =0; i<sbloques->numInodos; i++)
	{

		//IALLOC
		if (i_map[i] == '0') {
			// inodo ocupado ahora
			i_map[i] = '1';
			// valores por defecto en el i-nodo
			memset(&(inodos[i]), 0,	sizeof(TipoInodoDisco));
			
			//ALLOC
			int j=0;
			for(j=0; j< sbloques->numBloquesDatos; j++)
			{
				if (b_map[j] == '0')
				{
					b_map[j] = '1';
					char b[BLOCK_SIZE];
					memset(b, 0, BLOCK_SIZE);

					strcpy(inodos[i].nombre, fileName);
					inodos[i].bloqueDirecto= j+2;


					int result = bwrite(DISK, inodos[i].bloqueDirecto, b);
					//Si hay error en la lectura
					if(result < 0){
						return -1;
					}
					crcInfo[i] = CRC64( (void *) b,BLOCK_SIZE);

					

					//Si todo ha ido bien
					return 0;
				}
			}

		}
	}
	//En caso de Error
	  return -2;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
int i =0;
	// buscar i-nodo con nombre <fname>
	for (i =0; i<sbloques->numInodos; i++)
	{
		//Si el nombre de fichero ya existe
		if (strcmp(inodos[i].nombre, fileName) == 0){
			
			int check = checkFile(fileName);
			if(check < 0){
				return -1;
			}
			

			//Se pone un 0 en la posicion del inodo correspondiente en el mapa de inodos 
			i_map[i] = '0';

			//Se pone un 0 en la posicion del inodo correspondiente en el mapa de inodos 
			
			b_map[inodos[i].bloqueDirecto] = '0';
			char b[BLOCK_SIZE];
			memset(b, 0, BLOCK_SIZE);
			bwrite(DISK, sbloques->primerBloqueDatos + inodos[i].bloqueDirecto, b);
			
			crcInfo[i] = CRC64((void *)b,BLOCK_SIZE);

			//se limpia la informacion de ese inodo
			memset(&(inodos[i]), 0,	sizeof(TipoInodoDisco));


			return 0;
		}else{
			//Si el nombre del fichero no existe
			return -1;
		}	
	}

	//En caso de Error
	return -2;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
int i =0;
		// buscar i-nodo con nombre <fname>
	for ( i =0; i<sbloques->numInodos; i++)
	{

		//Si el nombre de fichero  existe
		if (strcmp(inodos[i].nombre, fileName) == 0){
			
			int check = checkFile(fileName);
			if(check < 0){
				
				return -1;
			}

			//Si el fichero estaba abierto no se puede volver a abrir y daria error
			if(inodos_x[i].abierto == 1){
				return -2;			
			
			}
			//se abre el fichero y se inicializa el puntero de posicion
			inodos_x[i].posicion = 0;
			inodos_x[i].abierto = 1;
		

		
			return i;
			//Si ha llegado al final y no ha encontrado el fichero
		}else if (i == sbloques->numInodos-1){
			//Si no existe el fichero
			return -1;
		}


	}
	return -2;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	int check = checkFile(inodos[fileDescriptor].nombre);
	if(check < 0){
		return -1;
	}
	if(inodos_x[fileDescriptor].abierto == 1){
		inodos_x[fileDescriptor].abierto = 0;
		return 0;
	}
	
	return -1;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{	

	int check = checkFile(inodos[fileDescriptor].nombre);

	if(check < 0){
		return -1;
	}

	//Si el fichero no esta abierto devuelve -1	
	if(inodos_x[fileDescriptor].abierto == 0){
		return -1;
	}
	//Si el puntero de posicion esta al final del fichero no se lee nada
	if(inodos_x[fileDescriptor].posicion == (BLOCK_SIZE-1)){
		return 0;
	}

	// Si el size al leer excede el tamaño de bloque o el numero de bytes es negativo
	if(numBytes > BLOCK_SIZE || numBytes < 0){
		return -1;
	}

	void * bufferAux = malloc(BLOCK_SIZE);

	int result = bread(DISK, inodos[fileDescriptor].bloqueDirecto, bufferAux);

	//Si hay error en la lectura
	if(result < 0){
		return -1;
	}

	long offset = inodos_x[fileDescriptor].posicion;
	//Si los numeros de bytes a leer son superiores a lo que queda en el bloque solo se lee lo que queda en el bloque
	if(numBytes > BLOCK_SIZE-offset){ 
		numBytes = BLOCK_SIZE-offset;

	}

	//Se copian los datos al buffer teniendo en cuenta el offset
	memcpy(buffer,(bufferAux + offset), numBytes);

	//Actualizamos la posicion del puntero de posicion del desciptor
	inodos_x[fileDescriptor].posicion = offset + numBytes;

	
	return numBytes;

}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	

	int check = checkFile(inodos[fileDescriptor].nombre);
	if(check < 0){
		return -1;
	}


	//Si el fichero no esta abierto devuelve -1	
	if(inodos_x[fileDescriptor].abierto == 0){
	
		return -1;
	}
	//Si el puntero de posicion esta al final del fichero no se escribe nada
	if(inodos_x[fileDescriptor].posicion == (BLOCK_SIZE-1)){
		return 0;
	}

	// Si el size a escribir excede el tamaño de bloque o el tamaño es negativo error
	if(numBytes > BLOCK_SIZE || numBytes < 0){
		
		return -1;
	}


	long offset = inodos_x[fileDescriptor].posicion;

	//Si los numeros de bytes a escribir son superiores a lo que queda en el bloque,
	// solo se escribe lo que queda para llegar hasta el final del bloque
	if(numBytes > BLOCK_SIZE-offset){ 
		numBytes = BLOCK_SIZE-offset;
	}

	void * bufferAux = malloc(BLOCK_SIZE);

	int result = bread(DISK, inodos[fileDescriptor].bloqueDirecto, bufferAux);

	//Si hay error en la lectura
	if(result < 0){
		
		return -1;
	}

	//Copiamos al bloque a partir del offset el buffer que nos dan a escribir con el numero de bytes correspondientes
	memcpy(bufferAux+offset, buffer, numBytes);

	//Escribimos el bloque
	int devWrite = bwrite(DISK, inodos[fileDescriptor].bloqueDirecto, bufferAux);
	if(devWrite < 0){
		return -1;
	}
	crcInfo[fileDescriptor] = CRC64((void *) bufferAux, BLOCK_SIZE);

	//Actualizamos la posicion del puntero de posicion del desciptor
	inodos_x[fileDescriptor].posicion = offset + numBytes;		

	return	numBytes;
}


/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, int whence, long offset)
{

	//Si el fichero no esta abierto devuelve -1	
	if(inodos_x[fileDescriptor].abierto == 0){
		return -1;
	}

	switch(whence){
		case FS_SEEK_CUR: //curr
				if(offset > (BLOCK_SIZE-1)){
					return -1;
				}
				inodos_x[fileDescriptor].posicion = offset;
				return 0;
			break;
		case FS_SEEK_BEGIN: //begin
				inodos_x[fileDescriptor].posicion = 0;
				return 0;
			break;
		case FS_SEEK_END: //end
				inodos_x[fileDescriptor].posicion = (BLOCK_SIZE-1);
				return 0;
			break;

	}
	

	return -1;
}

/*
 * @brief 	Verifies the integrity of the file system metadata.
 * @return 	0 if the file system is correct, -1 if the file system is corrupted, -2 in case of error.
 */
int checkFS(void)
{

	/*Hacer el crc de superb b_map i_map crcInfo de un buffer y en otro la de los inodos y comparar con crcSuperInodo en 0 y 1 respec*/
	char buffer[BLOCK_SIZE];
	char buffer2[BLOCK_SIZE];
	bzero(buffer,BLOCK_SIZE);
	bzero(buffer2,BLOCK_SIZE);	// Rellenamos de ceros el buffer

	

	memcpy(buffer, &sbloques, sizeof(TipoSuperbloque));
	memcpy(buffer+sizeof(TipoSuperbloque), &i_map, sizeof(char) * sbloques->numBloquesDatos);
	memcpy(buffer+ (sizeof(TipoSuperbloque)+sizeof(char) * sbloques->numBloquesDatos), &b_map, sizeof(char) * sbloques->numBloquesDatos);
	
	memcpy(buffer +(sizeof(TipoSuperbloque)+sizeof(char) * sbloques->numBloquesDatos+sizeof(char) * sbloques->numBloquesDatos), &crcInfo,sizeof( uint64_t)* sbloques->numBloquesDatos);

	memcpy(buffer2, &inodos, sizeof(TipoInodoDisco) * sbloques->numBloquesDatos);

	

	if (crcSuperbInodos[0] == CRC64((void*) buffer, BLOCK_SIZE) &&
		crcSuperbInodos[1] == CRC64((void*) buffer2, BLOCK_SIZE)){
			return 0;
	}else{
			return-1;
	}


	return -2;
}

/*
 * @brief 	Verifies the integrity of a file.
 * @return 	0 if the file is correct, -1 if the file is corrupted, -2 in case of error.
 */
int checkFile(char *fileName)
{

	/*leer el bloque del fichero 
	aplicar la funcion crc
	comprobar si lo que devuelve la funcion crc es igual que lo que hay guardado*/
int i =0;
	// buscar i-nodo con nombre <fname>
	for ( i =0; i<sbloques->numInodos; i++)
	{

		//Si el nombre de fichero  existe
		if (strcmp(inodos[i].nombre, fileName) == 0){
			
			void * bufferAux = malloc(BLOCK_SIZE);

			int result = bread(DISK, inodos[i].bloqueDirecto, bufferAux);

			//Si hay error en la lectura
			if(result < 0){
				return -1;
			}

			uint64_t comprobar = CRC64(bufferAux, BLOCK_SIZE);
			
			if(comprobar == crcInfo[i]){
				
				return 0;
			}else{
			
				return -1;
			}

		}
	}
	
	return -2;
}
