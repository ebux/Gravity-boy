//-------------------------------------------------------------------------
//
// Gravity boy for the Z80 based HomeLab 3/4 microcomputers
//
// Coded by: Gergely Eberhardt, 2024
// Many thanks to Zoltan Varga for his invaluable help
//
//-------------------------------------------------------------------------
// resources
#include "tiles.h"
#include "astronaut.h"
#include "tables.h"
#include "start_screen.h"
#include "enemy.h"
#include "rock.h"
#include "level_cleared.h"
#include "finish.h"

//-----------------------------------
// maps
#include "maps/gravity1.h"
#include "maps/gravity2.h"
#include "maps/gravity3.h"
#include "maps/gravity4.h"
#include "maps/gravity5.h"
#include "maps/gravity6.h"
#include "maps/gravity7.h"
#include "maps/gravity8.h"
#include "maps/gravity9.h"
#include "maps/gravity10.h"
#include "maps/gravity11.h"
#include "maps/gravity12.h"
#include "maps/gravity13.h"
#include "maps/gravity14.h"

unsigned char current_level = 0;
unsigned char *levels[] = {
	gravity1, gravity2, gravity3, gravity4, gravity5, gravity6, gravity7, gravity8, 
	gravity9, gravity10, gravity11, gravity12, gravity13, gravity14, 0
};

//-----------------------------------
unsigned char* screen = (unsigned char*)0xF800;	 

unsigned char* arrow_keys = (unsigned char*)0xE800;	
unsigned char* keysspace = (unsigned char*)0xE801;	
unsigned char* fghi_keys =  (unsigned char*)0xE80A;	
unsigned char* pqrs_keys =  (unsigned char*)0xE80D;	

#define KEY_SPACE 	1
#define KEY_UP 		2
#define KEY_DOWN 	1
#define KEY_RIGHT 	4
#define KEY_LEFT 	8
#define KEY_H		4
#define KEY_R		4

#define MOVING_NONE 	0xff
#define MOVING_DOWN 	0
#define MOVING_UP 		2
#define MOVING_LEFT		3
#define MOVING_RIGHT	1

#define LAST_LINE_START 29*64
#define LAST_COL_START  62

// max speed of the player
#define MAX_SPEED 200

unsigned char tick = 0;
unsigned char tick_frame = 0;

//--------------------------------------
// gravity
unsigned char moving_line = MOVING_NONE;
unsigned char next_move   = MOVING_NONE;
unsigned char current_direction = MOVING_DOWN;
int line_offset = 0;
unsigned int line_start = 0xf800;
unsigned char gravity_change_in_progress = 0;

//--------------------------------------
// player
unsigned int player_pos_x = 16<<8;			// positions in pixel
unsigned int player_pos_y = 30<<8;
int player_speed_x = 0;
int player_speed_y = 0;
int gravity = 10;

//--------------------------------------
// states
#define STATE_STARTING		 0
#define STATE_PLAYER_RESTART 1
#define STATE_NORMAL		 2
#define STATE_END			 3
#define STATE_GRAVITY_CHANGE 4

unsigned char game_state = STATE_PLAYER_RESTART;

//--------------------------------------
// game mode
#define GAME_MODE_EASY	0
#define GAME_MODE_HARD	1

unsigned char game_mode = GAME_MODE_EASY;

//--------------------------------------
// map data
#define T_NONE	 		0
#define T_GROUND 		1
#define T_DANGER 		2
#define T_END    		3
#define T_START  		4
#define T_ENEMY_UP 		5
#define T_ENEMY_LEFT 	6
#define T_ENEMY_ROCK 	7

unsigned char tile_types[] = {
	T_NONE, T_GROUND,							// 0
	T_GROUND, T_GROUND, T_GROUND, T_GROUND,		// 2
	T_DANGER, T_DANGER, T_DANGER, T_DANGER,		// 6
	T_GROUND, T_GROUND, T_GROUND, T_GROUND,		// 10
	T_GROUND, T_GROUND, T_GROUND, T_GROUND,		// 14
	T_GROUND, T_START, T_ENEMY_UP, T_ENEMY_LEFT, //18
	T_ENEMY_ROCK,								// 22
	T_END, T_END, T_END, T_END, T_END, T_END, T_END, T_END,
	T_NONE, T_NONE, T_NONE, T_NONE
};

unsigned char *current_map = 0;			// pointer to the current map data
unsigned char start_pos_x = 0;			// player start x position (T_START tile x position)
unsigned char start_pos_y = 0;			// player start y position (T_START tile y position)
unsigned char *end_tile_addr1 = 0;		// end tile addresses (first two T_END tiles)
unsigned char *end_tile_addr2 = 0;
unsigned char *end_tile_id1 = 19;		// which end tile was used
unsigned char *end_tile_id2 = 20;
unsigned char end_tile_out = 0;			// end tile is currently on the screen

//--------------------------------------
// enemies

#define MAX_ENEMY_COUNT	4
#define MAX_ENEMY_FRAME 3

#define ENEMY_DIR_UP 	0
#define ENEMY_DIR_DOWN 	1
#define ENEMY_DIR_LEFT 	2
#define ENEMY_DIR_RIGHT	3
#define ENEMY_DIR_ROCK	4

