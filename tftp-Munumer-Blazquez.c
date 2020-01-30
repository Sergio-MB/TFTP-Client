// Practica tema 7, Muñumer Blázquez Sergio
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>

#define MAX 		516
#define ACK_SIZE 	4
#define DATA_SIZE 	512
#define SIZE 		100
#define READ 		"-r"
#define WRITE 		"-w"
#define VERBOSE 	"-v"
#define MODE 		"octet"
#define RRQ 		1
#define WRQ			2
#define OK 			3
#define ACK 		4
#define ERROR 		5
#define SC 			"_tmp_"



void checkError(char *response);
int fcpy(FILE *original, FILE *copia);


//Funcion para copiar el contenido de un fichero en otro
//
//Return: Cantidad de bytes transmitidos
int fcpy(FILE *original, FILE *copia){
	
	int  count = 0;
    char ch;

    while ((ch = fgetc(original)) != EOF)
    {
        fputc(ch, copia);
        count++;
    }

    return count;	
}

//Funcion para chekear el error capturado
//Segun el codigo de error, imprimimos el mensaje
void checkError(char *response){
	
	switch(response[2]){	
		case 1:
			printf("Error: %s\n", &response[3]);	
			break;
		case 2:
			printf("Error: %s\n", &response[3]);	
			break;
		case 3:
			printf("Error: %s\n", &response[3]);
			break;
		case 4:
			printf("Error: %s\n", &response[3]);
			break;
		case 5:
			printf("Error: %s\n", &response[3]);
			break;
		case 6:
			printf("Error: %s\n", &response[3]);
			break;
		case 7:
			printf("Error: %s\n", &response[3]);
			break;
			
		default:
			printf("Error fatal %d\n", response[2]);
			
	}	
}

