#include "LoRaWan_APP.h"          // Libreria LoRaWAN per la gestione della comunicazione LoRa
#include "Arduino.h"               // Libreria di base per la gestione dell'IDE Arduino
#include "WiFi.h"                  // Libreria per la connessione WiFi
#include <Wire.h>                  // Libreria per la comunicazione I2C
#include "HT_SSD1306Wire.h"        // Libreria per il controllo del display OLED SSD1306
#include <HTTPClient.h>            // Libreria per inviare richieste HTTP
#include <ArduinoJson.h>           // Libreria per la gestione di JSON

// SEZIONE LORA

#define RF_FREQUENCY 915000000    // Frequenza LoRa (915 MHz)
#define TX_OUTPUT_POWER 14        // Potenza di trasmissione (dBm)
#define LORA_BANDWIDTH 0          // Larghezza di banda (125 kHz)
#define LORA_SPREADING_FACTOR 7   // Fattore di spreading (SF7)
#define LORA_CODINGRATE 1         // Codifica (4/5)
#define LORA_PREAMBLE_LENGTH 8    // Lunghezza del preambolo
#define LORA_SYMBOL_TIMEOUT 0     // Timeout simboli
#define LORA_FIX_LENGTH_PAYLOAD_ON false // Payload a lunghezza variabile
#define LORA_IQ_INVERSION_ON false // Inversione IQ disabilitata

#define RX_TIMEOUT_VALUE 1000     // Timeout per la ricezione
#define BUFFER_SIZE 100            // Dimensione del buffer per il pacchetto ricevuto (aumentato per contenere stringhe più lunghe)

char rxpacket[BUFFER_SIZE];      // Buffer per il pacchetto ricevuto
static RadioEvents_t RadioEvents; // Gestione degli eventi radio
int16_t rssi, rxSize;           // RSSI e dimensione del pacchetto ricevuto
bool lora_idle = true;           // Flag per verificare se LoRa è inattivo

// FINE SEZIONE LORA

// SEZIONE DISPLAY

SSD1306Wire Display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // Inizializza il display OLED

// FINE DISPLAY

// SEZIONE COLLEGAMENTO WIFI

// SSID e password della rete WiFi
char ssid[] = "IlTuoSSID";  // Nome della rete WiFi
char pass[] = "LaTuaPassword";    // Password della rete WiFi

// Funzione per configurare il WiFi
void WifiSetup(){
  WiFi.disconnect(true);              // Disconnessione da eventuali reti precedenti
  delay(100);                         // Piccola pausa
  WiFi.mode(WIFI_STA);                // Imposta il dispositivo in modalità STATION (client)
  WiFi.setAutoReconnect(true);        // Abilita il riconnessione automatica
  WiFi.begin(ssid, pass);             // Connessione alla rete WiFi
  delay(100);

  byte count = 0;                     // Contatore per il tentativo di connessione
  while(WiFi.status() != WL_CONNECTED && count < 10) { // Tenta la connessione fino a 10 volte
    count ++;
    delay(500);
    Display.drawString(0, 0, "Connecting..."); // Mostra il tentativo di connessione sul display
    Display.display();
  }

  Display.clear();                    // Pulisce lo schermo
  if(WiFi.status() == WL_CONNECTED) { // Se la connessione è riuscita
    Display.drawString(0, 0, "Connecting...OK."); // Mostra il messaggio di successo
    Display.display();

    // Ottieni e mostra l'indirizzo IP
    IPAddress localIP = WiFi.localIP();
    String ipStr = "IP: " + localIP.toString();
    Display.drawString(0, 10, ipStr); // Visualizza l'IP
    Display.display();
  } else { // Se la connessione fallisce
    Display.clear();
    Display.drawString(0, 0, "Connecting...Failed");
    Display.display();
  }
  Display.drawString(0, 20, "WIFI Setup done"); // Messaggio finale
  Display.display();
  delay(500);
}

// FINE SEZIONE WIFI

// SEZIONE CHIAMATA API

