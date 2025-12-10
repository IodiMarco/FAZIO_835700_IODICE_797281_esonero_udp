/* ------------------------------------------------------------------------------------------- */
								/* Modulo Gestione del Server */
/* ------------------------------------------------------------------------------------------- */

#include "protocol.h"
#include <time.h>//necessario per la funzione srand()

/* ------------------------------------------------------------------------------------------- */
									/* Funzioni Meteo */
/* ------------------------------------------------------------------------------------------- */

float get_temperature(void) { return -10.0f + (rand() / (float)RAND_MAX) * 50.0f; }
float get_humidity(void)    { return 20.0f + (rand() / (float)RAND_MAX) * 80.0f; }
float get_wind(void)        { return (rand() / (float)RAND_MAX) * 100.0f; }
float get_pressure(void)    { return 950.0f + (rand() / (float)RAND_MAX) * 100.0f; }

const char* supported_cities[] = {
    "Bari","Roma","Milano","Napoli","Torino",
    "Palermo","Genova","Bologna","Firenze","Venezia"
};

int is_supported_city(const char* city) {
    for (int i = 0; i < 10; i++) {
        if (strcasecmp(city, supported_cities[i]) == 0) return 1;
    }
    return 0;
}

/* --- Validazione caratteri (no caratteri speciali) --- */
int is_valid_city_string(const char* city) {
    size_t len = strlen(city);
    for(size_t i = 0; i < len; i++) {
        char c = city[i];
        if (c == '\t') return 0; // Tab non ammesso
        if (!isalnum((unsigned char)c) && c != ' ' && c != '-' && c != '\'') return 0;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));
    int port = SERVER_PORT;

    if (argc == 3 && strcmp(argv[1], "-p") == 0)
        port = atoi(argv[2]);

#if defined WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
        printf("Errore WSAStartup\n"); return 0;
    }
#endif

    SOCKET socket_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_server == INVALID_SOCKET) {
        perror("Socket creation failed");
        clearwinsock();
        return -1;
    }

    struct sockaddr_in addr_server;
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port);
    addr_server.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_server, (struct sockaddr*)&addr_server, sizeof(addr_server)) < 0) {
        perror("Bind failed");
        closesocket(socket_server);
        clearwinsock();
        return -1;
    }

    printf("Server avviato sulla porta %d (Premi Ctrl+C per terminare)\n", port);

    while (1) {
        struct sockaddr_in addr_client;
#if defined WIN32
        int len = sizeof(addr_client);
#else
        socklen_t len = sizeof(addr_client);
#endif

        // 1. RICEZIONE
        char recv_buffer[sizeof(char) + MAX_CITY_LEN];

        int n = recvfrom(socket_server, recv_buffer, sizeof(recv_buffer), 0,
                        (struct sockaddr*)&addr_client, &len);

        if (n < 0) continue;

        // 2. REQUEST
        weather_request_t request;
        memcpy(&request.type, recv_buffer, sizeof(char));
        memcpy(request.city, recv_buffer + sizeof(char), MAX_CITY_LEN);
        request.city[MAX_CITY_LEN - 1] = '\0';

        // 3. LOGGING CON REVERSE DNS
        char host_client[NI_MAXHOST];
        char ip_client[INET_ADDRSTRLEN];

        // Conversione IP in stringa
        inet_ntop(AF_INET, &(addr_client.sin_addr), ip_client, INET_ADDRSTRLEN);

        // Risoluzione inversa
        if (getnameinfo((struct sockaddr*)&addr_client, len, host_client, NI_MAXHOST,
                        NULL, 0, 0) != 0) {
            strcpy(host_client, ip_client); // Fallback su IP se DNS fallisce
        }

        printf("Richiesta ricevuta da %s (ip %s): type='%c', city='%s'\n",
               host_client, ip_client, request.type, request.city);

        // 4. LOGICA DI RISPOSTA
        weather_response_t response;
        memset(&response, 0, sizeof(response));
        response.type = request.type; // Echo del tipo

        // Validazione validità caratteri
        if (!is_valid_city_string(request.city)) {
             response.status = STATUS_INVALID_REQUEST;
        }
        else if (request.type != TYPE_TEMPERATURE && request.type != TYPE_HUMIDITY &&
            request.type != TYPE_WIND && request.type != TYPE_PRESSURE) {
            response.status = STATUS_INVALID_REQUEST;
        }
        else if (!is_supported_city(request.city)) {
            response.status = STATUS_CITY_NOT_FOUND;
        }
        else {
            response.status = STATUS_OK;
            switch (request.type) {
                case TYPE_TEMPERATURE: response.value = get_temperature(); break;
                case TYPE_HUMIDITY:    response.value = get_humidity();    break;
                case TYPE_WIND:        response.value = get_wind();        break;
                case TYPE_PRESSURE:    response.value = get_pressure();    break;
            }
        }

        // 5. RESPONSE
        char send_buffer[sizeof(uint32_t) + sizeof(char) + sizeof(float)];
        int offset = 0;

        // A. Status
        uint32_t net_status = htonl(response.status);
        memcpy(send_buffer + offset, &net_status, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // B. Type
        memcpy(send_buffer + offset, &response.type, sizeof(char));
        offset += sizeof(char);

        // C. Value
        uint32_t net_temp;
        memcpy(&net_temp, &response.value, sizeof(float));
        net_temp = htonl(net_temp);                        // Host to Network
        memcpy(send_buffer + offset, &net_temp, sizeof(uint32_t));
        offset += sizeof(float);

        // 6. INVIO
        if (sendto(socket_server, send_buffer, offset, 0,
                  (struct sockaddr*)&addr_client, len) != offset) {
            printf("Errore invio risposta\n");
        }
    }

    closesocket(socket_server);
    clearwinsock();
    return 0;
}
