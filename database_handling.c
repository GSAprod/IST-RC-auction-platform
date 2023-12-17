#include "database_handling.h"

void InitDatabase() {
	struct stat filestat;
	int ret_stat = stat("USERS", &filestat);
	if (ret_stat == -1) {
		mkdir("USERS", 0777);
	}

	ret_stat = stat("AUCTIONS", &filestat);
	if (ret_stat == -1) {
		mkdir("AUCTIONS", 0777);
	}
}

int CreateUser(char * UID, char * password) {
	char fileName[PATHNAME_SIZE];

	//Create user directory
	sprintf(fileName, "USERS/%s", UID);
	if (checkAssetFile(fileName) == 0) {
		if (mkdir(fileName, 0777) == -1) {
			return -1;
		}
	}
	memset(fileName, 0, sizeof(fileName));

	//Save password
	sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);

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
	sprintf(fileName, "USERS/%s/%s_login.txt", UID, UID);

	file = fopen(fileName, "w");
	if (file == NULL) {
		return -1;
	}
	fclose(file);

	memset(fileName, 0, sizeof(fileName));

	//Create hosted directory
	sprintf(fileName, "USERS/%s/HOSTED", UID);
	if (checkAssetFile(fileName) == 0) {
		if (mkdir(fileName, 0777) == -1) {
			return -1;
		}
	}

	memset(fileName, 0, sizeof(fileName));

	//Create bid directory
	sprintf(fileName, "USERS/%s/BIDDED", UID);
	if (checkAssetFile(fileName) == 0) {
		if (mkdir(fileName, 0777) == -1) {
			return -1;
		}
	}

	return 0;
}

int Login(char * UID, char * password) {
	char fileName[PATHNAME_SIZE];

	sprintf(fileName, "USERS/%s", UID);
	if (checkAssetFile(fileName)) {

		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);

		if (checkAssetFile(fileName) > 0) {

			char password_db[PASSWORD_SIZE];
			memset(password_db, 0, sizeof(password_db));
			FILE * file = fopen(fileName, "r");
			if (file == NULL) {
				if (get_mode_verbose()) printf("Error opening file\n");
				return -2;
			}

			if (fread(password_db, 1, strlen(password), file) <= 0) {
				if (get_mode_verbose()) printf("Error reading from pass file\n");
				return -2;
			}

			fclose(file);

			if (strncmp(password_db, password, 8)) {
				if (get_mode_verbose()) printf("Wrong password\n");
				return -1;
			}
		} 
		else {
			// CREATES PASSWORD FILE
			int ret = CreateUser(UID, password);
			if (ret == 0) {
				return 0;
			} else {
				if (get_mode_verbose()) printf("Error creating user %s\n", UID);
				return -2;
			}
		}

		// CREATES LOGIN FILE
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "USERS/%s/%s_login.txt", UID, UID);

		if (checkAssetFile(fileName)) {
			return 1;
		} else {
			FILE * file = fopen(fileName, "w");
			if (file == NULL) {
				if (get_mode_verbose()) printf("Error opening file\n");
				return -2;
			}
			fclose(file);
		}
		return 1;
	} else {
		int ret = CreateUser(UID, password);
		if (ret == 0) {
			return 0;
		} else {
			if (get_mode_verbose()) printf("Error creating user %s\n", UID);
			return -2;
		}
	}
}


int CheckUserLogged(char * UID, char * password) {
	char fileName[PATHNAME_SIZE];

	memset(fileName, 0, sizeof(fileName));
	
	sprintf(fileName, "USERS/%s/%s_login.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s is not logged in\n", UID);
		return 0;
	}

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s is not registered\n", UID);
		return -1;
	}

	int ret = CheckUserPassword(UID, password);

	switch (ret) {
		case 0:
			if (get_mode_verbose()) printf("Wrong password\n");
			return -2;
		case 1:
			return 1;
		default:
			if (get_mode_verbose()) printf("Error checking password\n");
			return -2;
	}
}

