#ifndef _MAIN_H_
#define _MAIN_H_

struct Rect {
	int left, right, top, bottom;
};

void file_seek(int sector);
int file_getc();
void emu_exit();
void set_area(Rect *rect);
void set_data(int data);

#endif
