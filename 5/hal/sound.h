#ifndef SOUND_H_
#define SOUND_H_

#include "../common.h"

typedef struct Note {
	uint8_t pitch;
	uint8_t duration; //??
} Note;

#define A_ 0
#define B_ 2
#define C_ 3
#define D_ 5
#define E_ 7
#define F_ 8
#define G_ 10
#define SHARP 1
#define SCALE 12
/*
 fa# (1) = 0*SCALE + FA + SHARP
 re  (2) = 1*SCALE + RE
 */

void play_magic_melody(uint8_t times);

bool play_note(Note note);
void stop_sound();
void play_smb();
void play_tloz();
void play_scale();
void play_next_note();

#endif /* SOUND_H_ */