int CheckUserPassword(char * UID, char * password) {
	char fileName[PATHNAME_SIZE];

	sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);
	if (checkAssetFile(fileName) <= 0) {
		if (get_mode_verbose()) printf("User %s has no password defined\n", UID);
		return -1;
	}

	char password_db[PASSWORD_SIZE];
	memset(password_db, 0, sizeof(password_db));
	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error opening file\n");
		return -1;
	}

	if (fread(password_db, 1, strlen(password), file) <= 0) {
		if (get_mode_verbose()) printf("Error reading from pass file\n");
		return -1;
	}

	fclose(file);

	if (strncmp(password_db, password, 8)) {
		if (get_mode_verbose()) printf("Wrong password\n");
		return 0;
	}

	return 1;
}

int Logout(char * UID) {
	char fileName[PATHNAME_SIZE];

	sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s has no password defined\n", UID);
		return -1;
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "USERS/%s/%s_login.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s is not logged in\n", UID);
		return 0;
	}

	unlink(fileName);
	return 1;
}

int Unregister(char * UID) {
	char fileName[PATHNAME_SIZE];

	sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s has no password defined\n", UID);
		return -1;
	}
	
	unlink(fileName);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "USERS/%s/%s_login.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s isn't logged in\n", UID);
		return -2;
	}

	unlink(fileName);

	return 0;
}

int CreateAuction(char * UID, char*name, char * asset_fname, char * start_value, char * time_active, char * start_datetime, time_t start_fulltime, char * file_size, int socket_fd) {

	char fileName[PATHNAME_SIZE];

	//Generate AID
	char AID[AID_SIZE];
	memset(AID, 0, sizeof(AID));
	int i = 0;
	for (i = 1; i < 1000; i++) {
		sprintf(fileName, "AUCTIONS/%03d", i);
		if (checkAssetFile(fileName) == 0) {
			sprintf(AID, "%03d", i);
			if (get_mode_verbose()) printf("AID: %s\n", AID);
			break;
		}
		if (i == 999) {
			if (get_mode_verbose()) printf("Error generating AID\n");
			return -1;
		}
	}

	//Create auction directory
	sprintf(fileName, "AUCTIONS/%s", AID);
	if (mkdir(fileName, 0777) == -1) {
		if (get_mode_verbose()) printf("Error creating auction directory\n");
		return -1;
	}
	memset(fileName, 0, sizeof(fileName));

	//Start_AID file
	sprintf(fileName, "AUCTIONS/%s/START_%s.txt", AID, AID);
	FILE * file = fopen(fileName, "w");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error creating Start_AID file\n");
		return -1;
	}

	char start_info[FILE_INFO_SIZE];

	//Write to file info about the auction
	sprintf(start_info, "%s %s %s %s %s %s %ld", UID, name, asset_fname, start_value, time_active, start_datetime, start_fulltime);
	
	size_t written = fwrite(start_info, 1, strlen(start_info), file);
	if (written != strlen(start_info)) {
		if (get_mode_verbose()) printf("Error writing to Start_AID file\n");
		fclose(file);
		return -1;
	}

	fclose(file);

	memset(fileName, 0, sizeof(fileName));

	//Asset directory
	sprintf(fileName, "AUCTIONS/%s/ASSET", AID);
	if (mkdir(fileName, 0777) == -1) {
		if (get_mode_verbose()) printf("Error creating asset directory\n");
		return -1;
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "AUCTIONS/%s/ASSET/%s", AID, asset_fname);
	long fsize = atol(file_size);
	ServerReceiveFile(fileName, fsize, socket_fd);

	char test[2];
	server_tcp_receive(socket_fd, test, 1);
	test[1] = '\0';
	if (strcmp(test, "\n")) {
		if (get_mode_verbose()) printf("Invalid server response (newline missing)\n");
		return -1;
	}

	memset(fileName, 0, sizeof(fileName));

	//Bids directory
	sprintf(fileName, "AUCTIONS/%s/BIDS", AID);
	if (mkdir(fileName, 0777) == -1) {
		if (get_mode_verbose()) printf("Error creating bids directory\n");
		return -1;
	}

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "USERS/%s/HOSTED/%s.txt", UID, AID);
	file = fopen(fileName, "w");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error creating hosted file\n");
		return -1;
	}
	fclose(file);
	
	return i;
}

