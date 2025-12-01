# Esonero di Laboratorio UDP - Reti di Calcolatori (ITPS A-L) 2025-26 :santa: :christmas_tree: :gift:

## Obiettivo Generale
**Migrare** l'applicazione client/server del servizio meteo dal protocollo **TCP** (realizzata nel primo esonero) al protocollo **UDP** (User Datagram Protocol).

L'obiettivo è modificare il codice già scritto per l'assegnazione TCP in modo da usare UDP come protocollo di trasporto, mantenendo invariato il protocollo applicativo (strutture dati, formati, funzionalità).

## Cosa Cambia nella Migrazione da TCP a UDP

### Modifiche da Apportare al Codice

Nel passaggio da TCP a UDP, dovrete modificare **solo il livello di trasporto**, lasciando invariato tutto il resto:

**DA MODIFICARE:**
- Tipo di socket
- Chiamate di sistema
- Output

**NON MODIFICARE:**
- Protocollo applicativo: strutture `struct request` e `struct response`
- Interfaccia a linea di comando (opzioni `-s`, `-p`, `-r`)
- Logica di business: parsing richieste, validazione città, generazione dati meteo
- Funzioni `get_temperature()`, `get_humidity()`, `get_wind()`, `get_pressure()`

## Protocollo Applicativo

**IMPORTANTE**: Il protocollo applicativo rimane **identico** all'assegnazione TCP.  
Le strutture dati definite in `protocol.h` **NON devono essere modificate**.

### Strutture Dati

**Richiesta Client:**
```c
struct request {
    char type;      // 't'=temperatura, 'h'=umidità, 'w'=vento, 'p'=pressione
    char city[64];  // nome città (null-terminated)
};
```

**Risposta Server:**
```c
struct response {
    unsigned int status;  // 0=successo, 1=città non trovata, 2=richiesta invalida
    char type;            // eco del tipo richiesto
    float value;          // dato meteo generato
};
```

### Network Byte Order

- `unsigned int status`: usare `htonl()` prima dell'invio e `ntohl()` dopo la ricezione  
- `float value`: convertire in formato network byte order usando la tecnica mostrata a lezione (float → uint32_t → htonl/ntohl → float)  
- `char type` e `char city[]`: non richiedono conversione

Esempio conversione float:
```c
uint32_t temp;
memcpy(&temp, &value, sizeof(float));
temp = htonl(temp);
memcpy(&response.value, &temp, sizeof(float));

memcpy(&temp, &response.value, sizeof(float));
temp = ntohl(temp);
memcpy(&value, &temp, sizeof(float));
```

### Formati di Output

- **Lingua italiana obbligatoria**  
- **Formato esatto**  
- **Nessun carattere extra**

Il client deve stampare **il nome del server ottenuto tramite reverse DNS lookup**, seguito dal suo **indirizzo IP**.  
Se il reverse DNS fallisce, il client mostra direttamente l’indirizzo IP.

**Successo (`status=0`):**
- `"Ricevuto risultato dal server <nomeserver> (ip <IP>). NomeCittà: Temperatura = XX.X°C"`
- `"Ricevuto risultato dal server <nomeserver> (ip <IP>). NomeCittà: Umidità = XX.X%"`
- `"Ricevuto risultato dal server <nomeserver> (ip <IP>). NomeCittà: Vento = XX.X km/h"`
- `"Ricevuto risultato dal server <nomeserver> (ip <IP>). NomeCittà: Pressione = XXXX.X hPa"`

**Errori:**
- `status 1`: `"Ricevuto risultato dal server <nomeserver> (ip <IP>). Città non disponibile"`
- `status 2`: `"Ricevuto risultato dal server <nomeserver> (ip <IP>). Richiesta non valida"`

### Esempi di Output

*(identici all'assegnazione originale, invariati)*

---

## Interfaccia Client

```
./client-project [-s server] [-p port] -r "type city"
```

### Parametri
- `-s server`: hostname o indirizzo IP (default `localhost`)
- `-p port`: porta server (default `56700`)
- `-r request`: richiesta meteo

### Parsing e Validazione (correzioni integrate)
- `type` deve essere **uno dei caratteri**: `t`, `h`, `w`, `p`  
  → altrimenti errore lato client, **nessun invio**
- **Nessuna normalizzazione degli spazi** della città
- Tabulazioni non ammesse
- `city` max 63 caratteri + `\0`  
  → se supera → **errore lato client**, nessun invio

### Flusso Operativo
1. Parsing argomenti
2. DNS forward: risoluzione server → IP
3. Creazione socket UDP
4. Invio richiesta
5. Ricezione risposta
6. **Reverse DNS** dell'IP del server  
   - se fallisce → mostra l’IP
7. Stampa formattata
8. Chiusura socket

---

## Interfaccia Server

```
./server-project [-p port]
```

### Comportamento
1. Crea socket UDP + bind
2. Ciclo infinito:
   - Riceve datagramma + indirizzo client
   - **Reverse DNS** dell’indirizzo client per i log  
     - se fallisce → mostra solo IP
   - Valida richiesta
   - Genera valore meteo
   - Invia risposta

### Esempio Log
```
Richiesta ricevuta da <nomeclient> (ip 127.0.0.1): type='t', city='Roma'
```
Se reverse DNS fallisce:
```
Richiesta ricevuta da 127.0.0.1: type='t', city='Roma'
```

---

## Funzioni di Generazione Dati

*(invariato rispetto all'originale)*

---

## Città Supportate

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

### Regole di validazione
- Case-insensitive  
- **Spazi multipli ammessi** (nessuna normalizzazione)  
- Nessuna tabulazione  
- Nessun carattere speciale

---

## Requisiti Tecnici

- Nessun overflow  
- Nessun memory leak  
- Input validati correttamente  
- Network byte order corretto  
- Supporto Windows / Linux / macOS  
- Nessun invio dal client in caso di input invalido

---

## Consegna

- **Scadenza**: xx dicembre 2025, entro h. 23.59.59  
- **Form**: https://forms.gle/P4kWH3M3zjXjsWWP7  
- **Formato**: repository GitHub pubblico  
- Una sola consegna per coppia  
- La pagina risultati si aggiorna ogni ora

_Ver. 1.0.1 (con chiarimenti integrati)_
