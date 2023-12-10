#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "file_handling.h"
#include <time.h>

typedef struct BIDLIST {
	char AID[3];
	char value[6];
	char UID[6];
	char datetime[19];
	char fulltime[6];
} BIDLIST[];

typedef struct AUCTIONLIST {
	char AID[3];
	int active;
} AUCTIONLIST[];

int DEBUG = 1; 

/*
 * Function that initializes the database. If db is already initialized, it does nothing
*/
void InitDatabase();

/*
 * Function that creates a new user
 * @param UID: User ID
 * @param password: User password
 * 
 * @returns 0 if no error occurred, -1 if an error occurred
*/
int CreateUser(char * UID, char * password);

/*
 * Function that logs in a user
 * @param UID: User ID
 * @param password: User password
 * 
 * @returns 1 if user logs in, 0 if it's a new user, -1 if an error occurred
*/
int Login(char * UID, char * password);

/*
 * Function that logs out a user
 * @param UID: User ID
 * 
 * @returns 1 if user logs out, 0 if user is not logged in
*/
int Logout(char * UID);

/*
 * Function that unregisters a user
 * @param UID: User ID
 * 
 * @returns 0 if no error occurred, -1 if an error occurred
*/
int Unregister(char * UID);

/*
 * Function that creates a new auction
 * @param UID: User ID
 * @param name: Auction name
 * @param asset_fname: Asset filename
 * @param start_value: Auction start value
 * @param time_active: Auction time active (in seconds)
 * @param start_datetime: Auction start datetime
 * @param start_fulltime: Time since auction start (in seconds)
 * 
 * @returns 0 if no error occurred, -1 if an error occurred
*/
int CreateAuction(char * UID, char*name, char * asset_fname, char * start_value, char * time_active, char * start_datetime, char * start_fulltime);

/*
 * Function that creates a new bid
 * @param AID: Auction ID
 * @param UID: User ID
 * @param value: Bid value
 * @param datetime: Bid datetime
 * @param fulltime: Time since auction start (in seconds)
 * 
 * @returns 0 if no error occurred, -1 if an error occurred
*/
int Bid(char * AID, char * UID, char * value, char * datetime, char * fulltime);

/*
 * Function that loads a bid from a file to a BIDLIST struct
 * @param pathname: path to bid file
 * @param bid: BIDLIST struct where the bid will be stored
 * 
 * @returns 0 if no error occurred, -1 if an error occurred
*/
int LoadBid(char * pathname, struct BIDLIST bid);

/*
 * Function that checks if a user is logged in
 * @param UID: User ID
 * 
 * @returns 1 if user is logged in, 0 if user is not logged in
*/
int CheckUserLogged(char * UID);

/*
 * Function that searches for the 50 highest bids on an auction
 * @param AID: Auction ID
 * @param bidlist: pointer to where the bidlist will be stored (MUST NOT be allocated. MUST be freed after use)
 * 
 * @returns number of bids found
 */
int GetBidList(char * AID, struct BIDLIST * bidlist);

/*
 * Function that checks if an auction ended
 * @param AID: Auction ID
 * 
 * @returns 1 if auction ended, 0 if auction did not end, -1 if an error occured
*/
int checkIfAuctionEnded(char * AID);

/*
 * Function that gets infor for a specific auction
 * @param AID: Auction ID
 * 
 * @returns 0 if no error occured or -1 if an error occured
*/
int GetAuctionInfo(char * AID);

/*
 * Function that gets all the auctions in the system
 * @param auction_list: pointer to where the auction list will be stored (MUST NOT be allocated. MUST be freed after use)
 * 
 * @returns number of auctions found
*/
int GetAuctionsList(struct AUCTIONLIST * auction_list);

/*
 * Function that gets all the auctions hosted by a user
 * @param UID: User ID
 * @param auction_list: pointer to where the auction list will be stored (MUST NOT be allocated. MUST be freed after use)
 * 
 * @returns number of auctions found
*/
int GetAuctionsListByUser(char * UID, struct AUCTIONLIST * auction_list);