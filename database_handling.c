#include "database_handling.h"

int DEBUG = 1;

void InitDatabase() {
	struct stat filestat;
	int ret_stat = stat("ASDIR", &filestat);
	if (ret_stat == -1) {
		mkdir("ASDIR", 0777);
		mkdir("ASDIR/USERS", 0777);
		mkdir("ASDIR/AUCTIONS", 0777);
	}
}

int CreateUser(char * UID, char * password) {
	char fileName[256];

	//Create user directory
	sprintf(fileName, "ASDIR/USERS/%s", UID);
	if (checkAssetFile(fileName) == 0) {
		if (mkdir(fileName, 0777) == -1) {
			return -1;
		}
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

	file = fopen(fileName, "w");
	if (file == NULL) {
		return -1;
	}
	fclose(file);

	memset(fileName, 0, sizeof(fileName));

	//Create hosted directory
	sprintf(fileName, "ASDIR/USERS/%s/HOSTED", UID);
	if (checkAssetFile(fileName) == 0) {
		if (mkdir(fileName, 0777) == -1) {
			return -1;
		}
	}

	memset(fileName, 0, sizeof(fileName));

	//Create bid directory
	sprintf(fileName, "ASDIR/USERS/%s/BIDDED", UID);
	if (checkAssetFile(fileName) == 0) {
		if (mkdir(fileName, 0777) == -1) {
			return -1;
		}
	}

	return 0;
}

int Login(char * UID, char * password) {
	char fileName[256];

	if (DEBUG) printf("Logging in user %s\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s", UID);
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
				return -2;
			}

			if (fread(password_db, 1, strlen(password), file) <= 0) {
				if (DEBUG) printf("Error reading from pass file\n");
				return -2;
			}

			fclose(file);

			if (strncmp(password_db, password, 8)) {
				if (DEBUG) printf("Wrong password\n");
				return -1;
			}
		} 
		else {
			// CREATES PASSWORD FILE
			int ret = CreateUser(UID, password);
			if (ret == 0) {
				if (DEBUG) printf("User %s created\n", UID);
				return 0;
			} else {
				if (DEBUG) printf("Error creating user %s\n", UID);
				return -2;
			}
		}

		// CREATES LOGIN FILE
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);

		if (checkAssetFile(fileName)) {
			if (DEBUG) printf("User %s is logged in\n", UID);
			return 1;
		} else {
			if (DEBUG) printf("User %s is not logged in\n", UID);
			FILE * file = fopen(fileName, "w");
			if (file == NULL) {
				if (DEBUG) printf("Error opening file\n");
				return -2;
			}
			fclose(file);
		}
		return 1;
	} else {
		if (DEBUG) printf("User %s does not exist\n", UID);
		int ret = CreateUser(UID, password);
		if (ret == 0) {
			if (DEBUG) printf("User %s created\n", UID);
			return 0;
		} else {
			if (DEBUG) printf("Error creating user %s\n", UID);
			return -2;
		}
	}
}

int Logout(char * UID) {
	char fileName[256];

	if (DEBUG) printf("Logging out user %s\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s/%s_pass.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s has no password defined\n", UID);
		return -1;
	}

	if (DEBUG) printf("User %s has password defined\n", UID);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s is not logged in\n", UID);
		return 0;
	}

	if (DEBUG) printf("User %s is logged in\n", UID);

	unlink(fileName);
	return 1;
}

int Unregister(char * UID) {
	char fileName[256];

	if (DEBUG) printf("Unregistering user %s\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s/%s_pass.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s has no password defined\n", UID);
		return -1;
	}
	
	unlink(fileName);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s isn't logged in\n", UID);
		return -2;
	}

	if (DEBUG) printf("User %s is logged in\n", UID);
	unlink(fileName);

	return 0;
}

