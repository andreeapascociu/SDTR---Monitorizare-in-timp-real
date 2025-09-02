// nmea.h
#pragma once
#include <stdbool.h>

typedef struct {
    bool   valid;        // true daca RMC/GLL au 'A'
    double lat_deg;      // grade zecimale
    double lon_deg;      // grade zecimale
    double speed_kn;     // din RMC (noduri)
    int    sats;         // din GGA
    double alt_m;        // din GGA
    char   time_utc[16]; // hhmmss.ss
    char   date_utc[8];  // ddmmyy (din RMC)
} nmea_fix_t;

bool nmea_validate(const char *sentence);
bool nmea_parse_sentence(const char *sentence, nmea_fix_t *out); // returneaza true daca a extras ceva util
