#include "types.h"
#include "xprintf.h"
#include "ff.h"
#include "picojpeg.h"

#define printf	xprintf

int pjpeg_decompress(void);

static FATFS fatfs;
static FIL file;
static FILINFO info;
static DIR dir;

void putchar(int c) {
	STDOUT = c;
}

u8 read_jpeg(u8 *buf, u8 buf_size, u8 *actually_read, void *data) {
	UINT bytesread;
	data;
	f_read(&file, buf, buf_size, &bytesread);
	*actually_read = bytesread;
	return 0;
}

int main() {
	int i, r;
	xdev_out(putchar);
	if (r = f_mount(&fatfs, "0:", 0) != FR_OK) {
		printf("f_mount: %d\n", r);
		return 1;
	}
	if (r = f_chdir("/MJPEG") != FR_OK) {
		printf("f_chdir: %d\n", r);
		return 1;
	}
	if (r = f_opendir(&dir, ".") != FR_OK) {
		printf("f_opendir: %d\n", r);
		return 1;
	}
	for (i = 1; f_readdir(&dir, &info) == FR_OK && *info.fname; i++) {
		if (r = f_open(&file, info.fname, FA_READ) != FR_OK) {
			printf("f_open: %d\n", r);
			return 1;
		}
		pjpeg_decompress();
		f_close(&file);
	}
	f_closedir(&dir);
	printf("%d frames\n", i - 1);
	return r;
}

