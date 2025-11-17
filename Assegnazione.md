# Primo Esonero di Laboratorio - Reti di Calcolatori (ITPS A-L) 2025-26

## Obiettivo Generale
Realizzare un'applicazione **UDP** client/server che implementa un servizio meteo, dove il server risponde alle richieste climatiche dei client utilizzando il protocollo UDP (User Datagram Protocol).

## Differenze rispetto al TCP
Questa assegnazione richiede l'implementazione di un'applicazione basata su **UDP** anziché TCP. Le principali differenze da considerare sono:

- **Connectionless**: UDP non stabilisce una connessione prima dello scambio dati
- **Inaffidabilità**: UDP non garantisce la consegna dei pacchetti, né il loro ordine
- **Dimensione pacchetti**: Considerare la dimensione massima dei datagrammi UDP
- **Timeout e ritrasmissione**: Necessario implementare meccanismi di gestione della perdita di pacchetti

## Protocollo Applicativo

### Strutture Dati

**Richiesta Client:**
```c
char type;      // 't'=temperatura, 'h'=umidità, 'w'=vento, 'p'=pressione
char city[64];  // nome città (null-terminated)
```

**Risposta Server:**
```c
unsigned int status;  // 0=successo, 1=città non trovata, 2=richiesta invalida
char type;            // eco del tipo richiesto
float value;          // dato meteo generato
```

### Formati di Output

**Successo (status=0):**
- Temperatura: `"NomeCittà: Temperatura = XX.X°C"`
- Umidità: `"NomeCittà: Umidità = XX.X%"`
- Vento: `"NomeCittà: Vento = XX.X km/h"`
- Pressione: `"NomeCittà: Pressione = XXXX.X hPa"`

**Errori:**
- Status 1: "Città non disponibile"
- Status 2: "Richiesta non valida"
- Timeout: "Timeout: nessuna risposta dal server"

## Interfaccia Client

**Sintassi:**
```
./client-project [-s server] [-p port] -r "type city"
```

**Parametri:**
- `-s server`: indirizzo server (default: 127.0.0.1)
- `-p port`: porta server (default: 56700)
- `-r request`: richiesta meteo obbligatoria

**Flusso operativo:**
1. Analizza argomenti da linea di comando
2. Crea socket UDP (SOCK_DGRAM)
3. Invia richiesta al server con `sendto()`
4. Attende risposta con timeout (3 secondi)
5. In caso di timeout, ritrasmette la richiesta (massimo 3 tentativi totali)
6. Riceve risposta con `recvfrom()`
7. Visualizza risultato formattato
8. Chiude socket

**Gestione errori:**
- Dopo 3 tentativi senza risposta, il client deve terminare con un messaggio di errore
- Implementare `setsockopt()` con `SO_RCVTIMEO` per il timeout di ricezione

## Interfaccia Server

**Sintassi:**
```
./server-project [-p port]
```

**Parametri:**
- `-p port`: porta ascolto (default: 56700)

**Comportamento:**
Il server rimane attivo continuamente in ascolto sulla porta specificata. Per ogni datagramma ricevuto:
1. Riceve richiesta con `recvfrom()` (acquisendo indirizzo client)
2. Valida il tipo di richiesta e il nome della città
3. Genera il valore meteo con le funzioni `get_*()`
4. Invia risposta al client con `sendto()` usando l'indirizzo acquisito
5. Continua in ascolto per nuove richieste

**Note:**
- Non c'è fase di "connessione" o "accettazione" come in TCP
- Il server può gestire richieste da client multipli in modo concorrente
- Ogni richiesta è indipendente e stateless

## Funzioni di Generazione Dati

Il server implementa quattro funzioni che generano valori casuali:
- `get_temperature()`: temperatura casuale tra -10.0 e 40.0 °C
- `get_humidity()`: umidità casuale tra 20.0 e 100.0%
- `get_wind()`: velocità vento casuale tra 0.0 e 100.0 km/h
- `get_pressure()`: pressione casuale tra 950.0 e 1050.0 hPa

## Città Supportate

Il server deve riconoscere almeno 10 città italiane (confronto case-insensitive):
- Bari
- Roma
- Milano
- Napoli
- Torino
- Palermo
- Genova
- Bologna
- Firenze
- Venezia

## Requisiti Tecnici

### 1. Organizzazione del Codice
- **File header `protocol.h`**: contiene le definizioni delle strutture dati, i prototipi delle funzioni e le costanti condivise tra client e server
- **File sorgente client**: `client.c` nell'apposita directory
- **File sorgente server**: `server.c` nell'apposita directory

### 2. Portabilità Multi-Piattaforma
Il codice deve compilare ed eseguire correttamente su:
- Windows (utilizzare Winsock2)
- Linux
- macOS

Utilizzare direttive di preprocessore (`#ifdef _WIN32`) per gestire le differenze tra piattaforme.

### 3. Gestione Memoria e Sicurezza
- Nessun buffer overflow
- Nessun memory leak
- Validazione corretta degli input utente
- Gestione appropriata degli errori di sistema

### 4. Socket UDP
- Utilizzare `SOCK_DGRAM` per creare socket UDP
- Implementare timeout sul socket di ricezione del client
- Gestire correttamente gli indirizzi con `struct sockaddr_in`

### 5. Timeout e Ritrasmissione
- Timeout di ricezione: 3 secondi
- Numero massimo di ritrasmissioni: 3 tentativi totali
- Messaggio di errore appropriato se tutti i tentativi falliscono

### 6. Dimensioni Datagrammi
- Dimensione massima datagramma: 512 byte
- Verificare che le strutture dati non eccedano questo limite

### 7. Compatibilità Eclipse CDT
Il progetto deve essere compatibile con Eclipse CDT e includere i file di configurazione necessari (.project, .cproject).

## Consegna

- **Scadenza:** 28 novembre 2025, ore 23:59:59
- **Formato:** link repository GitHub pubblico contenente:
  - Codice sorgente completo
  - File `protocol.h`
  - Makefile o script di build
  - File di progetto Eclipse (.project, .cproject)
  - README.md con istruzioni di compilazione ed esecuzione
- **Note:**
  - Una sola consegna per coppia di studenti
  - Utilizzare il form di consegna fornito dal docente
  - Testare il codice su almeno due piattaforme diverse

## Suggerimenti Implementativi

### Per il Client
```c
// Esempio di impostazione timeout
struct timeval timeout;
timeout.tv_sec = 3;
timeout.tv_usec = 0;
setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
```

### Per il Server
```c
// Il server usa recvfrom per ottenere l'indirizzo del client
struct sockaddr_in client_addr;
socklen_t client_len = sizeof(client_addr);
recvfrom(socket_fd, buffer, size, 0, (struct sockaddr*)&client_addr, &client_len);

// Risponde allo stesso indirizzo
sendto(socket_fd, response, size, 0, (struct sockaddr*)&client_addr, client_len);
```

## Criteri di Valutazione

- **Correttezza funzionale** (40%): il programma funziona come specificato
- **Gestione UDP** (25%): corretta implementazione del protocollo UDP con timeout e ritrasmissione
- **Portabilità** (15%): il codice compila ed esegue su Windows, Linux e macOS
- **Qualità del codice** (10%): leggibilità, commenti, organizzazione
- **Gestione errori** (10%): validazione input, gestione errori di rete e sistema

---

**Buon lavoro!**
