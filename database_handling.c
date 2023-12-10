#include "database_handling.h"

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

int Login(char * UID, char * password) {
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
			return 0;
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

int Logout(char * UID) {
	char fileName[256];

	if (DEBUG) printf("Logging out user %s\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s is logged in\n", UID);
		unlink(fileName);
		return 1;
	} else {
		if (DEBUG) printf("User %s is not logged in\n", UID);
		return 0;
	}
}

int Unregister(char * UID) {
	char fileName[256];

	if (DEBUG) printf("Unregistering user %s\n", UID);

	sprintf(fileName, "ASDIR/USERS/%s", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s exists\n", UID);
	} else {
		if (DEBUG) printf("User %s does not exist\n", UID);
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/USERS/%s/%s_pass.txt", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s has password defined\n", UID);
		unlink(fileName);
	} else {
		if (DEBUG) printf("User %s has no password defined\n", UID);
	}

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s is logged in\n", UID);
		unlink(fileName);
	} else {
		if (DEBUG) printf("User %s isn't logged in\n", UID);
	}

	return 0;
}

int CreateAuction(char * UID, char*name, char * asset_fname, char * start_value, char * time_active, char * start_datetime, char * start_fulltime) {
	if (DEBUG) printf("Creating auction %s\n");

	char fileName[256];

	//Generate AID
	char AID[3];
	int i = 0;
	for (i = 0; i < 1000; i++) {
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

int CloseAuction(char * AID, char * UID) {
	if (DEBUG) printf("Closing auction %s\n", AID);

	char fileName[256];

	if (!CheckUserLogged(UID)) {
		if (DEBUG) printf("User %s is not logged in\n", UID);
		return -1; //* -1 = user not logged in
	}
	
	if (DEBUG) printf("User %s is logged in\n", UID);




	//TODO: Make close auction function
	sprintf(fileName, "ASDIR/AUCTIONS/%s/START_%s.txt", AID, AID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("Auction %s exists\n", AID);
	} else {
		if (DEBUG) printf("Auction %s does not exist\n", AID);
		return -2; //* -2 = auction does not exist
	}

	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		if (DEBUG) printf("Error opening file\n");
		return -1;
	}

	char auction_info[256];

	if (fread(auction_info, 1, 256, file) < 0) {
		if (DEBUG) printf("Error reading from auction file\n");
		return -1;
	}

	char UID[6];

	char * token = strtok(auction_info, " ");
	strcpy(UID, token);

	if (strcmp(UID, UID)) {
		if (DEBUG) printf("User %s is not the auction host\n", UID);
		return -3; //* -3 = user is not the auction host
	}

	fclose(file);

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "ASDIR/AUCTIONS/%s/END_%s.txt", AID, AID);

	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("Auction %s already ended\n", AID);
		return -4; //* -4 = auction already ended
	}


	//end auction
  file = fopen(fileName, "w");
	if (file == NULL) {
		if (DEBUG) printf("Error creating end file\n");
		return -1;
	}

	time_t now;
	struct tm * timeinfo;
	char end_datetime[19];

	memset(end_datetime, 0, sizeof(end_datetime));

	time(&now);

	timeinfo = gmtime(&now);

	sprintf(end_datetime, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	if (DEBUG) printf("End datetime: %s\n", end_datetime);

	size_t written = fwrite(end_datetime, 1, strlen(end_datetime), file);
	if (written != strlen(end_datetime)) {
		if (DEBUG) printf("Error writing to end file\n");
		fclose(file);
		return -1;
	}

	// TODO: Finish calculating time elapsed
	// written = fwrite(" ", 1, 1, file);

	fclose(file);

	return 0;
}

