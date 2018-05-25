#include "sound.h"

#include "communication.h"
#include "timers.h"

Note *notes_g;
uint8_t note_idx;
uint8_t tempo_counter = 0;

/**
 * Sets the "tempo" of the sounds to last duration ms
 * @param duration - duration in ms
 */
void set_tempo(time_t duration) {
	// todo check this
	time_t divisor = 1000/duration;
	//uint32_t dividendo = 4294967296; // 24*2^16/8
    //TA1CCR = dividendo/divisor;
	// todo, resetear clock
}

void set_music(uint8_t length, Note* notes) {
	// todo
	notes_g = notes;
	note_idx = 0;
}

void play_magic_melody(uint8_t times) {
	uint8_t module_id = 100;
	uint8_t parameter_length = 3;
	uint8_t parameters[] = {
		0x28,
		times, // play x times
		0xFF, // play "special melody"
	};

	tx_instruction(module_id, parameter_length, WRITE_DATA, parameters);
}

void play_note(Note note) {
	uint8_t module_id = 100;
	uint8_t parameter_length = 3;
	uint8_t parameters[] = {
		0x28,
		note.pitch, // play x times
		note.duration
	};

	tx_instruction(module_id, parameter_length, WRITE_DATA, parameters);
}

void stop_sound() {
	uint8_t module_id = 100;
	uint8_t parameter_length = 2;
	uint8_t parameters[] = {
		0x29,
		0, // stop sound
	};

	tx_instruction(module_id, parameter_length, WRITE_DATA, parameters);
}

void on_music_tick() {
	if (tempo_counter >= notes_g[note_idx].duration) {
		tempo_counter = 0;
		note_idx++;

		stop_sound();
		play_note(notes_g[note_idx]);
	} else {
		tempo_counter++;
	}
}
