#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "qrcodegen.h"

int main(int argc, char** argv) {
	char text[] = "test\nlol";
	uint8_t tmp[1024] = {0};
	uint8_t qrc[1024] = {0}; 
	if (!qrcodegen_encodeText(text, tmp, qrc, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, false)) {
		return 1;
	}

	int size = qrcodegen_getSize(qrc);
	// (size+2)*(size+2)
	uint8_t pic[1024];
	memset(pic, 0, 1024);
	printf("%dx%d\n", size, size);
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			pic[(y+1)*(size+2)+(x+1)] = qrcodegen_getModule(&qrc, x, y);
		}
	}

	for (int x = 0; x < size+2; x++) {
		for (int y = 0; y < size+2; y++) {
			printf("%s", pic[y*(size+2)+x] ? "██" : "  ");
		}
		printf("\n");
	}

	return 0;
}
