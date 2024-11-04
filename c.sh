rm *.map
rm *.s
rm *.o

cc65 -Oirs --codesize 500 -t atmos bas.c
#cc65 -Cl -Ois --codesize 500 -t atmos bas.c
ca65 bas.s
ld65 -o bas.tap -t atmos -m bas.map bas.o atmos.lib