// Funzione per inviare i dati tramite HTTP
void sendDataToAPI(int soilMoisture, float soilTemperature, float airHumidity, float airTemperature) {
    HTTPClient http;  // Oggetto per la comunicazione HTTP
    String apiUrl = "http://172.20.10.7:5001/Misurazioni/PostMisurazione"; // URL dell'API

    // Crea un oggetto JSON
    StaticJsonDocument<1024> doc; // Oggetto JSON di dimensione 1024 byte

    // Aggiungi le misurazioni al documento JSON
    JsonArray measurements = doc.to<JsonArray>();  // Array di misurazioni

    // Aggiungi ogni misurazione con l'ID sensore e il valore
    for (int i = 1; i <= 4; i++) {  // Itera su tutte le misurazioni
        JsonObject measurement = measurements.createNestedObject();  // Crea un oggetto per ogni misurazione
        measurement["IdMisurazione"] = 0; 
        measurement["IdSensore"] = i;  // Imposta l'ID del sensore
        measurement["valoreMisurazione"] = (i == 1) ? airHumidity : (i == 2) ? soilMoisture : (i == 3) ? airTemperature : soilTemperature; // Imposta il valore
        measurement["dataOraCertaMisurazione"] = "1900-01-01T00:00:00.000Z";  // Data di default
    }

    // Serializza il documento JSON in una stringa
    String jsonPayload;
    serializeJson(doc, jsonPayload);

    Serial.printf("Json che sto inviando: %s\n", jsonPayload.c_str()); // Stampa il JSON che verrà inviato

    // Configura la richiesta HTTP
    http.begin(apiUrl);  // Inizia la connessione all'API
    http.addHeader("Content-Type", "application/json");  // Aggiungi header per JSON

    // Invia la richiesta POST
    int httpResponseCode = http.POST(jsonPayload);

    // Controlla la risposta dell'API
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);  // Stampa il codice di risposta
        Serial.printf("Response: %s\n", response.c_str());  // Stampa la risposta dell'API
    } else {  // Se c'è un errore
        Serial.printf("Error on sending POST: %s\n", http.errorToString(httpResponseCode).c_str());  // Stampa l'errore
    }

    http.end();  // Chiudi la connessione HTTP
}

// FINE SEZIONE CHIAMATA API

// Variabili per sensori
float soilTemperature, airHumidity, airTemperature;  // Variabili per la temperatura e umidità
int soilMoisture;  // Umidità del suolo

void setup() {
    Serial.begin(115200);  // Inizializza la seriale per il debug

    Display.init();  // Inizializza il display
    Display.clear();  // Pulisce il display

    WifiSetup();  // Configura il WiFi

    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);  // Inizializza la board

    // Configura gli eventi LoRa
    RadioEvents.RxDone = OnRxDone;
    Radio.Init(&RadioEvents);  // Inizializza la radio LoRa
    Radio.SetChannel(RF_FREQUENCY);  // Imposta la frequenza
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);  // Configura la ricezione LoRa
}

void loop() {
    if (lora_idle) {
        lora_idle = false;  // Imposta LoRa come non inattivo
        Serial.println("Into RX mode");  // Stampa il messaggio di ricezione
        Radio.Rx(0);  // Inizia la ricezione
    }
    Radio.IrqProcess();  // Gestisce le interruzioni LoRa
}

// Funzione per gestire la ricezione dei dati
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    rxSize = size;  // Imposta la dimensione del pacchetto ricevuto
    memcpy(rxpacket, payload, size);  // Copia il payload nel buffer
    rxpacket[size] = '\0';  // Termina la stringa ricevuta

    // Debug per visualizzare i dati ricevuti in formato esadecimale
    Serial.print("Ricevuto pacchetto: ");
    for (int i = 0; i < size; i++) {
        Serial.print(rxpacket[i], HEX);
        Serial.print(" ");
    }
    Serial.println();  // A capo dopo l'output esadecimale

    if (strlen((char*)rxpacket) > 0) {
        int parsed = sscanf(rxpacket, "SoilMoisture:%d SoilTemp:%f AirHum:%f AirTemp:%f", 
                            &soilMoisture, &soilTemperature, &airHumidity, &airTemperature);
        
        // Controllo per vedere se il parsing ha avuto successo
        if (parsed == 4) {
            Serial.printf("VALORI ESTRATTI: SoilMoisture: %d, SoilTemp: %.2f°C, AirHum: %.2f%%, AirTemp: %.2f°C\r\n", 
                           soilMoisture, soilTemperature, airHumidity, airTemperature);

            // Invia i dati all'API
            sendDataToAPI(soilMoisture, soilTemperature, airHumidity, airTemperature);
        } else {
            Serial.println("Errore nel parsing del pacchetto ricevuto!");
        }
    } else {
        Serial.println("Pacchetto vuoto ricevuto!");
    }

    // Aggiungi un piccolo delay per evitare che la ricezione sia troppo veloce
    delay(1500);

    Radio.Sleep();  // Metti LoRa in modalità sleep
    lora_idle = true;  // Riporta LoRa a stato inattivo
}


