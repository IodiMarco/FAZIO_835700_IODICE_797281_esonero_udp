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

**IMPORTANTE**: Il protocollo applicativo rimane **identico** all'assegnazione TCP. Le strutture dati definite in `protocol.h` **NON devono essere modificate**.

### Strutture Dati

**Richiesta Client:**
```c
struct request {
    char type;
    char city[64];
};
```

**Risposta Server:**
```c
struct response {
    unsigned int status;
    char type;
    float value;
};
```

### Network Byte Order

> [!WARNING]
> Uso corretto di htonl/ntohl e conversione float come mostrato a lezione.

## Interfaccia Client

**Sintassi:**
```
./client-project [-s server] [-p port] -r "type city"
```

> [!WARNING]
> Validazione lato client chiarita:
> - `type` ∈ {t,h,w,p} → altrimenti errore **lato client e nessun invio**
> - nessuna normalizzazione spazi
> - city max 63 caratteri → se troppo lunga: errore lato client

Il client deve effettuare **reverse DNS lookup** dell’IP del server per mostrarne il nome.

## Interfaccia Server

Il server deve:
- ricevere datagrammi
- validare richiesta
- generare risposta
- effettuare **reverse DNS** sul client per logging

## Logging
Esempio:
```
Richiesta ricevuta da <nomeclient> (ip X.X.X.X): type='t', city='Roma'
```

## Output Client (formato rigido)
> [!WARNING]
> Obbligatorio mantenere EXACT format, italiana, una cifra decimale.

Esempi, come originali.

## Città Supportate
Bari, Roma, Milano, Napoli, Torino, Palermo, Genova, Bologna, Firenze, Venezia.

Regole chiarite:
- case-insensitive
- spazi ok
- no tab
- nessuna normalizzazione.

## DNS (riorganizzato senza ripetizioni)
> [!NOTE]
> - Client: forward + reverse DNS  
> - Server: reverse DNS per logging  

## Requisiti tecnici, consegna
Invariati.

_Ver. 1.0.1_