int CloseAuction(char * AID, char * UID) {

	char fileName[PATHNAME_SIZE];

	sprintf(fileName, "AUCTIONS/%s/START_%s.txt", AID, AID);
	if (checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("Auction %s exists\n", AID);
	} else {
		if (get_mode_verbose()) printf("Auction %s does not exist\n", AID);
		return -1; //* -1 = auction does not exist
	}

	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error opening file\n");
		return -4;
	}

	char auction_info[FILE_INFO_SIZE];

	if (fread(auction_info, 1, FILE_INFO_SIZE, file) <= 0) {
		if (get_mode_verbose()) printf("Error reading from auction file\n");
		return -4;
	}
	fclose(file);

	char db_UID[UID_SIZE];

	char start_fulltime[FULLTIME_SIZE];
	sscanf(auction_info, "%s %*s %*s %*s %*s %*s %*s %s",db_UID, start_fulltime);

	if (strcmp(db_UID, UID)) {
		if (get_mode_verbose()) printf("User %s is not the auction host\n", UID);
		return -2; //* -2 = user is not the auction host
	}

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "AUCTIONS/%s/END_%s.txt", AID, AID);

	if (checkIfAuctionEnded(AID) == 1) {
		if (get_mode_verbose()) printf("Auction %s already ended\n", AID);
		return -3; //* -3 = auction already ended
	}

	//end auction
  file = fopen(fileName, "w");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error creating end file\n");
		return -4;
	}

	time_t now; // seconds since 1970-01-01 00:00:00
	char end_datetime_str[DATETIME_SIZE]; // yyyy-mm-dd hh:mm:ss
	time_t time_passed; // seconds since auction start

	memset(end_datetime_str, 0, sizeof(end_datetime_str));

	time(&now);

	timeToString(now, end_datetime_str); 

	time_passed = now - atol(start_fulltime);

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "%s %ld", end_datetime_str, time_passed);

	size_t written = fwrite(fileName, 1, strlen(fileName), file);
	if (written != strlen(fileName)) {
		if (get_mode_verbose()) printf("Error writing to end file\n");
		fclose(file);
		return -4;
	}

	fclose(file);

	return 0;
}

int GetHighestBid(char * AID) {
	struct dirent **filelist;
	char curPath[PATHNAME_SIZE];
	int n_entries, name_len;

	// Check if the auction exists
	sprintf(curPath, "AUCTIONS/%s/BIDS/", AID);
	if (checkAssetFile(curPath)) {
		if (get_mode_verbose()) printf("Auction %s exists\n", AID);
	} else {
		if (get_mode_verbose()) printf("Auction %s does not exist\n", AID);
		return -1; //* -1 = auction does not exist
	}

	// Start scanning the bids directory until a file is found
	n_entries = scandir(curPath, &filelist, 0, alphasort);
	if (n_entries <= 0) {
		if (get_mode_verbose()) printf("No files in asset folder of auction %s", AID);
		return 0;
	}

	int entries = n_entries;
	long value = 0;

	for (int i = n_entries -1; i >= 0; i--) {
		name_len=strlen(filelist[i]->d_name);
		if (name_len > 2) {
			sscanf(filelist[i]->d_name, "%ld.%*s", &value);
			break;
		}

		printf("%s\n", filelist[i]->d_name);
	}	

	for (int i = 0; i < entries; i++) {
		free(filelist[i]);
	}

	free(filelist);

	return value;
}

