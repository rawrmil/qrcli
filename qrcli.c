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

#define MAX_BUFFER_SIZE (qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_VERSION_MAX))

// --- ARGS ---

typedef struct AppConfig {
	int rargc;
	char** rargv;
	bool* help;
	uint64_t* correction;
} AppConfig;

AppConfig config = {0};

void PrintHelp() {
	printf("qrcli -help\n");
	printf("qrcli <flags> [text]\n");
  flag_print_options(stdout);
}

bool ArgsParse(int argc, char** argv) {
	config.help = flag_bool("help", 0, "show help");
	config.correction = flag_uint64("correction", 2, "error code correction, 1-4 (low, medium, quartile, high)");

	if (!flag_parse(argc, argv)) {
    PrintHelp();
		flag_print_error(stderr);
		exit(1);
	}

	if (*config.help) {
    PrintHelp();
		exit(0);
	}

	if (*config.correction < 1 || *config.correction > 4) {
		printf("Wrong error code correction, must be 1-4\n");
		exit(1);
	}

	config.rargc = flag_rest_argc();
	if (config.rargc == 0) {
    PrintHelp();
		exit(1);
	}
	config.rargv = flag_rest_argv();
}

void GetInputString(Nob_String_Builder* sb) {
	for (int i = 0; i < config.rargc; i++) {
		nob_sb_append_cstr(sb, config.rargv[i]);
		nob_sb_append_cstr(sb, i < config.rargc-1 ? " " : "");
	}
	nob_sb_append_null(sb);
}

// --- BITMAP ---

void PrintBitmapBig(uint8_t* bitmap, int bitmap_side, bool inv) {
	for (int x = 0; x < bitmap_side; x++) {
		for (int y = 0; y < bitmap_side; y++) {
			printf("%s", bitmap[y*bitmap_side+x] != inv ? "██" : "  ");
		}
		printf("\n");
	}
}

// --- MAIN ---

int main(int argc, char** argv) {

	ArgsParse(argc, argv);

	Nob_String_Builder sb = {0};
	GetInputString(&sb);
	printf("Encoding: '%s'\n", sb.items);

	uint8_t tmp[MAX_BUFFER_SIZE] = {0};
	uint8_t qrc[MAX_BUFFER_SIZE] = {0};

	bool success = qrcodegen_encodeText(
		sb.items,
		tmp,
		qrc,
		*config.correction-1,
		qrcodegen_VERSION_MIN,
		qrcodegen_VERSION_MAX,
		qrcodegen_Mask_AUTO,
		false);

	if (!success) {
		printf("Too much data\n");
		exit(1);
	}

	int size = qrcodegen_getSize(qrc);

	int bitmap_side = size+2;
	size_t bitmap_size = (size+2)*(size+2);
	uint8_t* bitmap = calloc(bitmap_size, sizeof(*bitmap));
	memset(bitmap, 0, bitmap_size);

	printf("%dx%d\n", size, size);
	for (int x = 0; x < size; x++) {
		for (int y = 0; y < size; y++) {
			bitmap[(y+1)*bitmap_side+(x+1)] = qrcodegen_getModule(qrc, x, y);
		}
	}

	PrintBitmapBig(bitmap, bitmap_side, 0);
	PrintBitmapBig(bitmap, bitmap_side, 1);

	free(bitmap);

	return 0;
}
