#ifndef DATABASE_HANDLING_H
#define DATABASE_HANDLING_H

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "file_handling.h"
#include "utils.h"
#include <time.h>

typedef struct BIDLIST {
	char AID[4];
	char value[12];
	char UID[7];
	char datetime[20];
	char fulltime[12];
} BIDLIST;

typedef struct AUCTIONLIST {
	char AID[4];
	int active;
} AUCTIONLIST;

/***
 * Function that initializes the database. If db is already initialized, it does nothing
*/
void InitDatabase();

/***
 * Function that creates a new user
 * @param UID: User ID
 * @param password: User password
 * 
 * @returns 0 if no error occurred, -1 if an error occurred
*/
int CreateUser(char * UID, char * password);

/***
 * Function that logs in a user
 * @param UID: User ID
 * @param password: User password
 * 
 * @returns 1 if user logs in, 0 if it's a new user, -1 if an error occurred
*/
int Login(char * UID, char * password);

/***
 * Function that closes an auction. Doenn't verify if user is logged in
 * @param AID: Auction ID
 * @param UID: User ID
 * 
 * @returns 0 if no error occurred, -1 if auction does not exist, -2 if user is not the auction owner, -3 if auction has ended, -4 if an error occurred
*/
int CloseAuction(char * AID, char * UID);

/***
 * Function that logs out a user
 * @param UID: User ID
 * 
 * @returns 1 if user logs out, 0 if user is not logged in, -1 if not even registered
*/
int Logout(char * UID);

/***
 * Function that unregisters a user
 * @param UID: User ID
 * 
 * @returns 0 if user is successfully unregistered, -1 if user is not registered, -2 if an error occurred
*/
int Unregister(char * UID);

/*****
 * Function that creates a new auction
 * @param UID: User ID
 * @param name: Auction name
 * @param asset_fname: Asset filename
 * @param start_value: Auction start value
 * @param time_active: Auction time active (in seconds)
 * @param start_datetime: Auction start datetime
 * @param start_fulltime: Time since auction start (in seconds)
 * @param file_size: Asset file size (in String)
 * @param socket_fd: Socket file descriptor
 * @param remaining_message: Remaining message to be read from socket
 * 
 * @returns 0 if no error occurred, -1 if an error occurred
*/
int CreateAuction(char * UID, char*name, char * asset_fname, char * start_value, char * time_active, char * start_datetime, time_t start_fulltime, char * file_size, int socket_fd);

/***
 * Returns the highest value of a bid
 * 
 * @param AID The ID of the auction to get the highest bid
 * @returns the highest bid value if there are bids, 0 if there are no bids,
 * -1 if an error occurred
*/
int GetHighestBid(char * AID);

/***
 * Function that creates a new bid
 * @param AID: Auction ID
 * @param UID: User ID
 * @param value: Bid value
 * 
 * @returns 0 if no error occurred, -1 if auction has ended, -2 if bid has not the highest value, -3 if user is the owner, -4 if user is not logged in, -5 if an error occurred
*/
int Bid(char * AID, char * UID, char * value);

/***
 * Opens the asset inside the folder of an auction and sends its data using TCP
 * 
 * @param AID the ID of the auction where the asset is located
 * @param socket_fd The socket where the file data will be sent
*/
int ShowAsset(char * AID, int socket_fd);

/***
 * Function that loads a bid from a file to a BIDLIST struct
 * @param pathname: path to bid file
 * @param bid: BIDLIST struct where the bid will be stored
 * 
 * @returns 0 if no error occurred, -1 if an error occurred
*/
int LoadBid(char * pathname, struct BIDLIST * bid);

/***
 * Function that checks if a user is logged in
 * @param UID: User ID
 * 
 * @returns 1 if user is logged in, 0 if user is not logged in, -1 if not registered, -2 if incorrect password
*/
int CheckUserLogged(char * UID, char * password);

/***
 * Function that searches for the 50 highest bids on an auction
 * @param AID: Auction ID
 * @param bidlist: pointer to where the bidlist will be stored (MUST NOT be allocated. MUST be freed after use)
 * 
 * @returns number of bids found
 */
int GetBidList(char * AID, struct BIDLIST ** bidlist);

/***
 * Function that checks if an auction ended
 * @param AID: Auction ID
 * 
 * @returns 1 if auction ended, 0 if auction did not end, -1 if an error occured
*/
int checkIfAuctionEnded(char * AID);

/***
 * Function that gets infor for a specific auction
 * @param AID: Auction ID
 * @param message: pointer to beginning of message where the info will be stored
 * 
 * @returns 0 if no error occured or -1 if an error occured
*/
int GetAuctionInfo(char * AID, char * message);

/***
 * Function that gets all the auctions in the system
 * @param auction_list: pointer to where the auction list will be stored (MUST NOT be allocated. MUST be freed after use)
 * 
 * @returns number of auctions found
*/
int GetAuctionsList(struct AUCTIONLIST ** auction_list);

/***
 * Function that gets all the auctions hosted by a user
 * @param UID: User ID
 * @param auction_list: pointer to where the auction list will be stored (MUST NOT be allocated. MUST be freed after use)
 * 
 * @returns number of auctions found
*/
int GetAuctionsListByUser(char * UID, struct AUCTIONLIST ** auction_list);

/*****
 * Function that gets all the auctions bidded by a user
 * @param UID: User ID
 * @param auction_list: pointer to where the auction list will be stored (MUST NOT be allocated. MUST be freed after use)
 * 
 * @returns number of auctions found
*/
int GetAuctionsListByUserBidded(char * UID, struct AUCTIONLIST ** auction_list);

/***
 * Function that checks if user password is correct
 * @param UID: User ID
 * @param password: User password
 * 
 * @returns 1 if password is correct, 0 if password is incorrect, -1 if an error occurred
 */
int CheckUserPassword(char * UID, char * password);

/***
 * Function that converts a time_t to a string
 * @param time: time_t to be converted
 * @param time_string: array where the string will be stored
*/
void timeToString(time_t time, char * time_string);

#endif