int Bid(char * AID, char * UID, char * value) {
	char fileName[PATHNAME_SIZE];

	time_t now;
	time(&now);

	char datetime[DATETIME_SIZE];

	memset(datetime, 0, sizeof(datetime));

	timeToString(now, datetime);

	char buffer[MEDIUM_BUFFER];

	memset(buffer, 0, sizeof(buffer));
	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "AUCTIONS/%s/START_%s.txt", AID, AID);

	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error creating bid file\n");
		return -5;
	}
	if (fread(buffer, 1, sizeof(buffer), file) <= 0) {
		if (get_mode_verbose()) printf("Error reading from start file\n");
		return -5;
	}

	fclose(file);

	// READ START auction FILE
	char UID_DB[UID_SIZE];
	memset(UID_DB, 0, sizeof(UID_DB));

	int start_value;
	int time_active;
	long fulltime;

	sscanf(buffer, "%s %*s %*s %d %d %19[^\n] %ld", UID_DB, &start_value, &time_active, datetime, &fulltime);

	int checkIfEnded = checkIfAuctionEnded(AID);
	if (checkIfEnded == 1) {
		if (get_mode_verbose()) printf("Auction %s ended\n", AID);
		return -1; //* -1 = auction already ended
	}

	if (CheckUserLogged(UID, "") == 0) {
		if (get_mode_verbose()) printf("User %s is not logged in\n", UID);
		return -4; //* -4 = user is not logged in
	}

	int highestBid = GetHighestBid(AID);

	int numeric_value = atoi(value);

	if (highestBid == 0) {
		if (numeric_value < start_value) {
			if (get_mode_verbose()) printf("Bid value is lower than the start value\n");
			return -2; //* -2 = bid value is lower than the start value
		}
	}


  if (numeric_value <= highestBid) {
  	if (get_mode_verbose()) printf("Bid value is lower than the highest bid\n");
    return -2; //* -2 = bid value is lower than the highest bid
  }

	if (!strcmp(UID_DB, UID)) {
		if (get_mode_verbose()) printf("User %s is the auction host\n", UID);
		return -3; //* -3 = user is the auction host
	}

	char bid_info[FILE_INFO_SIZE];

	time(&now);

	//Write to file info about the bid
	sprintf(bid_info, "%s %d %s %ld", UID, numeric_value, datetime, now - fulltime);


	//Create bid file
	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "AUCTIONS/%s/BIDS/%06d.txt", AID, numeric_value);
	file = fopen(fileName, "w");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error creating bid file\n");
		return -1;
	}
	
	size_t written = fwrite(bid_info, 1, strlen(bid_info), file);
	if (written != strlen(bid_info)) {
		if (get_mode_verbose()) printf("Error writing to bid file\n");
		fclose(file);
		return -5;
	}

	fclose(file);

	memset(fileName, 0, sizeof(fileName));

	//Add bid to user's bidded directory
	sprintf(fileName, "USERS/%s/BIDDED/%s.txt", UID, AID);
	file = fopen(fileName, "w");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error creating bid file\n");
		return -5;
	}
	fclose(file);


	return 0;
}