int CreateAuction(char * UID, char*name, char * asset_fname, char * start_value, char * time_active, char * start_datetime, time_t start_fulltime, char * file_size, int socket_fd, char * remaining_message, size_t remaining_size) {
	if (DEBUG) printf("Creating auction\n");

	char fileName[256];

	//Generate AID
	char AID[4];
	memset(AID, 0, sizeof(AID));
	int i = 0;
	for (i = 1; i < 1000; i++) {
		sprintf(fileName, "ASDIR/AUCTIONS/%03d", i);
		if (checkAssetFile(fileName) == 0) {
			sprintf(AID, "%03d", i);
			if (DEBUG) printf("AID: %s\n", AID);
			break;
		}
		if (i == 999) {
			if (DEBUG) printf("Error generating AID\n");
			return -1;
		}
	}

	//Create auction directory
	sprintf(fileName, "ASDIR/AUCTIONS/%s", AID);
	if (mkdir(fileName, 0777) == -1) {
		if (DEBUG) printf("Error creating auction directory\n");
		return -1;
	}
	memset(fileName, 0, sizeof(fileName));

	//Start_AID file
	sprintf(fileName, "ASDIR/AUCTIONS/%s/START_%s.txt", AID, AID);
	FILE * file = fopen(fileName, "w");
	if (file == NULL) {
		if (DEBUG) printf("Error creating Start_AID file\n");
		return -1;
	}

	char * start_info = malloc(strlen(UID) + strlen(name) + strlen(asset_fname) + strlen(start_value) + strlen(time_active) + strlen(start_datetime) + sizeof(start_fulltime) + 7);

	//Write to file info about the auction
	sprintf(start_info, "%s %s %s %s %s %s %ld", UID, name, asset_fname, start_value, time_active, start_datetime, start_fulltime);
	
	size_t written = fwrite(start_info, 1, strlen(start_info), file);
	if (written != strlen(start_info)) {
		if (DEBUG) printf("Error writing to Start_AID file\n");
		fclose(file);
		free(start_info);
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

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/AUCTIONS/%s/ASSET/%s", AID, asset_fname);
	long fsize = atol(file_size);
	//TODO: Create asset file (from TCP)
	printf("File sizessssss: %ld\n", fsize);
	ServerReceiveFile(fileName, fsize, socket_fd, remaining_message, remaining_size);

	memset(fileName, 0, sizeof(fileName));

	//Bids directory
	sprintf(fileName, "ASDIR/AUCTIONS/%s/BIDS", AID);
	if (mkdir(fileName, 0777) == -1) {
		if (DEBUG) printf("Error creating bids directory\n");
		return -1;
	}

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "ASDIR/USERS/%s/HOSTED/%s.txt", UID, AID);
	file = fopen(fileName, "w");
	if (file == NULL) {
		if (DEBUG) printf("Error creating hosted file\n");
		return -1;
	}
	fclose(file);
	
	return i;
}

int CloseAuction(char * AID, char * UID) {
	if (DEBUG) printf("Closing auction %s\n", AID);

	char fileName[256];

	//TODO: Make close auction function
	sprintf(fileName, "ASDIR/AUCTIONS/%s/START_%s.txt", AID, AID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("Auction %s exists\n", AID);
	} else {
		if (DEBUG) printf("Auction %s does not exist\n", AID);
		return -1; //* -1 = auction does not exist
	}

	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		if (DEBUG) printf("Error opening file\n");
		return -4;
	}

	char auction_info[256];

	if (fread(auction_info, 1, 256, file) <= 0) {
		if (DEBUG) printf("Error reading from auction file\n");
		return -4;
	}
	fclose(file);

	char db_UID[6];

	char * token = strtok(auction_info, " ");
	strcpy(db_UID, token);

	if (strcmp(db_UID, UID)) {
		if (DEBUG) printf("User %s is not the auction host\n", UID);
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

	sprintf(fileName, "ASDIR/AUCTIONS/%s/END_%s.txt", AID, AID);

	if (checkIfAuctionEnded(AID) == 1) {
		if (DEBUG) printf("Auction %s already ended\n", AID);
		return -3; //* -3 = auction already ended
	}

	//end auction
  file = fopen(fileName, "w");
	if (file == NULL) {
		if (DEBUG) printf("Error creating end file\n");
		return -4;
	}

	time_t now; // seconds since 1970-01-01 00:00:00
	char end_datetime_str[20]; // yyyy-mm-dd hh:mm:ss
	time_t time_passed; // seconds since auction start

	memset(end_datetime_str, 0, sizeof(end_datetime_str));

	time(&now);

	timeToString(now, end_datetime_str); 

	time_passed = now - atol(start_fulltime);

	if (DEBUG) printf("End datetime: %s\n", end_datetime_str);

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "%s %ld", end_datetime_str, time_passed);

	size_t written = fwrite(fileName, 1, strlen(fileName), file);
	if (written != strlen(fileName)) {
		if (DEBUG) printf("Error writing to end file\n");
		fclose(file);
		return -4;
	}

	fclose(file);

	return 0;
}

