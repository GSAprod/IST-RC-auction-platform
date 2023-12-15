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
	char fileName[256];

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
	char fileName[256];

	if (get_mode_verbose()) printf("Logging in user %s\n", UID);

	sprintf(fileName, "USERS/%s", UID);
	if (checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s exists\n", UID);

		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);

		if (checkAssetFile(fileName) > 0) {
			if (get_mode_verbose()) printf ("User has password defined\n");

			char password_db[8];
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
				if (get_mode_verbose()) printf("User %s created\n", UID);
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
			if (get_mode_verbose()) printf("User %s is logged in\n", UID);
			return 1;
		} else {
			if (get_mode_verbose()) printf("User %s is not logged in\n", UID);
			FILE * file = fopen(fileName, "w");
			if (file == NULL) {
				if (get_mode_verbose()) printf("Error opening file\n");
				return -2;
			}
			fclose(file);
		}
		return 1;
	} else {
		if (get_mode_verbose()) printf("User %s does not exist\n", UID);
		int ret = CreateUser(UID, password);
		if (ret == 0) {
			if (get_mode_verbose()) printf("User %s created\n", UID);
			return 0;
		} else {
			if (get_mode_verbose()) printf("Error creating user %s\n", UID);
			return -2;
		}
	}
}

int Logout(char * UID) {
	char fileName[256];

	if (get_mode_verbose()) printf("Logging out user %s\n", UID);

	sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s has no password defined\n", UID);
		return -1;
	}

	if (get_mode_verbose()) printf("User %s has password defined\n", UID);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "USERS/%s/%s_login.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (get_mode_verbose()) printf("User %s is not logged in\n", UID);
		return 0;
	}

	if (get_mode_verbose()) printf("User %s is logged in\n", UID);

	unlink(fileName);
	return 1;
}

int Unregister(char * UID) {
	char fileName[256];

	if (get_mode_verbose()) printf("Unregistering user %s\n", UID);

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

	if (get_mode_verbose()) printf("User %s is logged in\n", UID);
	unlink(fileName);

	return 0;
}