int ShowAsset(char * AID, int socket_fd) {
	struct dirent **filelist;
	struct stat filestat;
	char curPath[PATHNAME_SIZE], fileName[PATHNAME_SIZE], buffer[LARGE_BUFFER];
	int n_entries, name_len, fd;
	long file_size;

	// Check if the auction exists
	sprintf(curPath, "AUCTIONS/%s/ASSET/", AID);
	if (checkAssetFile(curPath)) {
		if (get_mode_verbose()) printf("Auction %s exists\n", AID);
	} else {
		if (get_mode_verbose()) printf("Auction %s does not exist\n", AID);
		return -1; //* -1 = auction does not exist
	}

	// Start scanning a directory unil a file is found
	n_entries = scandir(curPath, &filelist, 0, alphasort);
	if (n_entries <= 0) {
		if (get_mode_verbose()) printf("No files in asset folder of auction %s", AID);
		return -1;
	}

	while(n_entries--) {
		name_len=strlen(filelist[n_entries]->d_name);
		if (name_len > 2) break;	//Ignore '.' and '..'
	}

	// If a file is not found
	if (n_entries < 0) {
		if (get_mode_verbose()) printf("Error fetching file from auction %s", AID);
		free(filelist);
		return -1;
	}

	// Get the name of the file
	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, filelist[n_entries]->d_name);
	strcat(curPath, fileName);
	free(filelist);

	if (get_mode_verbose()) printf("Found file %s\n", curPath);

	// Get the length of the file
	stat(curPath, &filestat);
	file_size = filestat.st_size;
	if (file_size == 0) {
		if (get_mode_verbose()) printf("File is empty.\n");
		return -1;
	}

	// Open the file to start sending it
	fd = open(curPath, O_RDONLY);
	if (fd==-1) {
		if (get_mode_verbose()) printf("Couldn't open auction asset (id: %s)\n", AID);
		return -1;
	}

	sprintf(buffer, "RSA OK %s %ld ", fileName, file_size);
	if (get_mode_verbose()) printf("%s\n", buffer);
	server_tcp_send(socket_fd, buffer, strlen(buffer));

	serverSendFile(fd, file_size, socket_fd);

	server_tcp_send(socket_fd, "\n", 1);

	close(fd);

	return 0;
}

//Function to convert from time_t to string
void timeToString(time_t time, char * time_string) {
	struct tm * timeinfo;

	timeinfo = gmtime(&time);

	sprintf(time_string, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	return;
}

int LoadBid(char * pathname, struct BIDLIST * bid) {
	if (get_mode_verbose()) printf("Loading bid from file %s\n", pathname);
	
	FILE * file = fopen(pathname, "r");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error opening file\n");
		return -1;
	}

	char bid_info[FILE_INFO_SIZE];

	memset(bid_info, 0, sizeof(bid_info));

	if (fread(bid_info, 1, FILE_INFO_SIZE, file) <= 0) {
		if (get_mode_verbose()) printf("Error reading from bid file\n");
		return -1;
	}

	char fulltime[TIMEPASSED_SIZE];

	sscanf(bid_info, "%s %s %19[^\n] %s", bid->UID, bid->value, bid->datetime, fulltime);

	sprintf(bid->fulltime, "%d", atoi(fulltime));

	fclose(file);

	return 0;
}

int GetBidList(char * AID, struct BIDLIST ** bidlist) {

	struct dirent **filelist;
	int n_entries, n_bids, len;
	char dirname[SMALL_BUFFER];
	char pathname[PATHNAME_SIZE];

	sprintf(dirname, "AUCTIONS/%s/BIDS", AID);
	if (get_mode_verbose()) printf("Directory name: %s\n", dirname);

	n_entries = scandir(dirname, &filelist, 0, alphasort);

	if (get_mode_verbose()) printf("Number of entries: %d\n", n_entries);

	if (n_entries <= 0) {
		if (get_mode_verbose()) printf("Error scanning directory\n");
		return -1;
	}

	n_bids = 0;

	n_entries = n_entries >= 50 ? 50 : n_entries;

	*bidlist = (struct BIDLIST*)malloc((n_entries) * sizeof(struct BIDLIST));

	int i = 0;
	while (i < n_entries) {
		len = strlen(filelist[i]->d_name);
		if (get_mode_verbose()) printf("File name: %s\n", filelist[i]->d_name);
		if (len == 10) {
			sprintf(pathname, "AUCTIONS/%s/BIDS/%s", AID, filelist[i]->d_name);
			if (get_mode_verbose()) printf("Pathname: %s\n", pathname);
			if (LoadBid(pathname, &(*bidlist)[n_bids]) == 0) {
				strcpy((*bidlist)[n_bids].AID, AID);
				++n_bids;
			} else {
				if (get_mode_verbose()) printf("List bids: Last bid failed to open\n");
				return -1;
			}
		}
		i++;
	}

	for (int i = 0; i < n_entries; i++) {
		free(filelist[i]);
	}

	free(filelist);
	return n_bids;
}

