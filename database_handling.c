#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "file_handling.h"

int DEBUG = 1; 

void initDatabase() {
	struct stat filestat;
	int ret_stat = stat("ASDIR", &filestat);
	if (ret_stat == -1) {
		mkdir("ASDIR", 0777);
		mkdir("ASDIR/USERS", 0777);
		mkdir("ASDIR/AUCTIONS", 0777);
	}
}

int createUser(char * UID, char * password) {
	char fileName[256];

	//Create user directory
	sprintf(fileName, "ASDIR/USERS/%s", UID);
	if (mkdir(fileName, 0777) == -1) {
		return -1;
	}
	memset(fileName, 0, sizeof(fileName));

	//Save password
	sprintf(fileName, "ASDIR/USERS/%s/%s_pass.txt", UID, UID);

	FILE * file = fopen(fileName, "w");
	if (file == NULL) {
		return -1;
	}

	size_t written = fwrite(password, 1, strlen(password), file);
	if (written != strlen(password)) {
		fclose(file);
		return -1;
	}
	fclose(file);

	memset(fileName, 0, sizeof(fileName));

	//Create login file
	sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);

	memset(fileName, 0, sizeof(fileName));

	//Create hosted directory
	sprintf(fileName, "ASDIR/USERS/%s/HOSTED", UID);
	if (mkdir(fileName, 0777) == -1) {
		return -1;
	}

	memset(fileName, 0, sizeof(fileName));

	//Create bid directory
	sprintf(fileName, "ASDIR/USERS/%s/BIDDED", UID);
	if (mkdir(fileName, 0777) == -1) {
		return -1;
	}

	return 0;
}

int login(char * UID, char * password) {
	char fileName[256];

	if (DEBUG) printf("Logging in user %s\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s exists\n", UID);

		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "ASDIR/USERS/%s/%s_pass.txt", UID, UID);

		if (checkAssetFile(fileName) > 0) {
			if (DEBUG) printf ("User has password defined\n");

			char password_db[8];
			FILE * file = fopen(fileName, "r");
			if (file == NULL) {
				if (DEBUG) printf("Error opening file\n");
				return -1;
			}

			if (fread(password_db, 1, strlen(password_db), file) < 0) {
				if (DEBUG) printf("Error reading from pass file\n");
				return -1;
			}

			fclose(file);

			if (!strncmp(password_db, password, 8)) {
				if (DEBUG) printf("Wrong password\n");
				return -1;
			}
		}

		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);

		if (checkAssetFile(fileName)) {
			if (DEBUG) printf("User %s is logged in\n", UID);
			return -2;
		} else {
			if (DEBUG) printf("User %s is not logged in\n", UID);
			FILE * file = fopen(fileName, "w");
			if (file == NULL) {
				if (DEBUG) printf("Error opening file\n");
				return -1;
			}
			fclose(file);
		}
		return 1;
	} else {
		if (createUser(UID, password)) {
			//TODO: handle create user errors
			return -1;
		}
	}
}

int logout(char * UID) {
	char fileName[256];

	if (DEBUG) printf("Logging out user %s\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s is logged in\n", UID);
		remove(fileName);
		return 1;
	} else {
		if (DEBUG) printf("User %s is not logged in\n", UID);
		return -1;
	}
}

int unregister(char * UID) {
	char fileName[256];

	if (DEBUG) printf("Unregistering user %s\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s exists\n", UID);
		return -1;
	} else {
		if (DEBUG) printf("User %s is not logged in\n", UID);
		remove(fileName);
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/USERS/%s/%s_pass.txt", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s has password defined\n", UID);
		remove(fileName);
	} else {
		if (DEBUG) printf("User %s has no password defined\n", UID);
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/USERS/%s/HOSTED", UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s has hosted auctions\n", UID);
		remove(fileName);
	} else {
		if (DEBUG) printf("User %s has no hosted auctions\n", UID);
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/USERS/%s/BIDDED", UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s has bidded auctions\n", UID);
		remove(fileName);
	} else {
		if (DEBUG) printf("User %s has no bidded auctions\n", UID);
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/USERS/%s", UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s exists\n", UID);
		remove(fileName);
	} else {
		if (DEBUG) printf("User %s does not exist\n", UID);
	}

	return 0;
}

int createAuction(char * AID, char * UID, char*name, char * asset_fname, char * start_value, char * time_active, char * start_datetime, char * start_fulltime) {
	if (DEBUG) printf("Creating auction %s\n", AID);

	char fileName[256];

	//Create auction directory
	sprintf(fileName, "ASDIR/AUCTIONS/%s", AID);
	if (mkdir(fileName, 0777) == -1) {
		if (DEBUG) printf("Error creating auction directory\n");
		return -1;
	}
	memset(fileName, 0, sizeof(fileName));

	//Start_AID file
	sprintf(fileName, "ASDIR/AUCTIONS/%s/Start_%s.txt", AID, AID);
	FILE * file = fopen(fileName, "w");
	if (file == NULL) {
		if (DEBUG) printf("Error creating Start_AID file\n");
		return -1;
	}

	char * start_info = malloc(strlen(UID) + strlen(name) + strlen(asset_fname) + strlen(start_value) + strlen(time_active) + strlen(start_datetime) + strlen(start_fulltime) + 7);

	//Write to file info about the auction
	sprintf(start_info, "%s %s %s %s %s %s %s", UID, name, asset_fname, start_value, time_active, start_datetime, start_fulltime);
	
	size_t written = fwrite(start_info, 1, strlen(start_info), file);
	if (written != strlen(start_info)) {
		if (DEBUG) printf("Error writing to Start_AID file\n");
		fclose(file);
		return -1;
	}

	free(start_info);
	fclose(file);

	memset(fileName, 0, sizeof(fileName));

	//Asset directory
	sprintf(fileName, "ASDIR/AUCTIONS/%s/ASSET", AID);
	if (mkdir(fileName, 0777) == -1) {
		if (DEBUG) printf("Error creating asset directory\n");
		return -1;
	}
	//TODO: Create asset file (from TCP)

	memset(fileName, 0, sizeof(fileName));

	//Bids directory
	sprintf(fileName, "ASDIR/AUCTIONS/%s/BIDS", AID);
	if (mkdir(fileName, 0777) == -1) {
		if (DEBUG) printf("Error creating bids directory\n");
		return -1;
	}
	
	return 0;
}

int bid(char * AID, char * UID, char * value, char * datetime, char * fulltime) {
	if (DEBUG) printf("Bidding on auction %s\n", AID);

	char fileName[256];

	//Create bid file
	sprintf(fileName, "ASDIR/AUCTIONS/%s/BIDS/%s.txt", AID, AID, UID);
	FILE * file = fopen(fileName, "w");
	if (file == NULL) {
		if (DEBUG) printf("Error creating bid file\n");
		return -1;
	}

	char * bid_info = malloc(strlen(UID) + strlen(value) + strlen(datetime) + strlen(fulltime) + 4);

	//Write to file info about the bid
	sprintf(bid_info, "%s %s %s %s", UID, value, datetime, fulltime);
	
	size_t written = fwrite(bid_info, 1, strlen(bid_info), file);
	if (written != strlen(bid_info)) {
		if (DEBUG) printf("Error writing to bid file\n");
		fclose(file);
		return -1;
	}

	free(bid_info);
	fclose(file);

	return 0;
}

