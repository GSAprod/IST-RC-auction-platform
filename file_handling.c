#include "file_handling.h"

int checkAssetFile(char * filename) {
	struct stat filestat;
	int ret_stat = stat(filename, &filestat);

	if (ret_stat == -1 || filestat.st_size == 0) {
		return 0;
	}

	return filestat.st_size;
}

