#ifndef SOUND_H_
#define SOUND_H_

#include "../common.h"

typedef struct Note {
	uint8_t pitch;
	uint8_t duration; //??
} Note;

#define LA 0
#define SI 2
#define DO 3
#define RE 5
#define MI 7
#define FA 8
#define SOL 10
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
