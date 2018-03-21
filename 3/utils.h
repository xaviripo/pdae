#ifndef UTILS_H
#define UTILS_H

#include<stdint.h>

// DEFINES

#define LINEA_PAE 1
#define LINEA_ESTADO 2
#define LINEA_RETARD 3
#define LINEA_HORA 5
#define LINEA_HORA_SELEC 6
#define LINEA_ALARMA 7
#define LINEA_ALARMA_SELEC 8
#define LINEA_ALARMA_MSG 9


#define S1 1
#define S2 2
#define LEFT 3
#define RIGHT 4
#define UP 5
#define DOWN 6
#define CENTER 7

// typedefs

typedef uint8_t bool;
typedef uint8_t Estado;

// FUNCIONES PARA MODIFICAR EL TIEMPO

/**
 * Incrementa els segons d'un punter (HHMM'SS')
 */
void inc_seconds(uint32_t *time) {
    uint32_t secs = *time%60;
    *time -= secs;
    secs++;
    secs %= 60;
    *time += secs;
}

/**
 * Incrementa els minuts d'un punter (HH'MM'SS)
 */
void inc_minutes(uint32_t *time) {
    uint32_t mins = (*time/60)%60;
    *time -= mins*60;
    mins++;
    mins %= 60;
    *time += mins*60;
}

/**
 * Incrementa les hores d'un punter ('HH'MMSS)
 */
void inc_hours(uint32_t *time) {
    uint32_t hrs = (*time/3600)%24;
    *time -= hrs*3600;
    hrs++;
    hrs %= 24;
    *time += hrs*3600;
}

/**
 * Decrementea els segons d'un punter (HHMM'SS')
 */
void dec_seconds(uint32_t *time) {
    uint32_t secs = *time%60;
    *time -= secs;
    secs = secs == 0 ? 59 : secs-1;
    secs %= 60;
    *time += secs;
}

/**
 * Decrementa els minuts d'un punter (HH'MM'SS)
 */
void dec_minutes(uint32_t *time) {
    uint32_t mins = (*time/60)%60;
    *time -= mins*60;
    mins = mins == 0 ? 59 : mins-1;
    mins %= 60;
    *time += mins*60;
}

/**
 * Decrementa les hores d'un punter ('HH'MMSS)
 */
void dec_hours(uint32_t *time) {
    uint32_t hrs = (*time/3600)%24;
    *time -= hrs*3600;
    hrs = hrs == 0 ? 23 : hrs-1;
    hrs %= 24;
    *time += hrs*3600;
}

/**
 * Incrementa els minuts d'un punter (HH'MM')
 */
void inc_minutes_alarm(uint16_t *time) {
    uint16_t mins = *time%60;
    *time -= mins;
    mins++;
    mins %= 60;
    *time += mins;
}

/**
 * Incrementa les hores d'un punter (HH'MM')
 */
void inc_hours_alarm(uint16_t *time) {
    uint32_t hrs = (*time/60)%60;
    *time -= hrs*60;
    hrs++;
    hrs %= 60;
    *time += hrs*60;
}

/**
 * Incrementa els minuts d'un punter (HH'MM')
 */
void dec_minutes_alarm(uint16_t *time) {
    uint16_t mins = *time%60;
    *time -= mins;
    mins++;
    mins %= 60;
    *time += mins;
}

/**
 * Incrementa les hores d'un punter (HH'MM')
 */
void dec_hours_alarm(uint16_t *time) {
    uint32_t hrs = (*time/60)%60;
    *time -= hrs*60;
    hrs++;
    hrs %= 60;
    *time += hrs*60;
}

#endif /* UTILS_H */