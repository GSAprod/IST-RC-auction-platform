#ifndef UTILS_H
#define UTILS_H

#define FILE_INFO_SIZE 128
#define PATHNAME_SIZE 280
#define MESSAGE_TYPE_SIZE 5

#define TINY_BUFFER 8
#define SMALL_BUFFER 32
#define MEDIUM_BUFFER 128
#define LARGE_BUFFER 1024
#define LIST_BUFFER 8192

#define MAX_PROMPT_NUMBER 16
#define MAX_PROMPT_SIZE 512

#define AID_SIZE 4
#define UID_SIZE 7
#define FILENAME_SIZE 32
#define VALUE_SIZE 7
#define DATETIME_SIZE 20
#define FULLTIME_SIZE 12
#define PASSWORD_SIZE 9
#define TIMEPASSED_SIZE 6
#define MAX_FILE_SIZE 10485760
#define FILE_SIZE_STR 10

#define TIMEOUT 5
#define PORT_SIZE 8


void set_mode_verbose();

int get_mode_verbose();

#endif