//Funcion principal
int main(int argc, char *argv[]) {

	char ack[ACK_SIZE];		//Buffer para crear ACK's
	char request[MAX];		//Buffer para hacer peticiones
	char response[MAX];		//Buffer para almacenar respuestas del servidor
	int sockfd;
	int rcv_count;
	socklen_t len ;
	FILE  *file;
	FILE  *file2;
	int informer;			     //Flag verbose

	struct sockaddr_in client;   //Estructuras Cliente y Servidor
	struct sockaddr_in servaddr;
	char archivo[SIZE];	     //Archivo a tratar
	//char aux[SIZE];	
	servaddr.sin_family=AF_INET;
//Comprobamos si el numero de argumentos es válido
	if (argc < 4 || argc > 5) {
		printf("\nError de sintaxis\n");
		printf("--> ./tftp-Munumer-Blazquez ip-servidor {-r|-w} archivo [-v]\n");
		exit(EXIT_FAILURE);
	}
//Asignamos ARGV[1] como direccion del servidor
	inet_aton(argv[1], &servaddr.sin_addr);
//Buscamos puerto por defecto para el servicio TFTP
	struct servent *server;
	server=getservbyname("tftp","udp");
	if (server == NULL){
			printf("Error al obtener puerto\n");
			perror("\nserv");
			exit(EXIT_FAILURE);
		}
//Asignamos dicho puerto
	servaddr.sin_port = server->s_port;
	len = sizeof(servaddr);
	
	strcpy(archivo, argv[3]);
//Comprobamos si se solicito modo verbose
	if( (argc == 5) && (strcmp( VERBOSE , argv[4] ) == 0) ) informer = 1;
	
	int flag, bloque;
	flag = 1;		//flag de control del bucle
	bloque = 0;		//Contador de bloques
//Modo lectura
	if ( strcmp( READ, argv[2])==0  ){
//Preparamos el formato necesario para realizar la peticion de lectura		
		request[0] = 0;
		request[1] = RRQ;
		strcpy(&request[2], archivo);
		strcpy(&request[3+strlen(archivo)], MODE);
//Definimos el tipo de descriptor socket		
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("\nsocket()");
			exit(EXIT_FAILURE);
		}
//Completamos la estructura de conexion para el cliente
		client.sin_family=AF_INET;
		client.sin_port=0;
		client.sin_addr.s_addr= INADDR_ANY;
//Enlazamos socket y estructura 
		if (bind(sockfd, (struct sockaddr*) &client, sizeof(client)) < 0){
			printf("Fallo al enlazar el socket\n");
			perror("\nbind()");
			exit (EXIT_FAILURE);
		}
//Realizamos el envio de la peticion de lectura al servidor
		if(sendto(sockfd, request, sizeof(request),0, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
			printf("Fallo en el envio\n");
			perror("\nsendto()");
			exit(EXIT_FAILURE);
		}
//Imprimimos paso a paso si es necesario
		if (informer) printf("Enviada solicitud de lectura de %s a servidor tftp en %s\n", archivo, inet_ntoa(servaddr.sin_addr));
//Apertura del archivo en local para escribir lo que recibamos
		file = fopen(archivo, "w");
				if(file == NULL) {
						perror("\nfopen()");
						exit(EXIT_FAILURE);
				}
//Mientras flag sea 1, bucle infinito para recibir bloques
		while(flag){
//Recibimos del servidor, almacenando en una variable la cantidad de bytes recibidos
			if((rcv_count=recvfrom(sockfd, response, MAX, 0,(struct sockaddr*)&servaddr,&len)) < 0){
				perror("\nrecvfrom()");
				exit(EXIT_FAILURE);
			}	
			
//Comprobacion de bloqueEsperado contra bloqueRecibido, si son diferentes:
//Realizamos un ack del ultimo bloque recibido en el orden correcto
			if((bloque + 1) != (((unsigned char)response[2] << 8) + (unsigned char)response[3])){				
				ack[0] = 0;
				ack[1] = ACK;
				ack[2] = bloque / 256; 
				ack[3] = bloque-( ack[2] * 256 );
				
				if(sendto(sockfd, ack, ACK_SIZE, 0, (struct sockaddr*) &servaddr, sizeof(servaddr))<0){
					printf("Fallo en ACK\n");
					perror("\nsendto()");
					exit(EXIT_FAILURE);
				}
				bloque++;
				continue;	//Siguiente iteracion, porfavor
			}
			bloque++;		//Incrementamos = bloqueRecibido correcto
//Comprobamos el codigo recibido,
//Si es Correcto:
//	Escribimos en fichero
//Si no es Correcto:
//	Comprobamos el error
			if(response[1] == OK){
				if (informer) printf("Recibido bloque [Num:%d] del servidor TFTP\n", bloque);

				fwrite(&response[4], 1, rcv_count-4, file);
			}else if(response[1] == ERROR){
//Cerramos fichero y le eliminamos				
				checkError(&response[1]);
				if( fclose(file) < 0 ) perror("fclose()\n");
				if( remove(archivo) < 0) perror("remove()\n");
				
				break;
			}
//Preparamos ACK, para pedir siguiente bloque
			ack[0] = 0;
			ack[1] = 4;
			ack[2] = response[2];
			ack[3] = response[3];
			
			if (informer) printf("Enviando ACK bloque [Num:%d]...\n", bloque);
//Realizamos el envio
			if(sendto(sockfd, ack, ACK_SIZE, 0, (struct sockaddr*) &servaddr, sizeof(servaddr))<0){
				printf("Fallo en ACK\n");
				perror("\nsendto()");
				exit(EXIT_FAILURE);
			}
	
//Condicion para salir del bucle infinito
//Si la cantidad de bytes recibidos es inferior al tamano de un bloque entero
//Suponemos que es el ultimo bloque, cambiamos la flag a false para romper el bucle
//Y cerramos el fichero tras acabar de escribir en el			
			if(rcv_count < MAX){
				if (informer) printf("El bloque [Num:%d] era el último: cerramos el fichero\n", bloque);
				flag = 0;
				if( fclose(file) < 0 ){
						perror("fclose()");
				}
				
			}
		
	}
//Modo escritura
	}else if(strcmp( WRITE, argv[2])==0){
//Preparamos peticion de escritura
		request[0] = 0;
		request[1] = WRQ;
		strcpy(&request[2], archivo);
		strcpy(&request[3+strlen(archivo)], MODE);
		
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("\nsocket()");
			exit(EXIT_FAILURE);
		}
		
		client.sin_family=AF_INET;
		client.sin_port=0;
		client.sin_addr.s_addr= INADDR_ANY;

	   
		if (bind(sockfd, (struct sockaddr*) &client, sizeof(client)) < 0){
			printf("Fallo al enlazar el socket\n");
			perror("\nbind()");
			exit (EXIT_FAILURE);
		}
//Realizamos el envio de la peticion, comprobando error
		if(sendto(sockfd, request, sizeof(request),0, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
			printf("Fallo en el envio\n");
			perror("\nsendto()");
			exit(EXIT_FAILURE);
		}
		if (informer) printf("Enviada solicitud de escritura de %s a servidor tftp en %s\n", archivo, inet_ntoa(servaddr.sin_addr));

		int send_count;
//Abrimos el archivo solicitado para lectura, y poder extraer el contenido
		
	
//Creamos una copia del fichero para poder extraer bloques aislados en caso 
//de errores en la retransmision del archivo
		file2 = fopen(SC, "w");
		if(file2 == NULL) {
				perror("\nfopen2()");
				exit(EXIT_FAILURE);
		}
		file = fopen(archivo, "r");
		if(file == NULL) {
				perror("\nfopen()");
				exit(EXIT_FAILURE);
		}		
		if(fcpy(file, file2) < 0){
			printf("Fallo en la copia de seguridad\n");
			if( fclose(file) < 0 ) perror("fclose()\n");
			if( fclose(file2) < 0 ) perror("fclose()\n");
			perror("\nfcpy()");
			exit(EXIT_FAILURE);
		}
		if( fclose(file) < 0 ) perror("fclose()\n");
		if( fclose(file2) < 0 ) perror("fclose()\n");
//Abrimos fichero principal en la retransmision		
		file = fopen(archivo, "r");
		if(file == NULL) {
				perror("\nfopen()");
				exit(EXIT_FAILURE);
		}

//Bucle infinito para realizar los envios necesarios
		int enviado = 0; 		//flag para controlar el error de red
		while(flag){
			
//Recibimos ACK del server,
			if((rcv_count=recvfrom(sockfd, response, MAX, 0,(struct sockaddr*)&servaddr,&len)) < 0){
					perror("\nrecvfrom()");
					exit(EXIT_FAILURE);
				}	
			if(response[1] == ACK){
				if(informer) printf("Recibido ACK bloque [Num:%d]\n", (((unsigned char)response[2] << 8) + (unsigned char)response[3]));
				if((bloque) != (((unsigned char)response[2] << 8) + (unsigned char)response[3])){
					int contadorBloques = 0;
					file2 = fopen(SC, "r");
					if(file2 == NULL) {
						perror("\nfopen()");
						exit(EXIT_FAILURE);
					}
					while(contadorBloques != ((((unsigned char)response[2] << 8) + (unsigned char)response[3]) + 1)){
						
						send_count = fread(&request[4], 1, DATA_SIZE , file2);
						contadorBloques++;
					}
					bloque=contadorBloques;
				
					request[0]=0;
					request[1]=3;
					request[2] = bloque / 256; 
					request[3] = bloque-(request[2]*256);
					if(sendto(sockfd, request, (send_count + 4) ,0, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
						printf("Fallo en el envio\n");
						perror("\nsendto()");
						exit(EXIT_FAILURE);
					}//
					if (informer) printf("Enviado bloque [Num:%d]\n",bloque);
					if( fclose(file2) < 0 ) perror("fclose()\n");
					enviado = 1;
				
				}
			
				if(!enviado) bloque++;
	//Preparamos respuesta al ACK del servidor, con el bloque pertinente
				request[0] = 0;
				request[1] = 3;
				request[2] = bloque / 256; 
				request[3] = bloque-(request[2]*256);
				
				send_count = fread(&request[4], 1, DATA_SIZE , file);
				if(!enviado){
					if(sendto(sockfd, request, (send_count + 4) ,0, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
						printf("Fallo en el envio\n");
						perror("\nsendto()");
						exit(EXIT_FAILURE);
					}//
					if (informer) printf("Enviado bloque [Num:%d]\n",bloque);
				}
				enviado = 0;
				
			}else if(response[1] == ERROR){
//Cerramos fichero y le eliminamos				
				checkError(&response[1]);
				if( fclose(file) < 0 ) perror("fclose()\n");
			
				break;
			}
			
//Condicion de salida para el bucle
//Si los bytes enviados son inferiores a la cantidad de un datagrama entero es decir 512
//ponemos la flag falsa, y salimos del bucle
			if(send_count < DATA_SIZE){
				flag = 0;
				if( remove(SC) < 0) perror("remove()\n");
			}
		}
		if((rcv_count=recvfrom(sockfd, response, MAX, 0,(struct sockaddr*)&servaddr,&len)) < 0){
					perror("\nrecvfrom()");
					exit(EXIT_FAILURE);
				}
		if (informer) printf("Recibido ACK bloque [Num:%d]\n", (((unsigned char)response[2] << 8) + (unsigned char)response[3]));
		if (informer) printf("Ultimo bloque enviado [Num:%d]\n", bloque);
	}
	if (informer) printf("Cerrando conexion con el servidor TFTP\n");
	exit(EXIT_SUCCESS);
	close(sockfd);
	
	
	
//FIN
    return 0;
}
