#include <time.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
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
    POKE(0x26A, 11);                // disable keyb click with bitflag at #26A
}

byte* chr = (byte*)0x9900;  // pointer to ASCII characters
byte* achr = (byte*)0x9d00; // pointer to alternative characters
const byte* screen = (byte*)0xa000; // pointer to start of screen
const byte flipmap[8] = { 0,4,2,6,1,5,3,7 };

struct sprite {
    byte bwidth; // how may bytes wide (groups of 6 pixels)
    byte height; // how many lines
    byte nr_frames;
    byte skipped_frames;
    unsigned int frame_offset;
    unsigned int frame_pointer;
    byte cur_frame;
    unsigned int pos;
    byte faceright;
    byte moving;
    byte* walk_ani;
    byte* stand_ani;
};

struct sprite player;

void draw_spr(byte spr[], byte w, byte h, int haddr) {
    byte i;
    int b = 0;
    for(i=0;i<h;i++) {
        memcpy((char *)haddr, &spr[b], w);
        b += w;
        haddr+=40;
    }
}

// flip the six bits holding the pixels
byte flip(byte b) {
    byte r=b&0b00111111;
    r=(flipmap[r&0b111] << 3) | flipmap[r>>3];
    return r|0b01000000;
}

void draw_sprR(byte spr[], byte w, byte h, int haddr) {
    byte i,j, c;
    byte inv;
    int b=0;
    c = w-1;
    for(i=0;i<h;i++) {
        for(j=0;j<w;j++) {
            POKE(haddr+c-j, inv=flip(spr[b+j]));
        }
        b += w;
        haddr+=40;
    }
}

void draw_char(byte cnum, int haddr) {
    byte i=0;
    byte offst = cnum<<3;
    for(i;i<8;i++) {
        POKE(haddr,(64|chr[offst+i]));
        haddr+=40;
    }
}

void draw_achar(byte cnum, int haddr) {
    byte i=0;
    byte offst = cnum<<3;
    for(i;i<8;i++) {
        POKE(haddr, (64|achr[offst+1]));
        haddr+=40;
    }
}

void delay(byte c) {
    clock_t goal;
    goal = clock () + CLOCKS_PER_SEC/c;
    while ((long) (goal - clock ()) > 0) ;
}

void move_right(struct sprite *spr) {
    spr->moving = 1;
    if(!spr->faceright) {
        spr->faceright=1;
        spr->pos=spr->pos-1;
    }
//    if(spr->pos<38) {
        spr->pos++;
        spr->skipped_frames++;
//    }
}

void move_left(struct sprite *spr) {
    spr->moving = 1;
    if(spr->faceright) {
        spr->faceright=0;
        spr->pos=spr->pos+1;
    }
    if(spr->pos>1) {
        spr->pos=spr->pos-1;
        spr->skipped_frames++;
    }
}

void animate_sprite(struct sprite *spr) {
    byte i;
    unsigned int b,p = 0;
    if(spr->skipped_frames > 0) {
        p = spr->pos - spr->skipped_frames;
        for(i=0;i<spr->height;i++) {
            memset((char *)0xa000+p+b,64,spr->skipped_frames);
            b += 40;
        }
        b = 0;
        spr->skipped_frames = 0;
    }
    if(spr->cur_frame < spr->nr_frames) {
        spr->cur_frame++;
        spr->frame_pointer+=spr->frame_offset;
    } else {
        spr->cur_frame = 0;
        spr->frame_pointer = 0;
    }
    if(spr->faceright) {
        draw_spr(spr->walk_ani+spr->frame_pointer,spr->bwidth,spr->height,0xa000+spr->pos);
    } else {
        draw_sprR(spr->walk_ani+spr->frame_pointer,spr->bwidth,spr->height,0xa000+spr->pos);
    }
}

void init_player() {
    player.bwidth = 11;
    player.height = 64;
    player.frame_offset = 704;
    player.skipped_frames = 1;
    player.nr_frames = 6;
    player.pos = 0;
    player.faceright = 1;
    player.moving = 0;
    player.walk_ani = walk;
}

void print_stat(const char* str) {
    memcpy((char *)0xBF68, str, strlen(str));
}

void main() {
    byte c;
    char str[80];
    clock_t goal = 0;
    clock_t clk = 0;
    setup();
    init_player();
    while(1) {
        player.moving = 0;
        if(kbhit()>0) {
            c = cgetc();
            if(c==CH_ESC) return;
            if(c==CH_CURS_RIGHT) move_right(&player);
            if(c==CH_CURS_LEFT) {
                sprintf(str, "clk: %d, goal: %d", clock (), goal);
                print_stat(str);
            }
        }
        //animate_sprite(&player);
        //delay(30);
        clk = clock();
        if((long) (goal - clk) < 1) {
            goal = clk + CLOCKS_PER_SEC/10;
            animate_sprite(&player);
            //sprintf(str, "off: %d, skip: %d, pos: %d, right: %d, moving: %d", player.frame_offset, player.skipped_frames, player.pos, player.faceright, player.moving);
            //sprintf(str, "clk: %d, goal: %d      ", clk, goal);
            //print_stat(str);
        }
    }
}