int CreateAuction(char * UID, char*name, char * asset_fname, char * start_value, char * time_active, char * start_datetime, time_t start_fulltime, char * file_size, int socket_fd) {
	if (get_mode_verbose()) printf("Creating auction\n");

	char fileName[256];

	//Generate AID
	char AID[4];
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

	char start_info[128];

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
	if (get_mode_verbose()) printf("File size: %ld\n", fsize);
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
	if (get_mode_verbose()) printf("Closing auction %s\n", AID);

	char fileName[256];

	//TODO: Make close auction function
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

	char auction_info[256];

	if (fread(auction_info, 1, 256, file) <= 0) {
		if (get_mode_verbose()) printf("Error reading from auction file\n");
		return -4;
	}
	fclose(file);

	char db_UID[6];

	char * token = strtok(auction_info, " ");
	strcpy(db_UID, token);

	if (strcmp(db_UID, UID)) {
		if (get_mode_verbose()) printf("User %s is not the auction host\n", UID);
		return -2; //* -2 = user is not the auction host
	}

	char start_fulltime[20];

	token = strtok(NULL, " "); //Name
	token = strtok(NULL, " "); //Asset filename
	token = strtok(NULL, " "); //Start value
	token = strtok(NULL, " "); //Time active
	token = strtok(NULL, " "); //Start date
	token = strtok(NULL, " "); //Start time
	token = strtok(NULL, " "); //Start fulltime (time_t)
	strcpy(start_fulltime, token);

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
	char end_datetime_str[20]; // yyyy-mm-dd hh:mm:ss
	time_t time_passed; // seconds since auction start

	memset(end_datetime_str, 0, sizeof(end_datetime_str));

	time(&now);

	timeToString(now, end_datetime_str); 

	time_passed = now - atol(start_fulltime);

	if (get_mode_verbose()) printf("End datetime: %s\n", end_datetime_str);

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

// TODO TEST THIS FUNCTION
int GetHighestBid(char * AID) {
	struct dirent **filelist;
	char curPath[256];
	int n_entries, name_len;

	if (get_mode_verbose()) printf("Getting highest bid from auction %s\n", AID);

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

	if (get_mode_verbose()) printf("Number of entries: %d\n", n_entries);

	while(--entries) {
		name_len=strlen(filelist[entries]->d_name);
		if (name_len > 2) {
			char * str_token = strtok(filelist[entries]->d_name, ".");
			value = atol(str_token);
			break;
		}

		printf("%s\n", filelist[entries]->d_name);
	}	

	for (int i = 0; i < entries; i++) {
		free(filelist[i]);
	}

	free(filelist);

	return value;
}

int Bid(char * AID, char * UID, char * value) {
	char fileName[256];

	if (get_mode_verbose()) printf("Bidding on auction %s\n", AID);

	if (get_mode_verbose()) printf("UID: %s\nValue: %s\n", UID, value);

	time_t now;
	time(&now);

	char datetime[20];
	char fulltime[20];

	memset(datetime, 0, sizeof(datetime));
	memset(fulltime, 0, sizeof(fulltime));

	timeToString(now, datetime);

	char buffer[128];

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
	char UID_DB[7];
	memset(UID_DB, 0, sizeof(UID_DB));

	char * token = strtok(buffer, " "); //UID
	strcpy(UID_DB, token);

	token = strtok(NULL, " "); //Name
	token = strtok(NULL, " "); //Asset filename
	token = strtok(NULL, " "); //Start value
	int start_value = atoi(token);
	
	char time_active[10];
	token = strtok(NULL, " "); //Time active
	strcpy(time_active, token);
	
	token = strtok(NULL, " "); //Start date

	token = strtok(NULL, " "); //Start hours

	token = strtok(NULL, " "); //Start fulltime (time_t)
	strcpy(fulltime, token);

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
  if (get_mode_verbose()) printf("Highest bid: %d\n", highestBid);
	if (get_mode_verbose()) printf("Bid value: %ld\n", atol(value));

	if (highestBid == 0) {
		if (atoi(value) < start_value) {
			if (get_mode_verbose()) printf("Bid value is lower than the start value\n");
			return -2; //* -2 = bid value is lower than the start value
		}
	}

	int numeric_value = atoi(value);

  if (numeric_value <= highestBid) {
  	if (get_mode_verbose()) printf("Bid value is lower than the highest bid\n");
    return -2; //* -2 = bid value is lower than the highest bid
  }

	if (!strcmp(UID_DB, UID)) {
		if (get_mode_verbose()) printf("User %s is the auction host\n", UID);
		return -3; //* -3 = user is the auction host
	}

	char * bid_info = malloc(strlen(UID) + strlen(value) + strlen(datetime) + strlen(fulltime) + 4);

	//Write to file info about the bid
	sprintf(bid_info, "%s %06d %s %s", UID, numeric_value, datetime, fulltime);

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

	free(bid_info);
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
	char curPath[256], fileName[256], buffer[512];
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

int CheckUserLogged(char * UID, char * password) {
	char fileName[256];
	if (get_mode_verbose()) printf("Checking if user %s is logged in\n", UID);

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
			if (get_mode_verbose()) printf("Correct password\n");
			return 1;
		default:
			if (get_mode_verbose()) printf("Error checking password\n");
			return -2;
	}
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

	char bid_info[64];

	memset(bid_info, 0, sizeof(bid_info));

	if (fread(bid_info, 1, 64, file) <= 0) {
		if (get_mode_verbose()) printf("Error reading from bid file\n");
		return -1;
	}

	char * token = strtok(bid_info, " ");
	strcpy(bid->UID, token);
	token = strtok(NULL, " ");
	strcpy(bid->value, token);
	token = token + strlen(token) + 1;
	strncpy(bid->datetime, token, 19);
	bid->datetime[19] = '\0';
	token += 20;
	strcpy(bid->fulltime, token); //? CHECK THIS THING HERE

	fclose(file);

	return 0;
}

int GetBidList(char * AID, struct BIDLIST ** bidlist) {

	struct dirent **filelist;
	int n_entries, n_bids, len;
	char dirname[32];
	char pathname[564]; //! Change this

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
	free(filelist);
	return n_bids;
}

int checkIfAuctionEnded(char * AID) {
	if (get_mode_verbose()) printf("Checking if auction %s ended\n", AID);

	char fileName[256];
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

		char file_data[256];

		char time_active[10];
		char start_date_str[10];

		memset(time_active, 0, sizeof(time_active));
		memset(start_date_str, 0, sizeof(start_date_str));


		if (fread(file_data, 1, sizeof(file_data), file) <= 0) {
			if (get_mode_verbose()) printf("Error reading from start file\n");
			return -1;
		}

		char * token = strtok(file_data, " "); //UID
		token = strtok(NULL, " "); //Name
		token = strtok(NULL, " "); //Asset filename
		token = strtok(NULL, " "); //Start value
		token = strtok(NULL, " "); //Time active
		strcpy(time_active, token);
		token = strtok(NULL, " "); //Start date
		token = strtok(NULL, " "); //Start time
		token = strtok(NULL, " "); //Start fulltime (time_t)
		strcpy(start_date_str, token);

		fclose(file);

		time_t now;
		time_t start_date = atol(start_date_str);
		time_t end_date = start_date + atoi(time_active);

		

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

			char end_datetime[20];
			char time_to_end[16];

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
	char fileName[256];

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

	char auction_info[256];

	memset(auction_info, 0, sizeof(auction_info));

	if (fread(auction_info, 1, 256, file) <= 0) {
		if (get_mode_verbose()) printf("Error reading from auction file\n");
		return -1;
	}

	fclose(file);

	char UID[7];
	char name[32];
	char asset_fname[32];
	char start_value[16];
	char time_active[16];
	char start_datetime[20];
	char fulltime[16];

	char * token = strtok(auction_info, " ");
	strcpy(UID, token);
	
	token = strtok(NULL, " ");
	strcpy(name, token);

	token = strtok(NULL, " ");
	strcpy(asset_fname, token);

	token = strtok(NULL, " ");
	strcpy(start_value, token);

	token = strtok(NULL, " ");
	strcpy(time_active, token);

	token = token + strlen(token) + 1;
	strncpy(start_datetime, token, 19);
	start_datetime[19] = '\0';

	token += 20;
	strcpy(fulltime, token);

	if (get_mode_verbose()) printf("UID: %s\nName: %s\nAsset filename: %s\nStart value: %s\nTime active: %s\nStart datetime: %s\n", UID, name, asset_fname, start_value, time_active, start_datetime);

	sprintf(message_ptr, "%s %s %s %s %s %s", UID, name, asset_fname, start_value, start_datetime, time_active);

	// TODO Fix
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
		char end_info[256];
		if (fread(end_info, 1, 256, file) <= 0) {
			if (get_mode_verbose()) printf("Error reading from end file\n");
			return -1;
		}

		fclose(file);

		char end_datetime[20];
		char time_passed[16];

		memset(end_datetime, 0, sizeof(end_datetime));
		memset(time_passed, 0, sizeof(time_passed));

		token = end_info;
		strncpy(end_datetime, token, 19);

		token = token + 20;
		strcpy(time_passed, token);

		sprintf(message_ptr, " E %s %s", end_datetime, time_passed);
		message_ptr = message_ptr + strlen(message_ptr);
		
	} else if (checkIfEnded == 0) {
		if (get_mode_verbose()) printf("Auction %s did not end\n", AID);
	} else {
		if (get_mode_verbose()) printf("Error checking if auction %s ended\n", AID);
	}

	*message_ptr = '\n';

	return 0;
}

