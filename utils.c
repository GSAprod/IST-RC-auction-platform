#include "utils.h"

int mode_verbose = 0;

void set_mode_verbose() {
	mode_verbose = 1;
}

int get_mode_verbose() {
	return mode_verbose;
}