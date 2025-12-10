/* ------------------------------------------------------------------------------------------- */
								/* Modulo Gestione del Client */
/* ------------------------------------------------------------------------------------------- */

#include "protocol.h"

/* ------------------------------------------------------------------------------------------- */
							       /* Main e inizializzazione */
/* ------------------------------------------------------------------------------------------- */

int main(int argc, char* argv[]) {
    int server_port = SERVER_PORT;
    char* server_addr_str = "localhost";
    char type = 0;
    char city[MAX_CITY_LEN] = "";
    int city_set = 0;

    // --- 1. PARSING ARGOMENTI ---
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            server_addr_str = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            server_port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            char* req_str = argv[++i];

            // Verifica TAB
            if (strchr(req_str, '\t') != NULL) {
                printf("Errore: La richiesta contiene caratteri di tabulazione\n");
                return -1;
            }

            // Parsing "type city"
            char* space_pos = strchr(req_str, ' ');
            if (space_pos == NULL) {
                printf("Formato richiesta non valido. Uso: \"type city\"\n");
                return -1;
            }

            // Lunghezza del type
            int type_len = space_pos - req_str;
            if (type_len != 1) {
                printf("Errore: Il tipo deve essere un singolo carattere\n");
                return -1;
            }

            type = req_str[0];

            // Copia città
            char* city_start = space_pos + 1;

            // Verifica lunghezza città
            if (strlen(city_start) >= MAX_CITY_LEN) {
                printf("Errore: Nome città troppo lungo (max 63 caratteri)\n");
                return -1;
            }

            strncpy(city, city_start, MAX_CITY_LEN - 1);
            city[MAX_CITY_LEN - 1] = '\0';
            city_set = 1;
        }
    }

    if (!city_set) {
        printf("Uso: %s [-s server] [-p port] -r \"type city\"\n", argv[0]);
        return -1;
    }

#if defined WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != NO_ERROR) {
        printf("WSAStartup failed\n"); return -1;
    }
#endif

    // --- 2. DNS SERVER ---
    struct hostent *he;
    struct sockaddr_in server_addr;

    // Risoluzione nome
    if ((he = gethostbyname(server_addr_str)) == NULL) {
        printf("Errore risoluzione host: %s\n", server_addr_str);
        clearwinsock();
        return -1;
    }

    SOCKET sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        perror("Socket creation failed");
        clearwinsock();
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);

    // --- 3. REQUEST ---
    char send_buffer[sizeof(char) + MAX_CITY_LEN];
    int offset = 0;

    memcpy(send_buffer + offset, &type, sizeof(char));
    offset += sizeof(char);

    memset(send_buffer + offset, 0, MAX_CITY_LEN);
    strncpy(send_buffer + offset, city, MAX_CITY_LEN - 1);
    offset += MAX_CITY_LEN;

    // --- 4. INVIO ---
    if (sendto(sock, send_buffer, offset, 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr)) != offset) {
        perror("Errore sendto");
        closesocket(sock);
        clearwinsock();
        return -1;
    }

    // --- 5. RICEZIONE ---
    char recv_buffer[512]; // Dimensione ampia a sufficienza
    struct sockaddr_in from_addr;
#if defined WIN32
    int from_len = sizeof(from_addr);
#else
    socklen_t from_len = sizeof(from_addr);
#endif

    int n = recvfrom(sock, recv_buffer, sizeof(recv_buffer), 0,
                     (struct sockaddr*)&from_addr, &from_len);

    if (n < 0) {
        perror("Errore recvfrom");
        closesocket(sock);
        clearwinsock();
        return -1;
    }

    // --- 6. RESPONSE ---
    weather_response_t response;
    offset = 0;

    // A. Status
    uint32_t net_status;
    memcpy(&net_status, recv_buffer + offset, sizeof(uint32_t));
    response.status = ntohl(net_status);
    offset += sizeof(uint32_t);

    // B. Type
    memcpy(&response.type, recv_buffer + offset, sizeof(char));
    offset += sizeof(char);

    // C. Value
    uint32_t net_value;
    memcpy(&net_value, recv_buffer + offset, sizeof(uint32_t));
    net_value = ntohl(net_value);
    memcpy(&response.value, &net_value, sizeof(float));


    // --- 7. OUTPUT ---

    // IP in stringa
    char server_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(from_addr.sin_addr), server_ip, INET_ADDRSTRLEN);

    // Reverse Lookup per ottenere il Nome Server
    char server_name[NI_MAXHOST];
    if (getnameinfo((struct sockaddr*)&from_addr, from_len, server_name, NI_MAXHOST,
        NULL, 0, 0) != 0) {
        // Se il reverse lookup fallisce, usa l'IP come nome
        strcpy(server_name, server_ip);
    }

    printf("Ricevuto risultato dal server %s (ip %s). ", server_name, server_ip);

    if (response.status == STATUS_CITY_NOT_FOUND) {
        printf("Città non disponibile\n");
    }
    else if (response.status == STATUS_INVALID_REQUEST) {
        printf("Richiesta non valida\n");
    }
    else {
        city[0] = toupper(city[0]);

        switch (response.type) {
            case TYPE_TEMPERATURE:
                printf("%s: Temperatura = %.1f°C\n", city, response.value);
                break;
            case TYPE_HUMIDITY:
                printf("%s: Umidità = %.1f%%\n", city, response.value);
                break;
            case TYPE_WIND:
                printf("%s: Vento = %.1f km/h\n", city, response.value);
                break;
            case TYPE_PRESSURE:
                printf("%s: Pressione = %.1f hPa\n", city, response.value);
                break;
            default:
                printf("Tipo risposta sconosciuto\n");
        }
    }

    closesocket(sock);
    clearwinsock();
    return 0;
}
