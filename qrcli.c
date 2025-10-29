#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "qrcodegen.h"

#define NOB_IMPLEMENTATION
#include "nob.h"
#undef NOB_IMPLEMENTATION

#define FLAG_IMPLEMENTATION
#include "flag.h"
#undef FLAG_IMPLEMENTATION

#define BUF (qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_VERSION_MAX))

void print_help() {
	printf("qrcli -help\n");
	printf("qrcli <text> <flags>\n");
  flag_print_options(stdout);
}

void print_pic_big(uint8_t* pic, int side, bool inv) {
	for (int x = 0; x < side; x++) {
		for (int y = 0; y < side; y++) {
			printf("%s", pic[y*side+x] != inv ? "██" : "  ");
		}
		printf("\n");
	}
}

int main(int argc, char** argv) {

	bool* f_help = flag_bool("help", 0, "show help");

	if (!flag_parse(argc, argv)) {
    print_help();
		flag_print_error(stderr);
		exit(1);
	}

	if (*f_help) {
    print_help();
		exit(0);
	}

	int rargc = flag_rest_argc();
	if (rargc == 0) {
    print_help();
		exit(1);
	}
	char** rargv = flag_rest_argv();

	Nob_String_Builder sb = {0};
	for (int i = 0; i < rargc; i++) {
		nob_sb_append_cstr(&sb, rargv[i]);
		nob_sb_append_cstr(&sb, i < rargc-1 ? " " : "");
	}
	nob_sb_append_null(&sb);
	printf("Encoding: '%s'\n", sb.items);

	uint8_t tmp[BUF] = {0};
	uint8_t qrc[BUF] = {0};
	if (!qrcodegen_encodeText(sb.items, tmp, qrc, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, false)) {
		printf("Too much data\n");
		exit(1);
	}

	int size = qrcodegen_getSize(qrc);
	size_t pic_buf = (size+2)*(size+2);
	uint8_t* pic = calloc(pic_buf, sizeof(*pic));
	memset(pic, 0, pic_buf);
	printf("%dx%d\n", size, size);
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			pic[(y+1)*(size+2)+(x+1)] = qrcodegen_getModule(qrc, x, y);
		}
	}

	print_pic_big(pic, size+2, 0);
	print_pic_big(pic, size+2, 1);

	free(pic);

	return 0;
}