int checkIfAuctionEnded(char * AID) {
	if (get_mode_verbose()) printf("Checking if auction %s ended\n", AID);

	char fileName[PATHNAME_SIZE];
	sprintf(fileName, "AUCTIONS/%s", AID);

	if (checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("Auction %s exists\n", AID);
	} else {
		if (get_mode_verbose()) printf("Auction %s does not exist\n", AID);
		return -1;
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "AUCTIONS/%s/END_%s.txt", AID, AID);

	if (checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("Auction %s ended\n", AID);
		return 1;
	} else {
		if (get_mode_verbose()) printf("Checking file time:\n");
		
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "AUCTIONS/%s/START_%s.txt", AID, AID);

		FILE * file = fopen(fileName, "r");
		if (file == NULL) {
			if (get_mode_verbose()) printf("Error opening file\n");
			return -1;
		}

		char file_data[MEDIUM_BUFFER];


		if (fread(file_data, 1, sizeof(file_data), file) <= 0) {
			if (get_mode_verbose()) printf("Error reading from start file\n");
			return -1;
		}

		int time_active = 0;
		long start_date = 0;

		sscanf(file_data, "%*s %*s %*s %*s %d %*d-%*d-%*d %*d:%*d:%*d %ld", &time_active, &start_date);

		fclose(file);

		printf("Time active: %d\n", time_active);

		time_t now;
		time_t end_date = start_date + time_active;

		

		time(&now);

		if (end_date <= now) {
			if (get_mode_verbose()) printf("Auction %s ended\n", AID);
			memset(fileName, 0, sizeof(fileName));
			sprintf(fileName, "AUCTIONS/%s/END_%s.txt", AID, AID);
			file = fopen(fileName, "w");
			if (file == NULL) {
				if (get_mode_verbose()) printf("Error creating end file\n");
				return -1;
			}

			char end_datetime[DATETIME_SIZE];
			char time_to_end[FULLTIME_SIZE];

			memset(end_datetime, 0, sizeof(end_datetime));

			timeToString(end_date, end_datetime);

			size_t written = fwrite(end_datetime, 1, strlen(end_datetime), file);
			if (written != strlen(end_datetime)) {
				if (get_mode_verbose()) printf("Error writing to end file\n");
				fclose(file);
				return -1;
			}

			written = fwrite(" ", 1, 1, file);
			if (written != 1) {
				if (get_mode_verbose()) printf("Error writing to end file\n");
				fclose(file);
				return -1;
			}

			sprintf(time_to_end, "%ld", end_date - start_date);

			written = fwrite(time_to_end, 1, strlen(time_to_end), file);
			if (written != strlen(time_to_end)) {
				if (get_mode_verbose()) printf("Error writing to end file\n");
				fclose(file);
				return -1;
			}

			fclose(file);
			return 1;
		} else {
			if (get_mode_verbose()) printf("Auction %s did not end\n", AID);
			return 0;
		}
	}
}

