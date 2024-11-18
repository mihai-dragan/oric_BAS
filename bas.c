#include <time.h>
#include <conio.h>
#include <string.h>
#include <peekpoke.h>
#define byte unsigned char
#include "bird.h"

void setup() {
    if(PEEK(0xC800))
            asm("jsr $EC33");       // HIRES in Atmos (ROM 1.1)
    else
            asm("jsr $E9BB");       // HIRES in Oric-1 (ROM 1.0)
    memset((char*)0xBF68, 0, 120);  // Clear lower text area
    POKE(0x24E, 5);                 // set keyb delay 5 at #24E
    POKE(0x26A, 10);                // disable keyb click and caret blink with bitflag at #26A
}

struct sprite {
    byte bwidth; // how may bytes wide (groups of 6 pixels)
    byte height; // how many lines
    byte last_frame;
    byte skipped_frames;
    unsigned int frame_offset;
    unsigned int frame_pointer;
    byte cur_frame;
    unsigned int pos;
    byte* walk_ani;
};

struct sprite player;

// draw one frame pointed to by spr, at screen address haddr
void draw_spr(byte spr[], byte w, byte h, int haddr) {
    byte i;
    int b = 0;
    for(i=0;i<h;i++) {
        memcpy((char *)haddr, &spr[b], w); // draw one full line from frame
        b += w; // next line in frame
        haddr+=40; //next line on screen
    }
}

void move_right(struct sprite *spr) {
	spr->pos++;
	spr->skipped_frames++;
}

void animate_sprite(struct sprite *spr) {
    byte i;
    unsigned int p = 0;
    byte skip = spr->skipped_frames;
    if(skip > 0) { // erase part of old sprite
        p = spr->pos - skip + 0xa000;
        for(i=0;i<spr->height;i++) {
            memset((char *)p,64,skip);
            p += 40;
        }
        spr->skipped_frames = 0;
    }
    if(spr->cur_frame < spr->last_frame) { // set next frame
        spr->cur_frame++;
        spr->frame_pointer+=spr->frame_offset;
    } else {
        spr->cur_frame = 0;
        spr->frame_pointer = 0;
    }
    // draw frame
    draw_spr(spr->walk_ani+spr->frame_pointer,spr->bwidth,spr->height,0xa000+spr->pos);
}

void init_player() {
    player.bwidth = 11;
    player.height = 64;
    player.frame_offset = 704;
    player.skipped_frames = 1;
    player.last_frame = 6;
    player.pos = 0;
    player.walk_ani = walk;
}

void main() {
    byte c;
    clock_t interval = CLOCKS_PER_SEC/12;
    clock_t clk = 0;
    setup();
    init_player();
    while(1) {
        if(kbhit()>0) {
            c = cgetc();
            if(c==CH_ESC) return;
            if(c==CH_CURS_RIGHT) move_right(&player);
        }
        clk = clock();
        if(clk%interval==0) animate_sprite(&player);
    }
}