int Bid(char * AID, char * UID, char * value, char * datetime, char * fulltime) {
	if (DEBUG) printf("Bidding on auction %s\n", AID);

	char fileName[256];

	//TODO: CHECK IF BID VALUE IS HIGHER THAN CURRENT HIGHEST BID

	//Create bid file
	sprintf(fileName, "ASDIR/AUCTIONS/%s/BIDS/%s.txt", AID, value);
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

	memset(fileName, 0, sizeof(fileName));

	//Add bid to user's bidded directory
	sprintf(fileName, "ASDIR/USERS/%s/BIDDED/%s.txt", UID, AID);
	file = fopen(fileName, "w");
	if (file == NULL) {
		if (DEBUG) printf("Error creating bid file\n");
		return -1;
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
	sprintf(curPath, "ASDIR/AUCTIONS/%s/ASSET/", AID);
	if (checkAssetFile(curPath)) {
		if (DEBUG) printf("Auction %s exists\n", AID);
	} else {
		if (DEBUG) printf("Auction %s does not exist\n", AID);
		return -1; //* -1 = auction does not exist
	}

	// Start scanning a directory unil a file is found
	n_entries = scandir(curPath, &filelist, 0, alphasort);
	if (n_entries <= 0) {
		if (DEBUG) printf("No files in asset folder of auction %s", AID);
		return -1;
	}

	while(n_entries--) {
		name_len=strlen(filelist[n_entries]->d_name);
		if (name_len > 2) break;	//Ignore '.' and '..'
	}

	// If a file is not found
	if (n_entries < 0) {
		if (DEBUG) printf("Error fetching file from auction %s", AID);
		free(filelist);
		return -1;
	}

	// Get the name of the file
	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, filelist[n_entries]->d_name);
	strcat(curPath, fileName);
	free(filelist);

	if (DEBUG) printf("Found file %s\n", curPath);

	// Get the length of the file
	stat(curPath, &filestat);
	file_size = filestat.st_size;
	if (file_size == 0) {
		if (DEBUG) printf("File is empty.\n");
		return -1;
	}

	// Open the file to start sending it
	fd = open(curPath, O_RDONLY);
	if (fd==-1) {
		if (DEBUG) printf("Couldn't open auction asset (id: %s)\n", AID);
		return -1;
	}

	sprintf(buffer, "RSA OK %s %ld ", fileName, file_size);
	printf("%s\n", buffer);
	server_tcp_send(socket_fd, buffer, strlen(buffer));

	serverSendFile(fd, file_size, socket_fd);

	close(fd);

	return 0;
}

int CheckUserLogged(char * UID, char * password) {
	char fileName[256];
	if (DEBUG) printf("Checking if user %s is logged in\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s/%s_pass.txt", UID, UID);
	if (!checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s is not registered\n", UID);
		return -1;
	}

	int ret = CheckUserPassword(UID, password);

	switch (ret) {
		case 0:
			if (DEBUG) printf("Wrong password\n");
			return -2;
		case 1:
			if (DEBUG) printf("Correct password\n");
			break;
		default:
			if (DEBUG) printf("Error checking password\n");
			return -2;
	}

	memset(fileName, 0, sizeof(fileName));
	
	sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s is logged in\n", UID);
		return 1;
	} else {
		if (DEBUG) printf("User %s is not logged in\n", UID);
		return 0;
	}
}