int GetAuctionInfo(char * AID, char * message_ptr) {
	char fileName[PATHNAME_SIZE];

	sprintf(fileName, "AUCTIONS/%s/START_%s.txt", AID, AID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("Auction %s does not exist\n", AID);
		return -1;
	}

	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		if (get_mode_verbose()) printf("Error opening auction start file\n");
		return -1;
	}

	char auction_info[FILE_INFO_SIZE];

	memset(auction_info, 0, sizeof(auction_info));

	if (fread(auction_info, 1, sizeof(auction_info), file) <= 0) {
		if (get_mode_verbose()) printf("Error reading from auction file\n");
		return -1;
	}

	fclose(file);

	char UID[UID_SIZE];
	char name[FILENAME_SIZE];
	char asset_fname[FILENAME_SIZE];
	char start_value[VALUE_SIZE];
	int time_active;
	char start_datetime[DATETIME_SIZE];
	long fulltime;

	sscanf(auction_info, "%s %s %s %s %d %19[^\n] %ld", UID, name, asset_fname, start_value, &time_active, start_datetime, &fulltime);

	if (get_mode_verbose()) printf("UID: %s\nName: %s\nAsset filename: %s\nStart value: %s\nTime active: %d\nStart datetime: %s\n", UID, name, asset_fname, start_value, time_active, start_datetime);

	sprintf(message_ptr, "%s %s %s %s %s %d", UID, name, asset_fname, start_value, start_datetime, time_active);

	message_ptr = message_ptr + strlen(message_ptr);

	struct BIDLIST * bidlist = NULL;

	int n_bids = GetBidList(AID, &bidlist);
	if (get_mode_verbose()) printf("Number of bids: %d\n", n_bids);

	for (int i = 0; i < n_bids; i++) {
		if (get_mode_verbose()) printf("UID: %s\nValue: %s\nDatetime: %s\nFulltime: %s\n", bidlist[i].UID, bidlist[i].value, bidlist[i].datetime, bidlist[i].fulltime);
		sprintf(message_ptr, " B %s %s %s %s", bidlist[i].UID, bidlist[i].value, bidlist[i].datetime, bidlist[i].fulltime);
		message_ptr = message_ptr + strlen(message_ptr);
	}

	free(bidlist);

	int checkIfEnded = checkIfAuctionEnded(AID);

	if (checkIfEnded == 1) {
		if (get_mode_verbose()) printf("Auction %s ended\n", AID);
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "AUCTIONS/%s/END_%s.txt", AID, AID);
		file = fopen(fileName, "r");
		if (file == NULL) {
			if (get_mode_verbose()) printf("Error opening file\n");
			return -1;
		}
		char end_info[FILE_INFO_SIZE];
		if (fread(end_info, 1, sizeof(end_info), file) <= 0) {
			if (get_mode_verbose()) printf("Error reading from end file\n");
			return -1;
		}

		fclose(file);

		char end_datetime[DATETIME_SIZE];
		char time_passed[FULLTIME_SIZE];

		memset(end_datetime, 0, sizeof(end_datetime));
		memset(time_passed, 0, sizeof(time_passed));

		sscanf(end_info, "%19[^\n] %s", end_datetime, time_passed);

		sprintf(message_ptr, " E %s %s", end_datetime, time_passed);
		message_ptr = message_ptr + strlen(message_ptr);
		
	} else if (checkIfEnded == 0) {
		if (get_mode_verbose()) printf("Auction %s did not end\n", AID);
	} else {
		if (get_mode_verbose()) printf("Error checking if auction %s ended\n", AID);
	}

	return 0;
}

int GetAuctionsList(struct AUCTIONLIST ** auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[SMALL_BUFFER];
	char pathname[PATHNAME_SIZE];

	sprintf(dirname, "AUCTIONS");

	n_entries = scandir(dirname, &filelist, 0, alphasort);
	if (get_mode_verbose()) printf("Number of entries: %d\n", n_entries);

	if (n_entries <= 0) {
		if (get_mode_verbose()) printf("Error scanning directory\n");
		return -1;
	}

	n_auctions = 0;

	*auction_list = (struct AUCTIONLIST*)malloc((n_entries) * sizeof(struct AUCTIONLIST));

	int i = 0;

	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[i]->d_name);
		if (get_mode_verbose()) printf("File name: %s\n", filelist[i]->d_name);
		if (len == 3) {
			sprintf(pathname, "AUCTIONS/%s", filelist[i]->d_name);
			if (get_mode_verbose()) printf("Pathname: %s\n", pathname);
			if (checkAssetFile(pathname)) {
				strncpy((*auction_list)[n_auctions].AID, filelist[i]->d_name, 3);
				(*auction_list)[n_auctions].AID[3] = '\0';
				(*auction_list)[n_auctions].active = checkIfAuctionEnded((*auction_list)[n_auctions].AID) ? 0 : 1;
				++n_auctions;
			}
		}
		memset(pathname, 0, sizeof(pathname));
	}

	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[i]->d_name);
		if (len > 2)
			free(filelist[i]);
	}
	free(filelist);
	return n_auctions;
}

