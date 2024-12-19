# CampiAgricoli_StazioneCasa

Questo progetto Arduino utilizza LoRa, WiFi e un display OLED per raccogliere dati da sensori ambientali e inviarli a un server tramite API REST. È progettato per monitorare temperatura, umidità dell'aria e del suolo.

## Funzionalità principali

- **LoRaWAN**: Ricezione dei dati tramite tecnologia LoRa.
- **WiFi**: Connessione a una rete WiFi per inviare dati a un server.
- **Display OLED**: Visualizzazione dello stato e delle informazioni principali.
- **Chiamata API**: Invio dei dati raccolti al server tramite un'API REST.

## Requisiti hardware

- **Scheda Arduino** compatibile con LoRaWAN e WiFi (es. Heltec LoRa).
- **Sensori** per temperatura e umidità dell'aria e del suolo.
- **Modulo LoRa** per la comunicazione.
- **Display OLED SSD1306**.
- **Connessione WiFi**.

## Dipendenze software

Prima di utilizzare questo progetto, installa le seguenti librerie nell'IDE Arduino:
- [LoRaWAN_APP](https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series_LoRaWAN)
- [WiFi](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi)
- [ArduinoJson](https://arduinojson.org/)
- [HT_SSD1306Wire](https://github.com/ThingPulse/esp8266-oled-ssd1306)
- [HTTPClient](https://github.com/espressif/arduino-esp32/tree/master/libraries/HTTPClient)

## Configurazione

1. **Impostazioni WiFi**: Modifica le seguenti variabili con il tuo SSID e password WiFi:
   ```cpp
   char ssid[] = "IlTuoSSID";
   char pass[] = "LaTuaPassword";
