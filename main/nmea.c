#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>    
#include "nmea.h"

bool nmea_validate(const char *sentence)
{
    // Validare checksum NMEA
    if (sentence[0] != '$')
        return false;

    const char *chk_ptr = strchr(sentence, '*');
    if (!chk_ptr || strlen(chk_ptr) < 3)
        return false;

    unsigned char checksum = 0;
    for (const char *p = sentence + 1; p < chk_ptr; p++)
        checksum ^= (unsigned char)*p;

    unsigned int given_chk;
    sscanf(chk_ptr + 1, "%2X", &given_chk);

    return (checksum == given_chk);
}

bool nmea_parse_sentence(const char *sentence, nmea_fix_t *fix)
{
    if (!sentence || !fix)
        return false;

    memset(fix, 0, sizeof(nmea_fix_t)); // initializeaza structura

    // Parsare in functie de tip
    if (strstr(sentence, "GPRMC") || strstr(sentence, "GNRMC"))
    {
        char copy[128];
        strncpy(copy, sentence, sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = 0;

        strtok(copy, ","); // $GPRMC
        char *time = strtok(NULL, ",");
        char *status = strtok(NULL, ",");
        char *lat = strtok(NULL, ",");
        char *lat_dir = strtok(NULL, ",");
        char *lon = strtok(NULL, ",");
        char *lon_dir = strtok(NULL, ",");
        char *speed = strtok(NULL, ",");
        char *crs = strtok(NULL, ",");   
        char *date = strtok(NULL, ",");

        if (time)
            strncpy(fix->time_utc, time, sizeof(fix->time_utc) - 1);
        if (date)
            strncpy(fix->date_utc, date, sizeof(fix->date_utc) - 1);
        if (status && *status == 'A')
            fix->valid = true;

        if (lat && *lat)
        {
            double deg = floor(atof(lat) / 100.0);
            double min = atof(lat) - deg * 100.0;
            fix->lat_deg = deg + min / 60.0;
            if (lat_dir && *lat_dir == 'S')
                fix->lat_deg = -fix->lat_deg;
        }

        if (lon && *lon)
        {
            double deg = floor(atof(lon) / 100.0);
            double min = atof(lon) - deg * 100.0;
            fix->lon_deg = deg + min / 60.0;
            if (lon_dir && *lon_dir == 'W')
                fix->lon_deg = -fix->lon_deg;
        }

        if (speed)
            fix->speed_kn = atof(speed);

        return true;
    }
    else if (strstr(sentence, "GPGGA") || strstr(sentence, "GNGGA"))
    {
        char copy[128];
        strncpy(copy, sentence, sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = 0;

        strtok(copy, ","); // $GPGGA
        char *time = strtok(NULL, ",");
        char *lat = strtok(NULL, ",");
        char *lat_dir = strtok(NULL, ",");
        char *lon = strtok(NULL, ",");
        char *lon_dir = strtok(NULL, ",");
        char *fix_q = strtok(NULL, ",");
        char *sats = strtok(NULL, ",");
        char *hdop = strtok(NULL, ","); 
        char *alt = strtok(NULL, ",");
        char *um = strtok(NULL, ",");   

        if (time)
            strncpy(fix->time_utc, time, sizeof(fix->time_utc) - 1);
        if (fix_q && atoi(fix_q) > 0)
            fix->valid = true;
        if (sats)
            fix->sats = atoi(sats);
        if (alt)
            fix->alt_m = atof(alt);

        if (lat && *lat)
        {
            double deg = floor(atof(lat) / 100.0);
            double min = atof(lat) - deg * 100.0;
            fix->lat_deg = deg + min / 60.0;
            if (lat_dir && *lat_dir == 'S')
                fix->lat_deg = -fix->lat_deg;
        }

        if (lon && *lon)
        {
            double deg = floor(atof(lon) / 100.0);
            double min = atof(lon) - deg * 100.0;
            fix->lon_deg = deg + min / 60.0;
            if (lon_dir && *lon_dir == 'W')
                fix->lon_deg = -fix->lon_deg;
        }

        return true;
    }

    return false;
}
