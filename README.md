# Dispozitiv portabil pentru monitorizarea în timp real a animalelor de companie

Acest proiect implementează un sistem embedded bazat pe **ESP32 DevKitC v4** care monitorizează poziția unui animal de companie în timp real, folosind un **modul GPS NEO-6M** pentru achiziția datelor de localizare și un **modul Bluetooth HC-05** pentru transmiterea acestora către utilizator.  
Software-ul rulează pe **FreeRTOS**, asigurând execuția concurentă a mai multor task-uri și sincronizarea lor prin mecanisme RTOS.

---

## Cerințe minimale – Îndeplinire

- **Utilizare sistem de operare de timp real**  
  Proiectul folosește **FreeRTOS**, integrat în ESP-IDF.

- **Cel puțin două procese**  
  - Task GPS – achiziție și procesare date NMEA.  
  - Task Bluetooth – transmitere date prin HC-05.  

- **Un proces în timp real cu prelucrare locală**  
  Task-ul GPS rulează în timp real și interacționează cu mediul extern prin modulul GPS și LED-ul indicator.  

- **Un proces care implementează un protocol de comunicație la distanță**  
  Task-ul Bluetooth implementează protocolul SPP (Serial Port Profile) prin HC-05.  

- **Procese independente, dar sincronizate**  
  - Datele sunt transmise din Task GPS către Task Bluetooth printr-o **coadă FreeRTOS (queue)**.  
  - Sincronizarea se face cu **semafoare și mutex-uri**.  

- **Tratarea condițiilor de concurență**  
  - Mutex pentru acces exclusiv la UART.  
  - Semafor pentru semnalizarea mesajelor noi.  

---

## Cerințe specifice – Îndeplinire

- **Microcontroller ales de student**  
  ESP32 DevKitC v4 (dual-core, 240 MHz, suport Wi-Fi și Bluetooth).  

- **Interfață de intrare și ieșire**  
  - Intrare: date GPS prin UART.  
  - Ieșire: transmisie Bluetooth HC-05 și LED indicator.  

- **Folosirea resurselor integrate**  
  - UART2 – pentru GPS.  
  - UART1 – pentru Bluetooth.  
  - GPIO – pentru LED.  
  - Timere FreeRTOS – pentru heartbeat la 1 Hz.  

- **Protocol de comunicație**  
  Bluetooth SPP prin modulul HC-05.  

- **Stivă de nivel înalt implementată pe microcontroller**  
  Datele sunt validate, parsate și formatate în JSON pe ESP32 înainte de a fi trimise prin Bluetooth.  

- **Documentație proiect**  
  Repo include:  
  - Specificațiile aplicației  
  - Schema bloc și schema electrică  
  - Cod sursă complet (C, ESP-IDF)  
  - Analiză de timp real (latențe, timpi de reacție)  

---

## Specificații Produs

### Hardware
- Platformă: ESP32 DevKitC v4 (dual-core, Wi-Fi + Bluetooth integrat)  
- Modul GPS: NEO-6M (UART2, 9600 bps)  
- Interfață Bluetooth: UART1 conectat la modul HC-05 (SPP)  
- Indicator LED: GPIO5 pentru semnalizare vizuală (activitate GPS/SPP)  
- Alimentare: 5V prin USB sau baterie externă  

### Software
- Framework: ESP-IDF v5.3.1  
- RTOS: FreeRTOS integrat  
- Limbaj: C  

### Arhitectură software
- **Task GPS (prioritate 5):**  
  - Citește propoziții NMEA de la modulul GPS (UART2)  
  - Validează și parsează RMC/GGA  
  - Formatează datele în JSON  
  - Trimite mesajul într-o coadă și semnalizează prin semafor  

- **Task SPP (prioritate 4):**  
  - Preia mesaje din coadă  
  - Trimite datele prin Bluetooth (UART1)  
  - Trimite heartbeat periodic  

- **Sincronizare între taskuri:**  
  - Cozi FreeRTOS pentru transmiterea datelor GPS  
  - Semafor counting pentru semnalizarea mesajelor noi  
  - Mutex pentru protejarea accesului la UART1  

### Date transmise
Exemplu de mesaj JSON trimis prin Bluetooth:
```json
{
  "time": "123519",
  "date": "230394",
  "valid": 1,
  "lat": 48.1173,
  "lon": 11.5167,
  "alt": 545.4,
  "sats": 8,
  "speed": 13.2
}

## Analiza de timp real

- GPS furnizează date la **1 Hz**  

### Timp de răspuns tipic (GPS → coadă → SPP → UART)
- Parsare + formatare JSON în `gps_task`: câteva sute de microsecunde  
- Semnalizare cu semafor + coadă: 10–50 microsecunde  
- Preluare în `spp_task`: imediată (prioritate 4)  
- Scriere pe UART1 la 9600 bps: ~100 ms pentru 100 bytes  

**Latență logică (până la `uart_write_bytes`)**: sub 5–10 ms  
**Latență percepută de receptor**: ~100 ms, dominată de viteza UART  

### Timp maxim de răspuns – factori limitativi
- `vTaskDelay(10 ms)` în `gps_task` (propoziții valide)  
- `vTaskDelay(200 ms)` când propoziția e invalidă (jitter posibil 200 ms)  
- Baudrate mic (9600 bps)  
- Tick rate (ex. 100 Hz → rezoluție 10 ms)  

### Concluzie
- CPU liber aproximativ 99%  
- Taskurile critice (GPS și SPP) sub 3% CPU  
- Timp de răspuns logic sub 50 ms, încadrat în cerințele de timp real  
- Limitările provin din delay-urile introduse manual și baudrate-ul mic  
