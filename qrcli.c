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
	uint64_t* format;
	uint64_t* correction;
	uint64_t* version_min;
	uint64_t* version_max;
} AppConfig;

AppConfig config = {0};

void PrintHelp() {
	printf("qrcli -help\n");
	printf("qrcli <flags> [text]\n");
  flag_print_options(stdout);
}

bool ArgsParse(int argc, char** argv) {
	config.help = flag_bool("help", 0, "show help");
	config.format = flag_uint64("format", 1, "printed QR-code, 1-2 (small symbols/big symbols)");
	config.correction = flag_uint64("correction", 2, "error code correction, 1-4 (low, medium, quartile, high)");
	config.version_min = flag_uint64("version-min", qrcodegen_VERSION_MIN, "minimal QR-code version to choose from");
	config.version_max = flag_uint64("version-max", qrcodegen_VERSION_MAX, "maximal QR-code version to choose from");

	if (!flag_parse(argc, argv)) {
    PrintHelp();
		flag_print_error(stderr);
		exit(1);
	}

	// -help
	if (*config.help) {
    PrintHelp();
		exit(0);
	}

	// -format
	if (*config.format < 1 || *config.format > 2) {
		printf("Wrong print format of the QR-code, must be (1-2)\n");
		exit(1);
	}

	// -correction
	if (*config.correction < 1 || *config.correction > 4) {
		printf("Wrong error code correction, must be 1-4\n");
		exit(1);
	}

	// -version-min/-version-max
	if (*config.version_min < qrcodegen_VERSION_MIN || *config.version_min > qrcodegen_VERSION_MIN) {
		printf("Wrong minimal version, must be 1-%d\n", qrcodegen_VERSION_MIN);
	}

	if (*config.version_max < qrcodegen_VERSION_MIN || *config.version_max > qrcodegen_VERSION_MAX) {
		printf("Wrong maximal version, must be 1-%d\n", qrcodegen_VERSION_MAX);
	}

	if (*config.version_min > *config.version_max) {
		printf("Minimal version is higher than maximal version\n");
		exit(1);
	}

	// Rest argv
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
	for (int y = 0; y < bitmap_side; y++) {
		for (int x = 0; x < bitmap_side; x++) {
			printf("%s", bitmap[y*bitmap_side+x] != inv ? "██" : "  ");
		}
		printf("\n");
	}
}

void PrintBitmapSmall(uint8_t* bitmap, int bitmap_side, bool inv) {
	const char p[][4] = { " ", "▄", "▀", "█"};
	for (int hy = 0; hy < bitmap_side/2 + bitmap_side%2; hy++) {
		for (int x = 0; x < bitmap_side; x++) {
			uint8_t up, down;
			up = bitmap[(hy*2)*bitmap_side+x] != inv;
			if (hy*2+1 < bitmap_side)
				down = bitmap[(hy*2+1)*bitmap_side+x] != inv;
			else
				down = inv;
			uint8_t index = ((up & 1) << 1) | (down & 1);
			printf("%s", p[index]);
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
		*config.version_min,
		*config.version_max,
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

	switch (*config.format) {
		case 1:
			PrintBitmapSmall(bitmap, bitmap_side, 0);
			PrintBitmapSmall(bitmap, bitmap_side, 1);
			break;
		case 2:
			PrintBitmapBig(bitmap, bitmap_side, 0);
			PrintBitmapBig(bitmap, bitmap_side, 1);
			break;
	}

	free(bitmap);

	return 0;
}
