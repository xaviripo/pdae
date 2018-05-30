#include "sound.h"

#include "../hal/communication.h"
#include "../hal/timers.h"
#include "robot.h"

Note *notes_g;
uint8_t notes_len_g;
uint8_t note_idx;
uint8_t tempo_counter = 0;
bool can_play_g = 0;

Note smb[] = {
              {E_+SCALE,1},
              {E_+SCALE,1},
              {E_+SCALE,2},
              {C_+SCALE,2},
              {E_+SCALE,2},
              {G_+SCALE,3},
              {G_,3}

};

Note zelda[] = {
      {A_,10},
      {A_,2},
      {A_,2},
      {A_,2},
      {A_,2},
      {A_,4},
      {G_-SCALE,2},
      {A_,6},
      {A_,2},
      {A_,2},
      {A_,2},
      {A_,2},
      {A_,4},
      {G_-SCALE,2},
      {A_,6},
      {A_,2},
      {A_,2},
      {A_,2},
      {A_,2},
      {A_,2},
      {E_-SCALE,1},
      {E_-SCALE,1},
      {E_-SCALE,2},
      {E_-SCALE,1},
      {E_-SCALE,1},
      {E_-SCALE,2},
      {E_-SCALE,1},
      {E_-SCALE,1},
      {E_-SCALE,2},
      {E_-SCALE,2}
};

Note escala[] = {
      {C_,10},
      {D_,10},
      {E_,10},
      {F_,10},
      {G_,10},
      {A_+SCALE,10},
      {B_+SCALE,10},
};

Note megalovania[] = {
      {D_,SEMI},
      {D_,SEMI},
      {D_+SCALE,CORC},
      {A_+SCALE,CORC+SEMI},
      {G_+SHARP,CORC},
      {G_,CORC},
      {F_,CORC},
      {D_,SEMI},
      {F_,SEMI},
      {G_,SEMI},

      {C_,SEMI},
      {C_,SEMI},
      {D_+SCALE,CORC},
      {A_+SCALE,CORC+SEMI},
      {G_+SHARP,CORC},
      {G_,CORC},
      {F_,CORC},
      {D_,SEMI},
      {F_,SEMI},
      {G_,SEMI},

      {B_,SEMI},
      {B_,SEMI},
      {D_+SCALE,CORC},
      {A_,CORC+SEMI},
      {G_+SHARP,CORC},
      {G_,CORC},
      {F_,CORC},
      {D_,SEMI},
      {F_,SEMI},
      {G_,SEMI},

      {D_,SEMI},
      {D_,SEMI},
      {D_+SCALE,CORC},
      {A_+SCALE,CORC+SEMI},
      {G_+SHARP,CORC},
      {G_,CORC},
      {F_,CORC},
      {D_,SEMI},
      {F_,SEMI},
      {G_,SEMI},

      {C_,SEMI},
      {C_,SEMI},
      {D_+SCALE,CORC},
      {A_+SCALE,CORC+SEMI},
      {G_+SHARP,CORC},
      {G_,CORC},
      {F_,CORC},
      {D_,SEMI},
      {F_,SEMI},
      {G_,SEMI},

      {B_,SEMI},
      {B_,SEMI},
      {D_+SCALE,CORC},
      {A_,CORC+SEMI},
      {G_+SHARP,CORC},
      {G_,CORC},
      {F_,CORC},
      {D_,SEMI},
      {F_,SEMI},
      {G_,SEMI},
};

Note wii[] = {
              {F_+SHARP,NEGR},
              {A_+SCALE,CORC},
              {C_+SHARP+SCALE,NEGR},
              {A_+SCALE,NEGR},
              {F_+SHARP,CORC},

              {D_,CORC},
              {D_,CORC},
              {D_,BLAN},
};

/**
 * Sets the "tempo" of the sounds to last duration ms
 * @param duration - duration in ms
 */
void set_tempo(time_t duration) {
	//uint32_t dividendo = 4294967296; // 24*2^16/8
    //TA1CCR = dividendo/divisor;
	// todo, resetear clock
}

void set_music(uint8_t length, Note* notes) {
	// todo
	notes_g = notes;
	note_idx = 0;
	notes_len_g = length;
	set_sec_timer_interrupt(1);
	reset_sec_time();
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
    rx_status();
}

bool play_note(Note note) {
	uint8_t module_id = 100;
	uint8_t parameter_length = 3;
	uint8_t parameters[] = {
		0x28,
		note.pitch, // play x times
		note.duration
	};
    tx_instruction(module_id, parameter_length, WRITE_DATA, parameters);
    rx_status();
    return (get_status().checksum_correct && !get_status().timeout);
}

bool cut_note() {
    uint8_t module_id = 100;
    uint8_t parameter_length = 2;
    uint8_t parameters[] = {
        0x29,
        0
    };
    tx_instruction(module_id, parameter_length, WRITE_DATA, parameters);
    rx_status();
    return (get_status().checksum_correct && !get_status().timeout);
}

void play_next_note() {
    if(can_play_g) {
        can_play_g = !(cut_note() && play_note(notes_g[note_idx]));
    }
}

/**
 * NO funciona
 **/
void stop_sound() {
    // todo
    note_idx = 0;
    set_sec_timer_interrupt(0);
}



void play_smb() {
	set_music(7,smb);
}

void play_escala() {
    set_music(7,escala);
}

void play_tloz() {
    set_music(29,zelda);
}

void play_megalo() {
    set_music(60,megalovania);
}

void play_wii() {
    set_music(8, wii);
}

void on_music_tick() {
	if (has_passed_sec(notes_g[note_idx].duration)) {
	    can_play_g = 1;

		note_idx = (note_idx+1)%notes_len_g;
		reset_sec_time();
	}
}

void Sound__enable(bool flag) {
    set_sec_timer_interrupt(flag);
}
