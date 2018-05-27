#ifndef SOUND_H_
#define SOUND_H_

#include "../common.h"

typedef struct Note {
	uint8_t pitch;
	uint8_t duration; //??
} Note;

#define A 0
#define B 2
#define C 3
#define D 5
#define E 7
#define F 8
#define G 10
#define SHARP 1
#define SCALE 12
/*
 fa# (1) = 0*SCALE + FA + SHARP
 re  (2) = 1*SCALE + RE
 */

void play_magic_melody(uint8_t times);

void play_note(Note note);
void stop_sound();

#endif /* SOUND_H_ */
