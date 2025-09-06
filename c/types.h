typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef unsigned long u32;

// input
__sfr __at 0 STREAM;

// output
__sfr __at 0 STDOUT;
__sfr __at 1 SECTOR0;
__sfr __at 2 SECTOR1;
__sfr __at 3 SECTOR2;
__sfr __at 4 LEFT;
__sfr __at 5 RIGHT;
__sfr __at 6 TOP;
__sfr __at 7 BOTTOM;
__sfr __at 8 DATA_L;
__sfr __at 9 DATA_H;

