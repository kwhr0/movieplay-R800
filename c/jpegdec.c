#include "types.h"
#include "picojpeg.h"
#include "xprintf.h"

#define printf	xprintf

typedef unsigned char uint8;
typedef unsigned int uint;

uint8 read_jpeg(uint8 *buf, uint8 buf_size, uint8 *actually_read, void *data);

void pjpeg_decompress(void) {
	pjpeg_image_info_t info;
	int mcu_x = 0, mcu_y = 0;
	uint8 status;
	status = pjpeg_decode_init(&info, read_jpeg, 0, 0);
	if (status) {
		printf("pjpeg_decode_init() failed with status %u\n", status);
		goto exit;
	}
	while (1) {
		int x, y;
		status = pjpeg_decode_mcu();
		if (status) {
			if (status != PJPG_NO_MORE_BLOCKS) {
				printf("pjpeg_decode_mcu() failed with status %u\n", status);
				goto exit;
			}
			break;
		}
		if (mcu_y >= info.m_height) goto exit;
		for (y = 0; y < info.m_MCUHeight; y += 8) {
			for (x = 0; x < info.m_MCUWidth; x += 8) {
				uint src_ofs = (x << 3) + (y << 4);
				uint8 *pR = info.m_pMCUBufR + src_ofs;
				uint8 *pG = info.m_pMCUBufG + src_ofs;
				uint8 *pB = info.m_pMCUBufB + src_ofs;
				int bx, by, xpos, ypos;
				xpos = mcu_x + x;
				ypos = mcu_y + y;
				LEFT = xpos;
				RIGHT = xpos + 7;
				TOP = ypos;
				BOTTOM = ypos + 7;
				for (by = 0; by < 8; by++) {
					for (bx = 0; bx < 8; bx++) {
						uint g = *pG++ << 2;
						DATA_L = g & 0xe0 | *pB++ >> 3 & 0x1f;
						DATA_H = *pR++ >> 1 & 0x7c | g >> 8 & 3;
					}
				}
			}
		}
		mcu_x += info.m_MCUWidth;
		if (mcu_x >= info.m_width) {
			mcu_x = 0;
			mcu_y += info.m_MCUHeight;
		}
	}
exit:
}

