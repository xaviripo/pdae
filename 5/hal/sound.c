#include "sound.h"

#include "communication.h"
#include "timers.h"

Note *notes_g;
uint8_t notes_len_g;
uint8_t note_idx;
uint8_t tempo_counter = 0;

Note smb[] = {{E,0.3},{E,0.3},{E,0.3},{C,0.3},{E,0.3},{G,0.3},{G,1},{C,0.3},{G,1},{E,1},{A,1},{B,1},{A + SHARP,1},{A,1},{G,1},{E,0.3},{G,0.3},{A,0.3},{F,0.3},{G,0.3},{E,0.3},{C,0.3},{D,0.3},{B,1}}

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
	void set_sec_timer_interrupt(true);
	void reset_sec_time(void);
	notes_g = notes;
	note_idx = 0;
	notes_len_g = length;
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

/**
 * NO funciona
 **/
void stop_sound() {
	uint8_t module_id = 100;
	uint8_t parameter_length = 2;
	uint8_t parameters[] = {
		0x29,
		0, // stop sound
	};

	tx_instruction(module_id, parameter_length, WRITE_DATA, parameters);
}

void play_smb() {
	set_music(23,smb);
}

void on_music_tick() {
	if (has_passed_sec(notes_g[note_idx].duration * 1000)) {
		reset_sec_time(void);
		note_idx = (note_idx+1)%notes_len_g
		play_note(notes_g[note_idx]);
	}
}