unsigned char enemy_count = 0;
unsigned char enemy_dirs[MAX_ENEMY_COUNT] = {ENEMY_DIR_UP, ENEMY_DIR_UP, ENEMY_DIR_UP, ENEMY_DIR_UP};
unsigned char enemy_dir_orig[MAX_ENEMY_COUNT] = {ENEMY_DIR_UP, ENEMY_DIR_UP, ENEMY_DIR_UP, ENEMY_DIR_UP};
unsigned char enemy_frame[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_posx[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_posy[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_orig_posx[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_orig_posy[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_ysize[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_ysize_del[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_xsize[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_xsize_del[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char *enemy_delete_pos[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char *enemy_pos[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char *enemy_data[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_new_posx[MAX_ENEMY_COUNT] = {0,0,0,0};
unsigned char enemy_new_posy[MAX_ENEMY_COUNT] = {0,0,0,0};

unsigned char enemy_new_dir[] = {ENEMY_DIR_DOWN, ENEMY_DIR_UP, ENEMY_DIR_RIGHT, ENEMY_DIR_LEFT};

//--------------------------------------
// Additional text in the start screen
unsigned char press_space_line[] = {
		192, 192, 192, 192, 80, 114, 101, 115, 115, 96, 83, 80, 65, 67, 69, 96, 116, 111, 96, 115, 101, 108, 101, 99, 116, 96, 69, 
		97, 115, 121, 96, 77, 111, 100, 101, 96, 111, 114, 96, 72, 96, 116, 111, 96, 99, 104, 111, 111, 115, 101, 96, 72, 97, 114, 
		100, 96, 77, 111, 100, 101, 33, 192, 192, 192
		};

//--------------------------------------
// print a number to the screen
void printnum(int x,int y,int number,int hely) {
	x=hely+x+y*64;
	if (number==0) { screen[x]=48;return; }
    for (;hely>0;hely--) {
		if (number > 0) {
			screen[x]=(number % 10)+48;
			number /= 10;
		} else {
			screen[x]=0;
		}
        x--;
    }	
}

void waitforspace() {
	while ((keysspace[0] & KEY_SPACE)!=0) {  } // space	
}

//-------------------------------------
unsigned char sound_length = 0;
unsigned char sound_freq = 0;

void play_sound() {
#asm
	push HL
	push DE
	push AF
	push BC
	
	ld A,(_sound_length)
	ld B,A // E a hang hossza

	ld A,(_sound_freq)
	ld D,A // D a 0. frekvencia szerinti hang
	or A 
	jp z,hangszunet
	
hangdur:	
	 ld E,b

	 ld b,D	 
hang01:
	 ld a,($e880)
	 ld a,($e880)
	 ld a,($e880)
	 ld a,($e880)
	 djnz hang01
	 
	 ld b,D
hang02:	
	 ld a,($e800)
	 ld a,($e800)
	 ld a,($e800)
	 ld a,($e800)
	 djnz hang02
	 
	 ld b,E	 
	 djnz hangdur
	 jp hangvege

hangszunet:
	 ld E,b
	 ld b,D	 
hangszunet01:
	 ld a,($7f00)
	 djnz hangszunet01
	 
	 ld b,D
hangszunet02:	
	 ld a,($7f00)
	 djnz hangszunet02
	 
	 ld b,E	 
	 djnz hangszunet

hangvege:
	pop BC
	pop AF
	pop DE
	pop HL
#endasm	
}

#define note_A2 210
#define note_Ais2 197
#define note_H2 187
#define note_C3 176
#define note_Cis3 166
#define note_D3 157
#define note_Dis3 148
#define note_E3 140
#define note_F3 132
#define note_Fis3 125
#define note_G3 118
#define note_Gis3 111
#define note_A3 105
#define note_H3 93
#define note_Ais3 99
#define note_C4 88
#define note_Cis4 83
#define note_D4 78
#define note_Dis4 74 
#define note_E4 70
#define note_F4 66
#define note_Fis4 62
#define note_G4 59
#define note_Gis4 55
#define note_A4 52
#define note_Ais4 49
#define note_H4 47
#define note_C5 44

void sound_cleared() {
	sound_freq = note_C4;
	sound_length = (50*50)/(note_C4);
	play_sound();
	sound_freq = note_E4;
	sound_length = (50*50)/(note_E4);
	play_sound();
	sound_freq = note_G4;
	sound_length = (50*50)/(note_G4);
	play_sound();
	sound_freq = note_C5;
	sound_length = (50*100)/(note_C5);
	play_sound();
	sound_freq = 0;
	sound_length = 20;
	play_sound();
	sound_freq = note_G4;
	sound_length = (50*50)/(note_G4);
	play_sound();
	sound_freq = note_C5;
	sound_length = (50*100)/(note_C5);
	play_sound();
}

void sound_restart() {
	sound_freq = note_G4;
	sound_length = (50*100)/(note_G4);
	play_sound();
	sound_freq = note_F4;
	sound_length = (50*100)/(note_F4);
	play_sound();
	sound_freq = note_D4;
	sound_length = (50*100)/(note_D4);
	play_sound();
	sound_freq = note_C4;
	sound_length = (50*100)/(note_C4);
	play_sound();
	sound_freq = 0;
	sound_length = 20;
	play_sound();
	sound_freq = note_G3;
	sound_length = (50*200)/(note_G3);
	play_sound();
}

void sound_start() {
	sound_freq = note_C4;
	sound_length = (50*100)/(note_C4);
	play_sound();
	sound_freq = note_E4;
	sound_length = (50*100)/(note_E4);
	play_sound();
	sound_freq = note_G4;
	sound_length = (50*100)/(note_G4);
	play_sound();
	sound_freq = 0;
	sound_length = 40;
	play_sound();
	sound_freq = note_G4;
	sound_length = (50*100)/(note_F4);
	play_sound();
	sound_freq = note_G4;
	sound_length = (50*100)/(note_A4);
	play_sound();
	sound_freq = 0;
	sound_length = 40;
	play_sound();
	sound_freq = note_C5;
	sound_length = (50*100)/(note_C5);
	play_sound();
	sound_freq = note_G4;
	sound_length = (50*100)/(note_G4);
	play_sound();
	sound_freq = note_E4;
	sound_length = (50*100)/(note_E4);
	play_sound();
	sound_freq = note_C4;
	sound_length = (50*150)/(note_C4);
	play_sound();
}

//--------------------------------------
// Check the tiles where the spirte is over. Returns the first type, which requires action
// T_GROUND: stop movement
// T_END: level is cleared
// T_DANGER: restart the level

unsigned char check_sprite_new_position(unsigned char pos_x, unsigned char pos_y, unsigned char sizex, unsigned char sizey) {	
	unsigned char xchar = pos_x>>1;					// calculate char x,y position from pixel positions
	unsigned char ychar = table_div3[pos_y];
	unsigned char ychar_end = table_div3[pos_y-1];

	unsigned char start_tile_x = xchar>>2;			// calculate start and end tile positions from char positions
	unsigned char end_tile_x = (xchar+sizex-1) >> 2;
	unsigned char start_tile_y = table_div3[ychar];
	unsigned char end_tile_y = table_div3[ychar_end+sizey-1];

	unsigned char type = T_NONE;					// check every tiles between start and end tile positions
	for(unsigned char y = start_tile_y; y <= end_tile_y; y++){
		for(unsigned char x = start_tile_x; x <= end_tile_x; x++){
			unsigned char tile_pos = (y<<4)+x;
			unsigned char tile = current_map[tile_pos];
			type = tile_types[tile];
			if (type == T_GROUND || type == T_END || type == T_DANGER) {
				return type;
			}
		}
	}
	return T_NONE;
}

//--------------------------------------
// Clear 1 line in the screen

void clear_line(unsigned char dummy) {
	#asm
		ld hl,(_line_start)
		ld b,64
		ld a,0xc0
	loop_line_clear:
		ld (hl),a
		inc hl
		djnz loop_line_clear
	#endasm;
}

//--------------------------------------
// Invert a line for gravity change
void invert_line() {
	#asm
		ld hl,(_line_start)
		ld b,64
		ld d,63
	loop_line:
		ld a,d
		xor (hl)		
		ld (hl),a
		inc hl
		djnz loop_line
	#endasm;
}

// Invert a column for gravity change
void invert_column() {
	#asm
		ld hl,(_line_start)
		ld b,30
		ld de,64
		ld c,63
	loop_col:
		ld a,c
		xor (hl)		
		ld (hl),a
		add hl,de
		djnz loop_col
	#endasm;
}

// initiate next gravity change or close a running one
void init_next_move() {
	gravity_change_in_progress = 0;				// gravity change is finished
	if (current_direction == next_move) {		// do not change gravity for the same direction as the current one
		next_move = MOVING_NONE;
		return;
	}

	// check rotation of the player, change gravity only if the player fit in the place
	unsigned char fixed = 1;
	if (next_move != MOVING_NONE)
	{
		unsigned char type = check_sprite_new_position((player_pos_x>>8), (player_pos_y>>8), astronaut_sizex[next_move], astronaut_sizey[next_move]);
		if (type == T_GROUND) {
			fixed = 0;
			if (current_direction == MOVING_UP || current_direction == MOVING_DOWN) {
				if (next_move == MOVING_LEFT) {
					type = check_sprite_new_position((player_pos_x>>8)-4, (player_pos_y>>8), astronaut_sizex[next_move], astronaut_sizey[next_move]);
					if (type != T_GROUND) {
						player_pos_x = player_pos_x-(4*256);
						fixed = 1;
					}
				}
			}
			else if (current_direction == MOVING_LEFT || current_direction == MOVING_RIGHT) {
				if (next_move == MOVING_UP) {
					type = check_sprite_new_position((player_pos_x>>8), (player_pos_y>>8)-3, astronaut_sizex[next_move], astronaut_sizey[next_move]);
					if (type != T_GROUND) {
						player_pos_y = player_pos_y-(3*256);
						fixed = 1;
					}
				}
			}
		}
	}
	if (fixed == 0) {				// player rotation is not possible, don't change gravity
		next_move = MOVING_NONE;
	}

	// initialize gravity change lines based on the direction
	game_state = STATE_GRAVITY_CHANGE;
	moving_line = next_move;
	next_move = MOVING_NONE;
	switch(moving_line) {
		case MOVING_DOWN:
			line_offset = 0;
			break;
		case MOVING_UP:
			line_offset = LAST_LINE_START;
			break;
		case MOVING_LEFT:
			line_offset = LAST_COL_START;
			break;
		case MOVING_RIGHT:
			line_offset = 0;
			break;
		case MOVING_NONE:
			game_state = STATE_NORMAL;
	}

	// modify enemy direction based on the gravity change
	if (game_state != STATE_NORMAL) {
		for(unsigned char i=0; i < enemy_count; i++) {
			unsigned char dir_orig = enemy_dir_orig[i];
			if (dir_orig == ENEMY_DIR_UP) {
				if (moving_line == ENEMY_DIR_DOWN || moving_line == ENEMY_DIR_UP) {
					if (enemy_dirs[i] == ENEMY_DIR_LEFT) {
						enemy_dirs[i] = ENEMY_DIR_DOWN;
					}
					if (enemy_dirs[i] == ENEMY_DIR_RIGHT) {
						enemy_dirs[i] = ENEMY_DIR_UP;
					}
				}
				if (moving_line == ENEMY_DIR_LEFT || moving_line == ENEMY_DIR_RIGHT) {
					if (enemy_dirs[i] == ENEMY_DIR_DOWN) {
						enemy_dirs[i] = ENEMY_DIR_LEFT;
					}
					if (enemy_dirs[i] == ENEMY_DIR_UP) {
						enemy_dirs[i] = ENEMY_DIR_RIGHT;
					}
				}
			}
			else {
				if (moving_line == ENEMY_DIR_DOWN || moving_line == ENEMY_DIR_UP) {
					if (enemy_dirs[i] == ENEMY_DIR_DOWN) {
						enemy_dirs[i] = ENEMY_DIR_LEFT;
					}
					if (enemy_dirs[i] == ENEMY_DIR_UP) {
						enemy_dirs[i] = ENEMY_DIR_RIGHT;
					}
				}
				if (moving_line == ENEMY_DIR_LEFT || moving_line == ENEMY_DIR_RIGHT) {
					if (enemy_dirs[i] == ENEMY_DIR_LEFT) {
						enemy_dirs[i] = ENEMY_DIR_DOWN;
					}
					if (enemy_dirs[i] == ENEMY_DIR_RIGHT) {
						enemy_dirs[i] = ENEMY_DIR_UP;
					}
				}
			}
		}
	}
}

// invert lines moving down
void process_line_down() {
	gravity_change_in_progress = 1;
	if (line_offset == 0) {
		current_direction = MOVING_DOWN;
		line_start = 0xf800;
		invert_line();
		line_start += 64;
		invert_line();
		line_offset += 64*2;
	}
	else if (line_offset == LAST_LINE_START) {
		line_start = 0xf800-128+line_offset;
		invert_line();
		line_start += 64;
		invert_line();
		init_next_move();
	}
	else {
		line_start = 0xf800-128+line_offset;
		invert_line();
		line_start += 128;
		invert_line();
		line_offset += 64;
	}
}

// invert lines moving up
void process_line_up() {
	gravity_change_in_progress = 1;
	if (line_offset == LAST_LINE_START) {
		current_direction = MOVING_UP;
		line_start = 0xf800+line_offset;
		invert_line();
		line_start -= 64;
		invert_line();
		line_offset -= 64;
	}
	else if (line_offset == 0) {
		line_start = 0xf800+line_offset;
		invert_line();
		line_start += 64;
		invert_line();
		init_next_move();
	}
	else {
		line_start = 0xf800+64+line_offset;		// remove last line
		invert_line();
		line_start -= 128;
		invert_line();
		line_offset -= 64;
	}
}

// invert lines moving right
void process_line_right() {
	gravity_change_in_progress = 1;
	if (line_offset == 0) {
		current_direction = MOVING_RIGHT;
		line_start = 0xf800;
		invert_column();
		line_start += 1;
		invert_column();
		line_offset += 2;
	}
	else if (line_offset == LAST_COL_START) {
		line_start = 0xf800-2+line_offset;
		invert_column();
		line_start += 1;
		invert_column();
		init_next_move();
	}
	else {
		line_start = 0xf800-2+line_offset;
		invert_column();
		line_start += 2;
		invert_column();
		line_offset += 1;
	}
}

// invert lines moving left
void process_line_left() {
	gravity_change_in_progress = 1;
	if (line_offset == LAST_COL_START) {
		current_direction = MOVING_LEFT;
		line_start = 0xf800+line_offset;
		invert_column();
		line_start -= 1;
		invert_column();
		line_offset -= 1;
	}
	else if (line_offset == 0) {
		line_start = 0xf800+line_offset;
		invert_column();
		line_start += 1;
		invert_column();
		init_next_move();
	}
	else {
		line_start = 0xf800+1+line_offset;		// remove last line
		invert_column();
		line_start -= 2;
		invert_column();
		line_offset -= 1;
	}
}

// copy tiles to the screen
unsigned char *copy_pos = 0;		// destination screen position
unsigned char *copy_src = 0;		// tile source data

void copy4x3(unsigned char dummy) {
    #asm
		push af
		push bc
		push de
		push hl
		ld de, (_copy_src)
		ld hl, (_copy_pos)
        ld b, 3             // 3 rows to copy
    row_loop:
		ld a,(de)
		ld (hl),a
		inc de
		inc hl
		ld a,(de)
		ld (hl),a
		inc de
		inc hl
		ld a,(de)
		ld (hl),a
		inc de
		inc hl
		ld a,(de)
		ld (hl),a
		inc de
		inc hl

		add hl, 60

        djnz row_loop        // Repeat for 3 rows
		pop hl
		pop de
		pop bc
		pop af
    #endasm;
}

// draw and analyse map
void draw_map(unsigned char *map) {
	unsigned char *pos = 0xf800;
	unsigned char id = 0;
	unsigned char tile_id = 0;
	unsigned char *origpos = pos;
	unsigned char tile_type = 0;
	unsigned char current_frame_id = 0;
	unsigned char *tilepos = 0;

	// init map data
	start_pos_x = 16;
	start_pos_y = 30;
	end_tile_addr1 = 0;
	end_tile_addr2 = 0;

	for(int row=0; row<10; row++) {
		// draw the map row by row
		#asm
			call $00F6 // wait for screen sync
		#endasm			
		origpos = pos;
		for(int col=0; col<16; col++) {
			tile_id = map[id];								// get tile id from the map
			tilepos = (unsigned char*)tile_pos[tile_id];	// get tile source data
			copy_pos = pos;
			copy_src = tilepos;
			copy4x3(0);										// draw the tile

			tile_type = tile_types[tile_id];				// check tile type
			switch(tile_type) {
				case T_END:
					if (end_tile_addr1 == 0) {
						end_tile_addr1 = pos;
						end_tile_id1 = tilepos;
						end_tile_out = 1;
					}
					else {
						end_tile_addr2 = pos;
						end_tile_id2 = tilepos;
					}
					break;
				case T_ENEMY_UP:
				case T_ENEMY_LEFT:
				case T_ENEMY_ROCK:
					if (enemy_count < MAX_ENEMY_COUNT) {
						enemy_dirs[enemy_count] = ENEMY_DIR_UP;
						if (tile_type == T_ENEMY_LEFT) {
							enemy_dirs[enemy_count] = ENEMY_DIR_LEFT;
						}
						if (tile_type == T_ENEMY_ROCK) {
							enemy_dirs[enemy_count] = ENEMY_DIR_ROCK;
						}
						enemy_dir_orig[enemy_count] = enemy_dirs[enemy_count];
						enemy_orig_posx[enemy_count] = ((col<<2)<<1)<<1;
						enemy_orig_posy[enemy_count] = (row*9)<<1;
						enemy_frame[enemy_count] = current_frame_id;
						current_frame_id++;
						if (current_frame_id >= MAX_ENEMY_FRAME) {
							current_frame_id = 0;
						}
						enemy_count++;
					}
					break;
				case T_START:
					start_pos_x = (col<<2)<<1;
					start_pos_y = row*9;
					break;
			}
			id++;
			pos += 4;
		}
		pos = origpos+3*64;
	}
}

//-----------------------------------
// player and enemy sprite routines
unsigned char *sprite_pos = 0xf850;		// sprite screen position
unsigned char *sprite_data = 0;			// sprite data source
unsigned char sprite_y_size = 3;		// sprite heights in char

void sprite_put_3(int dummy) {
    #asm
		push af
		push de
		push bc
		ld hl,(_sprite_pos)
		ld de,(_sprite_data)		
        ld a,(_sprite_y_size)
		ld b,a
		dec b
    sprite_loop_3:
		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc de

        add hl, 62           // go to next row in screen
        djnz sprite_loop_3   
		pop bc
		pop de
		pop af
    #endasm;
}

void sprite_put_5(int dummy) {
    #asm
		push af
		ld hl,(_sprite_pos)
		ld de,(_sprite_data)		
        ld a,(_sprite_y_size)
		ld b,a
		dec b
    sprite_loop_5:
		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc de
        add hl, 60           // go to next row in screen
        djnz sprite_loop_5   
		pop af
    #endasm;
}

void sprite_put_5_or(int dummy) {
    #asm
		push af
		ld hl,(_sprite_pos)
		ld de,(_sprite_data)		
        ld a,(_sprite_y_size)
		ld b,a
    sprite_loop_or_5:
		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc hl
		inc de

		ld a,(de)
		and (hl)
		ld (hl),a
		inc de

        add hl, 60           // go to next row in screen
        djnz sprite_loop_or_5
		pop af
    #endasm;
}

// sprite delete routines
unsigned char delete_y_size = 3;
unsigned char delete_x_size = 4;
unsigned char *delete_pos = 0xf850;

void sprite_delete_3(int dummy) {
    #asm
		push af
		push de
		push bc
		ld hl,(_delete_pos)		
        ld a,(_delete_y_size)             // 3 rows to copy
		ld b,a
		ld a,0xff
    delete_loop_3:
		ld (hl),a
		inc hl
		ld (hl),a
		inc hl
		ld (hl),a
        add hl, 62
        djnz delete_loop_3        // Repeat for 3 rows
		pop bc
		pop de
		pop af
   #endasm;
}

void sprite_delete_4(int dummy) {
    #asm
		push af
		ld hl,(_delete_pos)		
        ld a,(_delete_y_size)             // 3 rows to copy
		ld b,a
		ld a,0xff
    delete_loop_4:
		ld (hl),a
		inc hl
		ld (hl),a
		inc hl
		ld (hl),a
		inc hl
		ld (hl),a
        add hl, 61
        djnz delete_loop_4        // Repeat for 3 rows
		pop af
    #endasm;
}

void sprite_delete_5(int dummy) {
    #asm
		push af
		ld hl,(_delete_pos)		
        ld a,(_delete_y_size)             // 3 rows to copy
		ld b,a
		ld a,0xff
    delete_loop_5:
		ld (hl),a
		inc hl
		ld (hl),a
		inc hl
		ld (hl),a
		inc hl
		ld (hl),a
		inc hl
		ld (hl),a
        add hl, 60
        djnz delete_loop_5        // Repeat for 3 rows
		pop af
    #endasm;
}

// precalculate sprite data for faster drawing
unsigned char *player_data = 0;
unsigned char *player_pos = 0xf800;
unsigned char *player_delete_pos = 0;
unsigned char player_delete_size_x = 0;
unsigned char player_delete_size_y = 0;

void calculate_sprite_data() {
	unsigned char pos_x = player_pos_x>>8;			// calculate player pixel positions
	unsigned char pos_y = player_pos_y>>8;

	unsigned char xshift = pos_x & 1;				// calculate x,y shifts inside the char
	unsigned char yshift = table_mod3[pos_y];
	unsigned char player_id = (yshift<<1)+xshift;	// sprite frame with the proper shift
	player_data = astronaut_ptrs[current_direction][player_id];

	if (yshift == 0) {								// get sprite size based on the actual direction
		sprite_y_size = astronaut_sizey[current_direction];
	}
	else {
		sprite_y_size = astronaut_sizey[current_direction]+1;
	}

	unsigned char xchar = pos_x>>1;					// player screen position
	player_pos = (unsigned char*)table_ypos[pos_y] + xchar;

	// calculate enemy sprites
	for(unsigned char i=0; i < enemy_count; i++) {
		pos_x = enemy_posx[i]>>1;
		pos_y = enemy_posy[i]>>1;

		xshift = pos_x & 1;
		yshift = table_mod3[pos_y];
		player_id = (yshift<<1)+xshift;

		if (yshift == 0) {
			enemy_ysize[i] = 3;
		}
		else {
			enemy_ysize[i] = 4;
		}
		if (xshift == 0) {
			enemy_xsize[i] = 4;
		}
		else {
			enemy_xsize[i] = 5;
		}

		if (enemy_dirs[i] == ENEMY_DIR_ROCK) {
			enemy_data[i] = rock_ptrs[0][player_id];
		}
		else {										// enemies are animated
			enemy_data[i] = enemy_ptrs[enemy_frame[i]][player_id];
		}
		unsigned char xchar = pos_x>>1;
		enemy_pos[i] = (unsigned char*)table_ypos[pos_y] + xchar;	
	}
}

// delete sprite from the screen
void delete_sprite() {
	if (game_state == STATE_NORMAL || game_state == STATE_PLAYER_RESTART) {
		// delete player sprite
		if (player_delete_pos != 0) {
			delete_pos = player_delete_pos;
			delete_y_size = player_delete_size_y;
			switch(player_delete_size_x) {
				case 3:
					sprite_delete_3(0);
					break;
				case 5:
					sprite_delete_5(0);
					break;
			}
		}

		// animate the end arrow
		if (end_tile_out == 1 && (tick & 0x10)) {
			end_tile_out = 0;
			delete_pos = end_tile_addr1;
			delete_y_size = 3;
			sprite_delete_4(0);
			delete_pos = end_tile_addr2;
			sprite_delete_4(0);
		}

		// delete enemies
		for(unsigned char i=0; i < enemy_count; i++) {
			delete_pos = enemy_delete_pos[i];
			delete_y_size = enemy_ysize_del[i];
			if (delete_pos != 0) {
				if (enemy_xsize_del[i] == 4) {
					sprite_delete_4(0);
				}
				else {
					sprite_delete_5(0);
				}
			}
		}
	}
}

// show sprites in the screen
void show_sprite() {
	delete_sprite();

	// check whether we have to show sprites during the restart
	if (game_state == STATE_PLAYER_RESTART && ((tick&0x10) == 0)) {
		return;
	}
	// sprites should be shown only in normal and restart states
	if (game_state == STATE_NORMAL || game_state == STATE_PLAYER_RESTART) {
		sprite_pos = player_pos;
		sprite_data = player_data;
		switch(astronaut_sizex[current_direction]) {	// show player sprites based on direction
			case 3:
				sprite_put_3(0);
				break;
			case 5:
				sprite_put_5(0);
				break;
		}

		player_delete_pos = player_pos;					// save player sprite data for deletion
		player_delete_size_x = astronaut_sizex[current_direction];
		if (game_state == STATE_PLAYER_RESTART) {
			player_delete_size_y = 4;
		}
		else {
			if (current_direction == MOVING_DOWN) {
				player_delete_size_y = sprite_y_size-1;
			}
			else if (current_direction == MOVING_UP) {
				player_delete_size_y = sprite_y_size-1;
			}
			else {
				player_delete_size_y = sprite_y_size-1;
			}
		}

		// show enemy sprites
		for(unsigned char i=0; i < enemy_count; i++) {
			sprite_pos = enemy_pos[i];
			sprite_data = enemy_data[i];
			sprite_y_size = enemy_ysize[i];
			sprite_put_5_or(0);
		}

		// show end arrows
		if (end_tile_out == 0 && (tick & 0x10) == 0) {
			copy_pos = end_tile_addr1;
			copy_src = end_tile_id1;
			copy4x3(0);
			copy_pos = end_tile_addr2;
			copy_src = end_tile_id2;
			copy4x3(0);
			end_tile_out = 1;
		}	
	}
}

// initialize game data
void init_player_for_map() {
	delete_sprite();					// remove sprites from the screen
	player_speed_x = 0;					// stop player
	player_speed_y = 0;
	player_pos_x = start_pos_x << 8;	// restore player position
	player_pos_y = start_pos_y << 8;
	game_state = STATE_PLAYER_RESTART;	// set game state for restart
	tick = 0;
	current_direction = MOVING_DOWN;	// gravity is down
	player_delete_pos = 0;

	// initialize enemies
	for(unsigned char i = 0; i < enemy_count; i++) {
		enemy_posx[i] = enemy_orig_posx[i];
		enemy_posy[i] = enemy_orig_posy[i];
		enemy_delete_pos[i] = 0;
		enemy_dirs[i] = enemy_dir_orig[i];
	}
}

//-----------------------------------------
// stage cleared and end game

unsigned char *message = 0;		// current message (cleared or end)

void stage_cleared() {
	message = (unsigned char*)level_cleared;
	if (levels[current_level+1] == 0) {
		message = (unsigned char*)finish;
	}

	// copy message to screen
#asm
	push de
	push bc
	push af

	ld de, 0xf800+10*64+14
	ld hl, (_message)
	ld bc, 37
	ldir
	add de, 64-37
	call $00F6
	ld bc, 37
	ldir
	add de, 64-37
	call $00F6
	ld bc, 37
	ldir
	add de, 64-37
	call $00F6
	ld bc, 37
	ldir
	add de, 64-37
	call $00F6
	ld bc, 37
	ldir
	add de, 64-37
	call $00F6
	ld bc, 37
	ldir
	add de, 64-37
	call $00F6
	ld bc, 37
	ldir
	add de, 64-37
	call $00F6
	ld bc, 37
	ldir
	add de, 64-37
	call $00F6

	pop af
	pop bc
	pop de
#endasm
	sound_cleared();		// play sound

	waitforspace();

	// next level
	current_level++;
	if (levels[current_level] == 0) {
		// last map, restart the game
		current_level = 0;
	}
	current_map = levels[current_level];

	enemy_count = 0;
	// draw new map
	draw_map(current_map);

	// don't delete anything
	player_delete_pos = 0;
	for(unsigned char i = 0; i < enemy_count; i++) {
		enemy_delete_pos[i] = 0;
	}

	// init player
	init_player_for_map();
}

//---------------------------------------
// calculate sprite next positions

void sprite_movements() {
	unsigned char type;

	tick_frame += 1;
	// verify enemy positions
	for (unsigned char i=0; i < enemy_count; i++){
		unsigned char current_dir = enemy_dirs[i];
		enemy_delete_pos[i] = enemy_pos[i];
		enemy_new_posx[i] = enemy_posx[i];
		enemy_new_posy[i] = enemy_posy[i];
		enemy_ysize_del[i] = enemy_ysize[i];
		enemy_xsize_del[i] = enemy_xsize[i];
		switch (current_dir) {
			case ENEMY_DIR_DOWN:
				enemy_new_posy[i] = enemy_posy[i]+1;
				break;
			case ENEMY_DIR_UP:
				enemy_new_posy[i] = enemy_posy[i]-1;
				break;
			case ENEMY_DIR_RIGHT:
				enemy_new_posx[i] = enemy_posx[i]+1;
				break;
			case ENEMY_DIR_LEFT:
				enemy_new_posx[i] = enemy_posx[i]-1;
				break;
			case ENEMY_DIR_ROCK:
				switch(current_direction) {
					case MOVING_DOWN:
						enemy_new_posy[i] = enemy_posy[i]+2;
						break;
					case MOVING_UP:
						enemy_new_posy[i] = enemy_posy[i]-2;
						break;
					case MOVING_RIGHT:
						enemy_new_posx[i] = enemy_posx[i]+2;
						break;
					case MOVING_LEFT:
						enemy_new_posx[i] = enemy_posx[i]-2;
						break;
				}
				break;
		}
		unsigned char newy = enemy_new_posy[i]>>1;
		unsigned char newx = enemy_new_posx[i]>>1;

		unsigned char xshift = newx & 1;
		unsigned char yshift = table_mod3[newy];
		unsigned char sizex = 4;
		unsigned char sizey = 3;
		if (xshift != 0) {
			sizex++;
		}
		if (yshift != 0) {
			sizey++;
		}

		type = check_sprite_new_position(newx, newy, sizex, sizey);
		if (type == T_GROUND || type == T_DANGER || type == T_END) {
			// change direction
			if (enemy_dirs[i] != ENEMY_DIR_ROCK) {
				enemy_dirs[i] = enemy_new_dir[current_dir];
			}
		}
		else {
			unsigned char set_enemy_pos = 1;
			// check collision for rocks
			if (enemy_dirs[i] == ENEMY_DIR_ROCK) {
				for (unsigned char j=0; j < enemy_count; j++) {
					if (i != j && enemy_dirs[j] == ENEMY_DIR_ROCK) {
						unsigned char pposx = newx;
						unsigned char pposy = newy;
						unsigned char pposx_end = pposx+8;
						unsigned char pposy_end = pposy+9;

						unsigned char eposx = (enemy_posx[j]>>1);
						unsigned char eposy = (enemy_posy[j]>>1);
						unsigned char eposx_end = eposx+8;
						unsigned char eposy_end = eposy+9;

						if ((pposx <= eposx && pposx_end > eposx) ||
							(pposx >= eposx && pposx < eposx_end)) {
							if ((pposy <= eposy && pposy_end > eposy) ||
							(pposy >= eposy && pposy < eposy_end)) {
								set_enemy_pos = 0;
								break;
							}
						}
					}
				}
			}
			if (set_enemy_pos == 1) {
				// apply new positions
				enemy_posx[i] = enemy_new_posx[i];
				enemy_posy[i] = enemy_new_posy[i];
			}
		}

		if (tick_frame > 4) {
			enemy_frame[i] += 1;
			if (enemy_frame[i] >= MAX_ENEMY_FRAME) {
				enemy_frame[i] = 0;
			}
		}
	}

	if (tick_frame > 4) {
		tick_frame = 0;
	}

	// calculate player new position
	unsigned int new_pos_x = player_pos_x + player_speed_x;
	unsigned int new_pos_y = player_pos_y + player_speed_y;

	// verify player position
	type = check_sprite_new_position((new_pos_x>>8), (new_pos_y>>8), astronaut_sizex[current_direction], astronaut_sizey[current_direction]);
	switch(type) {
		case T_GROUND:
			player_speed_x = 0;
			player_speed_y = 0;
			return;
		case T_DANGER:
			sound_restart();
			init_player_for_map();
			return;
		case T_END:
			stage_cleared();
			return;
	}

	// calculate collisions with enemies
	// new_pos_x, new_pos_y
	// enemy_posx[i]>>1, enemy_posy[i]>>1

	unsigned char pposx = (new_pos_x>>8)+1;
	unsigned char pposy = (new_pos_y>>8)+1;
	unsigned char pposx_end = pposx + 5;
	unsigned char pposy_end = pposy + 5;

	for (unsigned char i=0; i < enemy_count; i++) {
		unsigned char eposx = (enemy_posx[i]>>1)+1;
		unsigned char eposy = (enemy_posy[i]>>1)+1;
		unsigned char eposx_end = eposx + 5;
		unsigned char eposy_end = eposy + 5;

		if ((pposx <= eposx && pposx_end >= eposx) ||
			(pposx >= eposx && pposx <= eposx_end)) {
			if ((pposy <= eposy && pposy_end >= eposy) ||
			(pposy >= eposy && pposy <= eposy_end)) {
				sound_restart();
				init_player_for_map();
				return;
			}
		}
	}

	// adjust new position
	player_pos_x = new_pos_x;
	player_pos_y = new_pos_y;

	// adjust gravity
	switch(current_direction) {
		case MOVING_DOWN:
			player_speed_y += gravity;
			if (player_speed_y > MAX_SPEED) {
				player_speed_y = MAX_SPEED;
			}
			break;
		case MOVING_UP:
			player_speed_y -= gravity;
			if (player_speed_y < -MAX_SPEED) {
				player_speed_y = -MAX_SPEED;
			}
			break;
		case MOVING_RIGHT:
			player_speed_x += gravity;
			if (player_speed_x > MAX_SPEED) {
				player_speed_x = MAX_SPEED;
			}
			break;
		case MOVING_LEFT:
			player_speed_x -= gravity;
			if (player_speed_x < -MAX_SPEED) {
				player_speed_x = -MAX_SPEED;
			}
			break;
	}
}

//--------------------------------------
// starting screen

unsigned char *show_line_src = 0;

void show_line(unsigned char dummy) {
#asm
	push de
	push bc
	push af
	ld de, 0xf800+31*64
	ld hl, (_show_line_src)
	ld bc, 64
	ldir
	pop af
	pop bc
	pop de
#endasm
}

void show_start_screen() {
#asm
	push de
	push bc
	push af

	ld de, 0xf800
	ld hl, _start_screen
	ld bc, 0x200
	ldir
	call $00F6
	ld bc, 0x200
	ldir
	call $00F6
	ld bc, 0x200
	ldir
	call $00F6
	ld bc, 0x200
	ldir
	call $00F6

	pop af
	pop bc
	pop de
#endasm

	sound_start();

	tick = 0;
	while(1) {
#asm
		call $00F6 // wait for screen sync
#endasm
		if (tick & 0x40) {
			show_line_src = (unsigned char*)(start_screen+31*64);
		}
		else {
			show_line_src = (unsigned char*)press_space_line;
		}
		show_line(0);

		if ((keysspace[0] & KEY_SPACE) == 0) {
			game_mode = GAME_MODE_EASY;
			return;
		}
		if ((fghi_keys[0] & KEY_H) == 0) {
			game_mode = GAME_MODE_HARD;
			return;
		}
		tick++;
	}
}

//--------------------------------------
// main game loop

void main() { // A program kezdete
	unsigned char new_direction = MOVING_NONE;
#asm
	ld sp,$40ff
	call $00F6 // wait for screen sync
#endasm			
	//current_map = &test_map_struct;
	//draw_map(current_map->map_data);
	show_start_screen();

	line_start = 0xf800+30*64;
	clear_line(0);
	line_start = 0xf800+31*64;
	clear_line(0);

	//current_map = test_map;
	current_map = levels[current_level];
	draw_map(current_map);

	init_player_for_map();

	while (1) {
		if (game_state == STATE_NORMAL) {
			sprite_movements();
		}
		calculate_sprite_data();
		tick++;
#asm
	call $00F6 // wait for screen sync
#endasm			
		show_sprite();

		if (game_state == STATE_NORMAL || game_state == STATE_GRAVITY_CHANGE) {
			new_direction = MOVING_NONE;
			if ((arrow_keys[0] & KEY_LEFT)==0) {
				new_direction = MOVING_LEFT;
			}
			else if ((arrow_keys[0] & KEY_RIGHT)==0) {
				new_direction = MOVING_RIGHT;
			}
			else if ((arrow_keys[0] & KEY_DOWN)==0) {
				new_direction = MOVING_DOWN;
			}
			else if ((arrow_keys[0] & KEY_UP)==0) {
				new_direction = MOVING_UP;
			}

			if (moving_line != new_direction) {
				next_move = new_direction;
			}

			if (next_move != MOVING_NONE && moving_line == MOVING_NONE) {
				init_next_move();
			}

			if (game_state == STATE_GRAVITY_CHANGE) {
				switch (moving_line) {
					case MOVING_DOWN:
						process_line_down();
						break;
					case MOVING_UP:
						process_line_up();
						break;
					case MOVING_LEFT:
						process_line_left();
						break;
					case MOVING_RIGHT:
						process_line_right();
						break;
				}
			}
		}
		// finish player restart
		if (game_state == STATE_PLAYER_RESTART) {
			if (tick > 80) {
				game_state = STATE_NORMAL;
			}
		}

		// remove if hard mode
		if (gravity_change_in_progress == 1) {
			if (game_mode == GAME_MODE_EASY) {
				player_speed_x = 0;
				player_speed_y = 0;
			}
		}

		// restart level?
		if ((pqrs_keys[0] & KEY_R)==0) {
			init_player_for_map();
		}
	}
}