int GetAuctionsList(struct AUCTIONLIST ** auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[32];
	char pathname[512];

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
		free(filelist[n_entries]);
	}
	free(filelist);
	return n_auctions;
}

int GetAuctionsListByUser(char * UID, struct AUCTIONLIST ** auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[32];
	char pathname[512];

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
				char AID [4];
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
		free(filelist[n_entries]);
	}
	free(filelist);


	return n_auctions;
}

int GetAuctionsListByUserBidded(char * UID, struct AUCTIONLIST ** auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[32];
	char pathname[300];

	sprintf(dirname, "USERS/%s/BIDDED", UID);

	n_entries = scandir(dirname, &filelist, 0, alphasort);

	if (n_entries <= 0) {
		if (get_mode_verbose()) printf("Error scanning directory\n");
		return -1; //! USER WAS NOT LOGGED IN
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
				char AID [4];
				memset(AID, 0, sizeof(AID));
				sscanf(filelist[i]->d_name, "%3s.txt", AID);
				strncpy((*auction_list)[n_auctions].AID, AID, 3);
				(*auction_list)[n_auctions].AID[3] = '\0';
				(*auction_list)[n_auctions].active = checkIfAuctionEnded((*auction_list)[n_auctions].AID) ? 0 : 1;
				++n_auctions;
			}
			free(filelist[n_entries]);
		}
		memset(pathname, 0, sizeof(pathname));
	}
	
	
	free(filelist);
	return n_auctions;
}

int CheckUserPassword(char * UID, char * password) {
	char fileName[256];

	sprintf(fileName, "USERS/%s/%s_pass.txt", UID, UID);
	if (checkAssetFile(fileName) <= 0) {
		if (get_mode_verbose()) printf("User %s has no password defined\n", UID);
		return -1;
	}
	
	if (get_mode_verbose()) printf("User %s has password defined\n", UID);

	char password_db[8];
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

	if (get_mode_verbose()) printf("\n%s : %s \n", password_db, password);

	if (strncmp(password_db, password, 8)) {
		if (get_mode_verbose()) printf("Wrong password\n");
		return 0;
	}

	return 1;
}