## Rotilio

![Rotilio, piattaforma IoT hw e sw open source](./images/tm_rotilio.jpg "Rotilio")

Piattaforma IoT hardware e software open source per makers, architetti, ingegneri e sviluppatori.
Basato sullo standard di programmazione Arduino, connesso al cloud, subito pronto ad interagire con App e l'ambiente che lo circonda.

```
"Considerate sempre gli errori come parte del processo. Non potete evitarli ma potete usarli a vostro vantaggio" 
- Alessio Ricco esperto di Co-Design
```

Quando abbiamo scoperto Arduino, siamo rimasti colpiti dalla semplicità con cui si possano realizzare prototipi e installazioni, ma spingendoci oltre, e volendo ingegnerizzare la soluzione in un prodotto nostro (il classico termostato comandato da un App :-) )  ci siamo scontrati con diversi problemi: costi, dimensioni, consumo della batteria, connettività.

Abbiamo passato qualche tempo a cercare soluzioni, ma nulla di fatto.

Allora abbiamo deciso di costruire noi una piattaforma che comprendesse già una serie di componenti configurabili a seconda del contesto di utilizzo.

E' nato così Rotilio, un dispositivo pensato non solo per sperimentare ma anche per essere utilizzato sul campo in soluzioni per utente finale.

L'idea è quella di fornire ad architetti, ingegneri e sviluppatori un sistema completo per connettere le cose alla rete.

La procedura di avviamento è semplicissima: tiri fuori Rotilio dalla sua scatola (o anche no) e lo accendi.

Con un App (Android o iOS) puoi facilmente configurare la connessione alla rete e creare il tuo account sul cloud.

Subito dopo potrai leggere i valori di temperatura, pressione, umidità, luminosità e comandare un eventuale utenza collegata al relais di Rotilio.

Tutto senza scrivere una riga di codice e senza specifiche conoscenze tecnologiche.

Rotilio è hardware e software OPEN SOURCE, per questo potrai essere tu o un tuo partner a decidere quanto spingerti in profondità nella personalizzazione o espansione della piattaforma con aggiunta di altri sensori o attuatori o nell'integrazione con siti Web e APPs, nuovi o esistenti.

Tutto è pronto anche per la distribuzione e l'installazione sul campo, procedure di setup, aggiornamenti firmware, dashboard di controllo.

Il cuore di Rotilio è il microcontrollore WiFi Particle Photon [http://particle.io](), ma è predisposto anche per accogliere il Particle Electron, per connettività GPRS o 3G.

L'intelligenza è costituita dal firmware che abbiamo scritto appositamente per rendere semplice e accessibile tutto questo.

L'interfaccia WEB standard è disponibile a questo indirizzo: [http://techmakers.io/rotilio]()

## Applicazioni

I sensori presenti a bordo di Rotilio permettono la lettura di temperatura, umidità, pressione, luminosità. Inoltre il relais può comandare carichi in funzione delle impostazioni del firmware o ricevute dalle applicazioni in cloud.
Sono presenti anche uno switch a due posizioni, due pulsanti, un led RGB e un buzzer utili per segnalare situazioni e interagire con le persone.
Il "pettine di espansione" permette di aggiungere facilmente altri sensori o attuatori.
Le sue ridotte dimensioni (70x70x20 mm) permettono una facile integrazione anche in piccoli oggetti.


Alcune idee per architetti:

- Illuminazione interattiva e connessa
- Stazione meteo
- Termostato wifi
- Apri porta
- Oggetti connessi come: sveglie intelligenti, lampade reattive, stufe automatiche
- Meccanizzazione mobili e complementi d'arredo
- Segnalazione luci accese nelle stanze
- Verifica condizioni ambientali ideali per gli ambienti

Alcune idee per ingegneri:

- Monitoraggio produzione energia rinnovabile
- Monitoraggio consumi energetici
- Monitoraggio condizioni ambientali
- Sicurezza cantieri
- Reti di sensori
- Sensore allagamento ambienti
- Sensore livello liquidi
- Irrigazione automatica
- Ventilazione ambienti
- Comando remoto di utenze elettriche
- Allarme black-out
- Apri cancello
- Segnalazione presenza
- Interfacciamento WiFI PLC
- Reti di sensori RS485

Alcune idee per makers:

- Robot
- Altimetri
- Giochi
- Timer telecontrollati
- Allarme frigo aperto
- Integrazione social network con oggetti fisici


## Scheda tecnica di Rotilio

- Sensore temperatura, precisione 0.1°C
- Sensore umidità, precisione 1%
- Sensore pressione,  precisione 1 mbar
- Sensore luminosità, risoluzione 12 bit
- Potenziometro, risoluzione 12 bit
- Switch a due posizioni
- Pulsanti (2x)
- Relais (mono o bi-stabile)
- Porta RS485
- Porta Seriale TTL
- Ingresso alimentazione 5V
- Batteria tampone per stand-by
- Pettine espansione I/O Digitale e Analogico
- Pettine espansione sensori di bordo
- Connettore per Particle Photon o Particle Electron [http://particle.io]()

### Caratteristiche

- Dimensioni: 70x70x20mm
- Assorbimento: da 3 microA a 450 mA, a seconda del modo di funzionamento
- Connettività WiFI con o senza cifratura
- Connessione semplificata WiFI mediante apposita APP Android e iOS
- Integrazione con il cloud per lettura delle variabili, invio allarmi, ricezione comandi
- Aggiornamento firmware da remoto
- Programmazione firmware in cloud con un semplice browser
- API di programmazione via HTTP, Javascript, Android, iOS
- Open source

## Techmakers

Techmakers costruisce hardware e software per l'internet delle cose.

Se vuoi realizzare oggetti interattivi e connessi sei nel posto giusto.

Con noi puoi realizzare prototipi ma anche versioni commerciali, supportato in tutto il ciclo di vita della tua idea. 

Anche quando avrai migliaia di dispositivi installati, potrai aggiornarli e gestirli da remoto.

La nostra piattaforma, composta da hardware e software perfettamente integrati tra loro, costituisce un ottimo punto di partenza per ogni tuo progetto. 

Sappiamo che quando si intraprende un viaggio nello sviluppo di una nuova idea è bello essere affiancati da compagni di viaggio discreti ma pronti a dare una mano quando serve. 
Noi e la nostra comunità di partner certificati possiamo essere quella mano utile per suggerimenti, analisi di soluzioni, test e anche co-progettazione.

## Per saperne di più

Visita il nostro sito: [http://techmakers.io]()