//Function to convert from time_t to string
void timeToString(time_t time, char * time_string) {
	struct tm * timeinfo;

	timeinfo = gmtime(&time);

	sprintf(time_string, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	return;
}

int LoadBid(char * pathname, struct BIDLIST bid) {
	if (DEBUG) printf("Loading bid from file %s\n", pathname);
	
	FILE * file = fopen(pathname, "r");
	if (file == NULL) {
		if (DEBUG) printf("Error opening file\n");
		return -1;
	}

	char bid_info[64];

	if (fread(bid_info, 1, 64, file) <= 0) {
		if (DEBUG) printf("Error reading from bid file\n");
		return -1;
	}

	char * token = strtok(bid_info, " ");
	strcpy(bid.UID, token);
	token = strtok(NULL, " ");
	strcpy(bid.value, token);
	token = strtok(NULL, " ");
	strcpy(bid.datetime, token);
	token = strtok(NULL, "\n");
	strcpy(bid.fulltime, token); //? CHECK THIS THING HERE

	if (DEBUG) printf("UID: %s\nValue: %s\nDatetime: %s\nFulltime: %s\n (não é suposto ter 2 newlines)\n", bid.UID, bid.value, bid.datetime, bid.fulltime);

	fclose(file);

	return 0;
}

int GetBidList(char * AID, struct BIDLIST * bidlist) {

	struct dirent **filelist;
	int n_entries, n_bids, len;
	char dirname[32];
	char pathname[564]; //! Change this shit

	sprintf(dirname, "ASDIR/AUCTIONS/%s/BIDS", AID);

	n_entries = scandir(dirname, &filelist, 0, alphasort);

	if (n_entries <= 0) {
		if (DEBUG) printf("Error scanning directory\n");
		return -1;
	}

	n_bids = 0;

	len = n_entries >= 50 ? 50 : n_entries;

	bidlist = malloc((len) * sizeof(struct BIDLIST));

	while (n_entries--) {
		len = strlen(filelist[n_bids]->d_name);
		if (DEBUG) printf("File name: %s\n", filelist[n_bids]->d_name);
		if (len == 10) {
			sprintf(pathname, "ASDIR/AUCTIONS/%s/BIDS/%s", AID, filelist[n_bids]->d_name);
			if (DEBUG) printf("Pathname: %s\n", pathname);
			if (LoadBid(pathname, bidlist[n_bids])) {
				++n_bids;
			}
			free(filelist[n_entries]);
		}
		if (n_bids == 50) {
			break;
		}
	}
	free(filelist);
	return n_bids;
}

int checkIfAuctionEnded(char * AID) {
	if (DEBUG) printf("Checking if auction %s ended\n", AID);

	char fileName[256];
	sprintf(fileName, "ASDIR/AUCTIONS/%s", AID);

	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("Auction %s exists\n", AID);
	} else {
		if (DEBUG) printf("Auction %s does not exist\n", AID);
		return -1;
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/AUCTIONS/%s/END_%s.txt", AID, AID);

	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("Auction %s ended\n", AID);
		return 1;
	} else {
		if (DEBUG) printf("Checking file time:\n");
		
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "ASDIR/AUCTIONS/%s/START_%s.txt", AID, AID);

		FILE * file = fopen(fileName, "r");
		if (file == NULL) {
			if (DEBUG) printf("Error opening file\n");
			return -1;
		}

		char file_data[256];

		char time_active[10];
		char start_date_str[10];

		memset(time_active, 0, sizeof(time_active));
		memset(start_date_str, 0, sizeof(start_date_str));


		if (fread(file_data, 1, sizeof(file_data), file) <= 0) {
			if (DEBUG) printf("Error reading from start file\n");
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
		time_t start_date = atoi(start_date_str);
		time_t end_date = start_date + atoi(time_active);

		time(&now);

		if (end_date <= now) {
			if (DEBUG) printf("Auction %s ended\n", AID);
			memset(fileName, 0, sizeof(fileName));
			sprintf(fileName, "ASDIR/AUCTIONS/%s/END_%s.txt", AID, AID);
			file = fopen(fileName, "w");
			if (file == NULL) {
				if (DEBUG) printf("Error creating end file\n");
				return -1;
			}

			char end_datetime[20];
			char time_to_end[10];

			memset(end_datetime, 0, sizeof(end_datetime));

			timeToString(end_date, end_datetime);

			size_t written = fwrite(end_datetime, 1, strlen(end_datetime), file);
			if (written != strlen(end_datetime)) {
				if (DEBUG) printf("Error writing to end file\n");
				fclose(file);
				return -1;
			}

			written = fwrite(" ", 1, 1, file);
			if (written != 1) {
				if (DEBUG) printf("Error writing to end file\n");
				fclose(file);
				return -1;
			}

			sprintf(time_to_end, "%ld", end_date - start_date);

			written = fwrite(time_to_end, 1, strlen(time_to_end), file);
			if (written != strlen(time_to_end)) {
				if (DEBUG) printf("Error writing to end file\n");
				fclose(file);
				return -1;
			}

			fclose(file);
			return 1;
		} else {
			if (DEBUG) printf("Auction %s did not end\n", AID);
			return 0;
		}
	}
}

