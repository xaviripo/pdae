#ifndef SOUND_H_
#define SOUND_H_

#include "../common.h"

typedef struct Note {
	uint8_t pitch;
	uint8_t duration; //??
} Note;

// las notas empiezan en la escala 3
#define A_ 24
#define B_ 26
#define C_ 27
#define D_ 29
#define E_ 31
#define F_ 32
#define G_ 34
#define SHARP 1
#define SCALE 12
/*
 fa# (3) = 0*SCALE + FA + SHARP
 re  (2) = RE - 1*SCALE
 */

#define SEMI 1
#define CORC 2
#define NEGR 4
#define BLAN 8

void play_magic_melody(uint8_t times);

bool play_note(Note note);
void stop_sound();
void play_smb();
void play_tloz();
void play_scale();
void play_megalo();
void play_wii();
void play_portal();

void play_next_note();

void Sound__enable(bool flag);

#endif /* SOUND_H_ */
