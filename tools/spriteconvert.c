#include <stdio.h>
#include <stdint.h>
#include "../images/bird.h"

#define byte unsigned char
#define STANDING_FRAME_COUNT 7
#define STANDING_FRAME_WIDTH 66
#define STANDING_FRAME_HEIGHT 64

byte debug=0;

byte mask[6]={
  0b100000, //32
  0b010000, //16
  0b001000, //8
  0b000100, //4
  0b000010, //2
  0b000001, //1
};

byte f[704];

int main()
{
    byte one = 64; byte d = 0;
    if(!debug) printf("{ ");
    for(int k=0;k<STANDING_FRAME_COUNT;k++) {
        for(int j=0;j<STANDING_FRAME_HEIGHT;j++) {
            for(int i=0;i<STANDING_FRAME_WIDTH;i++) {
                d=i%6;
                if(bird_data[k][j*STANDING_FRAME_WIDTH+i] == 0xff00ffff) {
                    if(debug) putchar('*');
                    one = one|mask[d];
                } else {
                    if(debug) putchar(' ');
                }
                if(d==5) {
                    byte c = i/6;
                    f[j*c+c]=one;
                    if(!debug) printf("%d, ",one);
                    one=64;
                }
            }
            if(debug) putchar('\n');
        }
    }

    if(!debug) printf("} \n");

    return 0;
}