int GetAuctionsListByUser(char * UID, struct AUCTIONLIST ** auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[SMALL_BUFFER];
	char pathname[PATHNAME_SIZE];

	sprintf(dirname, "USERS/%s/HOSTED", UID);

	n_entries = scandir(dirname, &filelist, 0, alphasort);

	if (n_entries <= 0) {
		if (get_mode_verbose()) printf("Error scanning directory\n");
		return -1;
	}

	n_auctions = 0;

	*auction_list = (struct AUCTIONLIST*)malloc((n_entries) * sizeof(struct AUCTIONLIST));

	int i = 0;

	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[i]->d_name);
		if (get_mode_verbose()) printf("File name: %s\n", filelist[i]->d_name);
		if (len == 7) {
			sprintf(pathname, "USERS/%s/HOSTED/%s", UID, filelist[i]->d_name);
			if (get_mode_verbose()) printf("Pathname: %s\n", pathname);
			if (checkAssetFile(pathname)) {
				char AID [AID_SIZE];
				memset(AID, 0, sizeof(AID));
				sscanf(filelist[i]->d_name, "%3s.txt", AID);
				strncpy((*auction_list)[n_auctions].AID, AID, 3);
				(*auction_list)[n_auctions].AID[3] = '\0';
				(*auction_list)[n_auctions].active = checkIfAuctionEnded(AID) ? 0 : 1;
				++n_auctions;
			}
		}
		memset(pathname, 0, sizeof(pathname));
	}
	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[i]->d_name);
		if (len > 2)
			free(filelist[i]);
	}
	free(filelist);

	return n_auctions;
}

int GetAuctionsListByUserBidded(char * UID, struct AUCTIONLIST ** auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[SMALL_BUFFER];
	char pathname[PATHNAME_SIZE];

	sprintf(dirname, "USERS/%s/BIDDED", UID);

	n_entries = scandir(dirname, &filelist, 0, alphasort);

	if (n_entries <= 0) {
		if (get_mode_verbose()) printf("Error scanning directory\n");
		return -1; //* USER WAS NOT LOGGED IN
	}

	n_auctions = 0;

	*auction_list = (struct AUCTIONLIST*)malloc((n_entries) * sizeof(struct AUCTIONLIST));

	int i = 0;

	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[i]->d_name);
		if (get_mode_verbose()) printf("File name: %s\n", filelist[i]->d_name);
		if (len == 7) {
			sprintf(pathname, "USERS/%s/BIDDED/%s", UID, filelist[i]->d_name);
			if (get_mode_verbose()) printf("Pathname: %s\n", pathname);
			if (checkAssetFile(pathname)) {
				char AID[AID_SIZE];
				memset(AID, 0, sizeof(AID));
				sscanf(filelist[i]->d_name, "%3s.txt", AID);
				strncpy((*auction_list)[n_auctions].AID, AID, 3);
				(*auction_list)[n_auctions].AID[3] = '\0';
				(*auction_list)[n_auctions].active = checkIfAuctionEnded((*auction_list)[n_auctions].AID) ? 0 : 1;
				++n_auctions;
			}
		}
		memset(pathname, 0, sizeof(pathname));
	}

	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[i]->d_name);
		if (len > 2)
			free(filelist[i]);
	}
	
	free(filelist);
	return n_auctions;
}