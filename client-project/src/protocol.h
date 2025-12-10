/* ------------------------------------------------------------------------------------------- */
									/* File Protocol.h */
/* ------------------------------------------------------------------------------------------- */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

/* ------------------------------------------------------------------------------------------- */
						/* Gestione include per portabilità */
/* ------------------------------------------------------------------------------------------- */

#if defined WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <errno.h>

    #define SOCKET int
    #define INVALID_SOCKET -1
    #define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------------------------------- */
							/* Costanti */
/* ------------------------------------------------------------------------------------------- */

#define SERVER_PORT 56700
#define NO_ERROR 0
#define MAX_CITY_LEN 64

// Codici di stato
#define STATUS_OK 0
#define STATUS_CITY_NOT_FOUND 1
#define STATUS_INVALID_REQUEST 2

// Tipi meteo
#define TYPE_TEMPERATURE 't'
#define TYPE_HUMIDITY    'h'
#define TYPE_WIND        'w'
#define TYPE_PRESSURE    'p'

/* ------------------------------------------------------------------------------------------- */
								/* Strutture Dati */
/* IMPORTANTE: NON modificare queste strutture come da traccia       */
/* ------------------------------------------------------------------------------------------- */

typedef struct {
    char type;
    char city[MAX_CITY_LEN];
} weather_request_t;

typedef struct {
    unsigned int status;
    char type;
    float value;
} weather_response_t;

/* ------------------------------------------------------------------------------------------- */
							/* Utility Windows */
/* ------------------------------------------------------------------------------------------- */

static inline void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

#endif /* PROTOCOL_H_ */
