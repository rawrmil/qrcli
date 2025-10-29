#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"
#undef NOB_IMPLEMENTATION

int main(int argc, char** argv) {
	NOB_GO_REBUILD_URSELF(argc, argv);
	Cmd c = {0};
	cmd_append(&c, "cc", "qrcli.c", "-o", "qrcli");
	if (!cmd_run(&c)) return 1;
	return 0;
}