int GetAuctionInfo(char * AID) {
	char fileName[256];

	sprintf(fileName, "ASDIR/AUCTIONS/%s/START_%s.txt", AID, AID);
	if (!checkAssetFile(fileName)) {
		if (DEBUG) printf("Auction %s does not exist\n", AID);
		return -1;
	}

	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		if (DEBUG) printf("Error opening file\n");
		return -1;
	}

	char auction_info[256];

	memset(auction_info, 0, sizeof(auction_info));

	if (fread(auction_info, 1, 256, file) <= 0) {
		if (DEBUG) printf("Error reading from auction file\n");
		return -1;
	}

	char UID[6];
	char name[32];
	char asset_fname[32];
	char start_value[6];
	char time_active[6];
	char start_datetime[19];

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

	token = strtok(NULL, " ");
	strcpy(start_datetime, token);

	if (DEBUG) printf("UID: %s\nName: %s\nAsset filename: %s\nStart value: %s\nTime active: %s\nStart datetime: %s\n", UID, name, asset_fname, start_value, time_active, start_datetime);

	fclose(file);

	struct BIDLIST * bidlist = NULL;

	int n_bids = GetBidList(AID, bidlist);
	if (DEBUG) printf("Number of bids: %d\n", n_bids);

	for (int i = 0; i < n_bids; i++) {
		if (DEBUG) printf("UID: %s\nValue: %s\nDatetime: %s\nFulltime: %s\n", bidlist[i].UID, bidlist[i].value, bidlist[i].datetime, bidlist[i].fulltime);
		//TODO: HANDLE BIDS HERE
	}

	free(bidlist);

	int checkIfEnded = checkIfAuctionEnded(AID);

	if (checkIfEnded == 1) {
		if (DEBUG) printf("Auction %s ended\n", AID);
		
	} else if (checkIfEnded == 0) {
		if (DEBUG) printf("Auction %s did not end\n", AID);
	} else {
		if (DEBUG) printf("Error checking if auction %s ended\n", AID);
	}
	return 0;
}

