#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <sys/stat.h>


/*
 * Check if the file exists and is not empty.
 * 
 * @param filename The name of the file to check.
 * @return 0 if the file does not exist or is empty, otherwise the size of the file.
*/
int checkAssetFile(char * filename);



#endif