int Bid(char * AID, char * UID, char * value, char * datetime, char * fulltime) {
	if (DEBUG) printf("Bidding on auction %s\n", AID);

	char fileName[256];

	//TODO: CHECK IF BID VALUE IS HIGHER THAN CURRENT HIGHEST BID

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

int CheckUserLogged(char * UID) {
	char fileName[256];

	sprintf(fileName, "ASDIR/USERS/%s/%s_login.txt", UID, UID);
	if (checkAssetFile(fileName)) {
		if (DEBUG) printf("User %s is logged in\n", UID);
		return 1;
	} else {
		if (DEBUG) printf("User %s is not logged in\n", UID);
		return 0;
	}
}

int LoadBid(char * pathname, struct BIDLIST bid) {
	if (DEBUG) printf("Loading bid from file %s\n", pathname);
	
	FILE * file = fopen(pathname, "r");
	if (file == NULL) {
		if (DEBUG) printf("Error opening file\n");
		return -1;
	}

	char bid_info[64];

	if (fread(bid_info, 1, 64, file) < 0) {
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
	char fileName[256];

	struct dirent **filelist;
	int n_entries, n_bids, len;
	char dirname[32];
	char * pathname[64];

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
	sprintf(fileName, "ASDIR/AUCTIONS/%s/", AID, AID);

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
		if (DEBUG) printf("Checking file time:\n", AID);
		
		memset(fileName, 0, sizeof(fileName));
		sprintf(fileName, "ASDIR/AUCTIONS/%s/START_%s.txt", AID, AID);

		FILE * file = fopen(fileName, "r");
		if (file == NULL) {
			if (DEBUG) printf("Error opening file\n");
			return -1;
		}

		char file_data[256];

		char start_datetime[19];
		char time_to_end[6];

		memset(start_datetime, 0, sizeof(start_datetime));
		memset(time_to_end, 0, sizeof(time_to_end));

		if (fread(file_data, 1, 19, file) < 0) {
			if (DEBUG) printf("Error reading from start file\n");
			return -1;
		}

		char * token = strtok(file_data, " ");
		token = strtok(NULL, " ");
		token = strtok(NULL, " ");
		token = strtok(NULL, " ");
		token = strtok(NULL, " ");
		strcpy(start_datetime, token);
		token = strtok(NULL, " ");
		strcpy(time_to_end, token);

		if (DEBUG) printf("Start datetime: %s\nStart time_to_end: %s\n", start_datetime, time_to_end);

		fclose(file);

		time_t now;
		struct tm * timeinfo;
		char end_datetime[19];
		char now_datetime[19];

		memset(now_datetime, 0, sizeof(now_datetime));
		memset(end_datetime, 0, sizeof(end_datetime));

		time(&now);

		timeinfo = gmtime(&now);

		sprintf(now_datetime, "%04d-%02d-%02d %02d:%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

		if (DEBUG) printf("Now datetime: %s\n", now_datetime);

		int time_to_end_int = atoi(time_to_end);


		// ? ESTA PORRA TA MUITO CONFUSA, DEPOIS TENHO QUE REFAZER ISTO
		//calculate end datetime from start datetime and time_to_end
		int start_year, start_month, start_day, start_hour, start_min, start_sec;
		int end_year, end_month, end_day, end_hour, end_min, end_sec;
		int remaining;

		sscanf(start_datetime, "%04d-%02d-%02d %02d:%02d:%02d", &start_year, &start_month, &start_day, &start_hour, &start_min, &start_sec);

		end_sec = start_sec + time_to_end_int;
		remaining = end_sec / 60;
		end_sec = end_sec % 60;

		if (remaining > 0) {
			end_min = start_min + remaining;
			remaining = end_min / 60;
			end_min = end_min % 60;

			if (remaining > 0) {
				end_hour = start_hour + remaining;
				remaining = end_hour / 24;
				end_hour = end_hour % 24;

				if (remaining > 0) {
					end_day = start_day + remaining;
					remaining = end_day / 30;
					end_day = end_day % 30;

					if (remaining > 0) {
						end_month = start_month + remaining;
						remaining = end_month / 12;
						end_month = end_month % 12;

						if (remaining > 0) {
							end_year = start_year + remaining;
						} else {
							end_year = start_year;
						}
					} else {
						end_month = start_month;
						end_year = start_year;
					}
				} else {
					end_day = start_day;
					end_month = start_month;
					end_year = start_year;
				}
			} else {
				end_hour = start_hour;
				end_day = start_day;
				end_month = start_month;
				end_year = start_year;
			}
		} else {
			end_min = start_min;
			end_hour = start_hour;
			end_day = start_day;
			end_month = start_month;
			end_year = start_year;
		}

		sprintf(end_datetime, "%04d-%02d-%02d %02d:%02d:%02d", end_year, end_month, end_day, end_hour, end_min, end_sec);

		if (DEBUG) printf("End datetime: %s\n", end_datetime);

		if (strcmp(now_datetime, end_datetime) > 0) {
			if (DEBUG) printf("Auction %s ended\n", AID);
			memset(fileName, 0, sizeof(fileName));
			sprintf(fileName, "ASDIR/AUCTIONS/%s/END_%s.txt", AID, AID);
			file = fopen(fileName, "w");
			if (file == NULL) {
				if (DEBUG) printf("Error creating end file\n");
				return -1;
			}

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

	if (fread(auction_info, 1, 256, file) < 0) {
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

	struct  BIDLIST * bidlist;

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
	char * pathname[64];

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
	char * pathname[64];

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