int GetAuctionsList(struct AUCTIONLIST * auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[32];
	char pathname[512];

	sprintf(dirname, "ASDIR/AUCTIONS");

	n_entries = scandir(dirname, &filelist, 0, alphasort);

	if (n_entries <= 0) {
		if (DEBUG) printf("Error scanning directory\n");
		return -1;
	}

	n_auctions = 0;

	auction_list = malloc((n_entries) * sizeof(struct AUCTIONLIST));

	int i = 0;

	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[n_auctions]->d_name);
		if (DEBUG) printf("File name: %s\n", filelist[n_auctions]->d_name);
		if (len == 3) {
			sprintf(pathname, "ASDIR/AUCTIONS/%s", filelist[n_auctions]->d_name);
			if (DEBUG) printf("Pathname: %s\n", pathname);
			if (checkAssetFile(pathname)) {
				strcpy(auction_list[n_auctions].AID, filelist[n_auctions]->d_name);
				if (DEBUG) printf("AID: %s\n", auction_list[n_auctions].AID);
				++n_auctions;
			}
		}
		free(filelist[n_entries]);
	}
	free(filelist);
	return n_auctions;
}

int GetAuctionsListByUser(char * UID, struct AUCTIONLIST * auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[32];
	char pathname[512];

	sprintf(dirname, "ASDIR/USERS/%s/HOSTED", UID);

	n_entries = scandir(dirname, &filelist, 0, alphasort);

	if (n_entries <= 0) {
		if (DEBUG) printf("Error scanning directory\n");
		return -1;
	}

	n_auctions = 0;

	auction_list = malloc((n_entries) * sizeof(struct AUCTIONLIST));

	int i = 0;

	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[n_auctions]->d_name);
		if (DEBUG) printf("File name: %s\n", filelist[n_auctions]->d_name);
		if (len == 3) {
			sprintf(pathname, "ASDIR/USERS/%s/HOSTED/%s", UID, filelist[n_auctions]->d_name);
			if (DEBUG) printf("Pathname: %s\n", pathname);
			if (checkAssetFile(pathname)) {
				strcpy(auction_list[n_auctions].AID, filelist[n_auctions]->d_name);
				if (DEBUG) printf("AID: %s\n", auction_list[n_auctions].AID);
				++n_auctions;
			}
		}
		free(filelist[n_entries]);
	}
	free(filelist);
	return n_auctions;
}

int GetAuctionsListByUserBidded(char * UID, struct AUCTIONLIST * auction_list) {
	struct dirent **filelist;
	int n_entries, n_auctions, len;
	char dirname[32];

	sprintf(dirname, "ASDIR/USERS/%s/BIDDED", UID);

	n_entries = scandir(dirname, &filelist, 0, alphasort);

	if (n_entries <= 0) {
		if (DEBUG) printf("Error scanning directory\n");
		return -1; //! USER WAS NOT LOGGED IN
	}

	n_auctions = 0;

	auction_list = malloc((n_entries) * sizeof(struct AUCTIONLIST));

	int i = 0;

	for (i = 0; i < n_entries; i++) {
		len = strlen(filelist[n_auctions]->d_name);
		if (DEBUG) printf("File name: %s\n", filelist[n_auctions]->d_name);
		if (len == 3) {
			strcpy(auction_list[n_auctions].AID, filelist[n_auctions]->d_name);
			auction_list[n_auctions].active = 1; //! checkIfAuctionEnded(auction_list[n_auctions].AID) ? 1 : 0;
			if (DEBUG) printf("AID: %s\n", auction_list[n_auctions].AID);
			++n_auctions;
		}
		free(filelist[n_entries]);
	}
	free(filelist);
	return n_auctions;
}

int CheckUserPassword(char * UID, char * password) {
	char fileName[256];

	sprintf(fileName, "ASDIR/USERS/%s/%s_pass.txt", UID, UID);
	if (checkAssetFile(fileName) <= 0) {
		if (DEBUG) printf("User %s has no password defined\n", UID);
		return -1;
	}
	
	if (DEBUG) printf("User %s has password defined\n", UID);

	char password_db[8];
	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		if (DEBUG) printf("Error opening file\n");
		return -1;
	}

	if (fread(password_db, 1, strlen(password), file) <= 0) {
		if (DEBUG) printf("Error reading from pass file\n");
		return -1;
	}

	fclose(file);

	printf("\n%s : %s \n", password_db, password);

	if (strncmp(password_db, password, 8)) {
		if (DEBUG) printf("Wrong password\n");
		return 0;
	}

	return 1;
}