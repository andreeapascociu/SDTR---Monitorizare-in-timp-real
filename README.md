# Dispozitiv portabil pentru monitorizarea în timp real a animalelor de companie

Acest proiect implementează un sistem embedded bazat pe **ESP32 DevKitC v4** care monitorizează poziția unui animal de companie în timp real, folosind un **modul GPS NEO-6M** pentru achiziția datelor de localizare și un **modul Bluetooth HC-05** pentru transmiterea acestora către utilizator.  
Software-ul rulează pe **FreeRTOS**, asigurând execuția concurentă a mai multor task-uri și sincronizarea lor prin mecanisme RTOS.

## Cerințe minimale – Îndeplinire

**Utilizare sistem de operare de timp real**  
  Proiectul folosește **FreeRTOS**, integrat în ESP-IDF.

**Cel puțin două procese**  
  - Task GPS – achiziție și procesare date NMEA.  
  - Task Bluetooth – transmitere date prin HC-05.  

**Un proces în timp real cu prelucrare locală**  
  Task-ul GPS rulează în timp real și interacționează cu mediul extern prin modulul GPS și LED-ul indicator.  

**Un proces care implementează un protocol de comunicație la distanță**  
  Task-ul Bluetooth implementează protocolul SPP (Serial Port Profile) prin HC-05.  

**Procese independente, dar sincronizate**  
  - Datele sunt transmise din Task GPS către Task Bluetooth printr-o **coadă FreeRTOS (queue)**.  
  - Sincronizarea se face cu **semafoare și mutex-uri**.  

**Tratarea condițiilor de concurență**  
  - Mutex pentru acces exclusiv la UART.  
  - Semafor pentru semnalizarea mesajelor noi.  

## Cerințe specifice – Îndeplinire

**Microcontroller ales de student**  
  ESP32 DevKitC v4 (dual-core, 240 MHz, suport Wi-Fi și Bluetooth).  

**Interfață de intrare și ieșire**  
  - Intrare: date GPS prin UART.  
  - Ieșire: transmisie Bluetooth HC-05 și LED indicator.  

**Folosirea resurselor integrate**  
  - UART2 – pentru GPS.  
  - UART1 – pentru Bluetooth.  
  - GPIO – pentru LED.  
  - Timere FreeRTOS – pentru heartbeat la 1 Hz.  

**Protocol de comunicație**  
  Bluetooth SPP prin modulul HC-05.  

**Stivă de nivel înalt implementată pe microcontroller**  
  Datele sunt validate, parse și formatate CSV pe ESP32 înainte de a fi trimise prin Bluetooth.  

**Documentație proiect**  
  Repo include:  
  - Specificațiile aplicației  
  - Schema bloc și schema electrică  
  - Cod sursă complet (C, ESP-IDF)  
  - Analiză de timp real (latențe, timpi de reacție)  

## Analiza de timp real

- GPS furnizează date la **1 Hz**.  
- Timpul de la recepția propoziției NMEA până la transmiterea prin Bluetooth este sub **50 ms**.  
- Sistemul este capabil să reacționeze în timp real la evenimente externe (date GPS valide).  
