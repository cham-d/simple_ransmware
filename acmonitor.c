
#include <time.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

struct entry
{

	int uid;		   /* user id (positive integer) */
	int access_type;   /* access type values [0-2] */
	int action_denied; /* is action denied values [0-1] */

	time_t date; /* file access date */
	time_t time; /* file access time */

	char *file;		   /* filename (string) */
	char *fingerprint; /* file fingerprint */

	/* add here other fields if necessary */
	/* ... */
	/* ... */
};

struct user_entry
{

	int uid; /* user id (positive integer) */
	int fail_attempts;
	char **try_access_files;
};

void usage(void)
{
	printf(
		"\n"
		"usage:\n"
		"\t./monitor \n"
		"Options:\n"
		"-m, Prints malicious users\n"
		"-i <filename>, Prints table of users that modified \n"
		"-v <number of files>Prints the total number of files created in the last 20 minutes\n"
		"-e, Prints all the files that were encrypted by the ransomware"
		"the file <filename> and the number of modifications\n"
		"-h, Help message\n\n");

	exit(1);
}

//Returns time in current moment in time_t format
time_t
getTimeAndDateNow()
{
	char cur_time[128];
	char *actual_time = malloc(255);
	time_t t;
	struct tm *ptm;
	t = time(NULL);
	ptm = localtime(&t);
	return t;
}

//Compare cuurent timedate and timedate in args and return the diff in secs.
int timeCompare(char *date, char *time)
{
	struct tm tminfo;
	double diff_t;
	char buffer[80];

	int hour = atoi(strtok(time, ":"));
	int min = atoi(strtok(NULL, ":"));
	int sec = atoi(strtok(NULL, "-"));

	int mday = atoi(strtok(date, "-"));
	int month = atoi(strtok(NULL, "-"));
	int year = atoi(strtok(NULL, "-"));

	tminfo.tm_mday = mday;
	tminfo.tm_mon = month - 1;
	tminfo.tm_year = year - 1900;
	tminfo.tm_hour = hour;
	tminfo.tm_min = min;
	tminfo.tm_sec = sec;
	tminfo.tm_isdst = -1;

	time_t t = mktime(&tminfo);

	time_t tmNow = getTimeAndDateNow();

	diff_t = difftime(tmNow, t);
	//printf("H diafora einai %f\n",diff_t);
	return diff_t;
}

void list_unauthorized_accesses(FILE *log)
{
	char line[256];
	int maxUsersCount = 0;
	while (fgets(line, sizeof(line), log))
	{
		maxUsersCount++;
	}
	printf("Lines of Text: %d\n", maxUsersCount);

	struct user_entry *user_entries = malloc(maxUsersCount * sizeof(struct user_entry)); //Dhmioyrgia pinaka apo structs
	rewind(log);
	int lineCount = 0;

	user_entries[0].uid = 0;
	user_entries[0].fail_attempts = 0;
	int distinct_users = 1;
	while (fgets(line, sizeof(line), log))
	{
		int userid = atoi(strtok(line, "|"));

		char *path = strtok(NULL, "|");
		char *date = strtok(NULL, "|");
		char *time = strtok(NULL, "|");
		char *mode = strtok(NULL, "|");
		char *privileges = strtok(NULL, "|");
		char *filehash = strtok(NULL, "|");
		int k = distinct_users;
		int exists = 0;
		int hasNoPriveleges = 0;
		int findPointer = 0;

		exists = 0;
		hasNoPriveleges = 0;
		for (int j = 0; j < k; j++)
		{
			if ((strcmp(privileges, "1")) == 0) //An den exei diakiwmata
			{
				hasNoPriveleges = 1;
				if (user_entries[j].uid == userid) //Elegxos An yparxei ston pinaka
				{
					exists = 1;
					findPointer = j;
				}
			}
		}
		int hpointer = sizeof(user_entries[findPointer].try_access_files) / sizeof(user_entries[findPointer].try_access_files[0]) - 1; //BAZW TO MEGETHOS TO PINAKA

		if (exists == 0 && hasNoPriveleges == 1) //An den yparxei ston pinaka kai den exei dikaiwmata,ton prosthetoyme ston pinaka
		{
			//user_entries[distinct_users].uid = malloc(6 * sizeof(char));
			user_entries[distinct_users].uid = userid;
			user_entries[distinct_users].fail_attempts = 0;
			user_entries[distinct_users].try_access_files = malloc(maxUsersCount * sizeof(char *) * 20); //Desmeysh xwrou gia pinaka pou krata ta diaforetika arxeia pou prospathei na epeksergastei o xrhsths

			user_entries[distinct_users].try_access_files[0] = malloc(256 * sizeof(char)); //Desmeysh xwrou gia thn prwth grammh tou pinaka pou krata ta diaforetika arxeia pou prospathei na epeksergastei o xrhsths
			strcpy(user_entries[distinct_users].try_access_files[0], path);

			distinct_users++;
		}
		else if (exists == 1) //An yparxei ston pinaka mas o xrhsths
		{
			int searchForAlreadyNoPrevFile = 0;
			int arrayLength = user_entries[findPointer].fail_attempts + 1;
			for (int arrayFileSearcher = 0; arrayFileSearcher < arrayLength; arrayFileSearcher++) //anazhthsh an exei ksanaprospathhsei na peiraksei to sygkekrimeno arxeio
			{
				if (strcmp((user_entries[findPointer].try_access_files[arrayFileSearcher]), path) == 0)
				{
					searchForAlreadyNoPrevFile = 1;
				}
			}
			if (searchForAlreadyNoPrevFile == 0) //Prosthkh arxeio sta arxeia pou prospathhse na peiraksei o xrhsths
			{
				user_entries[findPointer].fail_attempts++;
				arrayLength = user_entries[findPointer].fail_attempts;
				user_entries[findPointer].try_access_files[arrayLength] = malloc(256 * sizeof(char)); //Desmeysh xwrou gia thn prwth grammh tou pinaka pou krata ta diaforetika arxeia pou prospathei na epeksergastei o xrhsths
				strcpy(user_entries[findPointer].try_access_files[arrayLength], path);
			}
		}
	}
	int notExists = 1;
	for (int i = 1; i < distinct_users; i++)
	{
		if ((user_entries[i].fail_attempts) >= 6) //Einai 6,epeidh to fail attempts xrhshmopoieitai san deikths pinaka opote ksekina apo to 0
		{
			printf("User: %d \n", user_entries[i].uid);
			printf("Accesses: %d \n", user_entries[i].fail_attempts + 1);
			printf("------------\n");
			notExists = 0;
		}
	}

	if (notExists == 1)
	{
		printf("No malicius Users detected.\n");
	}
}

void list_last_created_files(FILE *log, char *uphold)
{
	int limit = atoi(uphold);
	char line[256];
	int totalLastCreated = 0;
	while (fgets(line, sizeof(line), log))
	{
		int userid = atoi(strtok(line, "|"));
		char *path = strtok(NULL, "|");
		char *date = strtok(NULL, "|");
		char *time = strtok(NULL, "|");

		char *mode = strtok(NULL, "|");
		char *privileges = strtok(NULL, "|");
		char *filehash = strtok(NULL, "|");
		int secFromNow = timeCompare(date, time);
		if (strcmp(mode, "0") == 0 && (secFromNow < 1200))
		{
			totalLastCreated++;
		}
	}
	if (totalLastCreated > limit)
	{
		printf("Malicious behaviour detected!\n");
	}
	printf("%d files created in the last 20 minutes \n", totalLastCreated);
}

void list_file_modifications(FILE *log, char *file_to_scan)
{
	//Edw bazoyme oloklhro to path mazi me to arxeio
	char cwd[256];
	char pathToWrite[260];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		sprintf(pathToWrite, "%s/%s", cwd, file_to_scan);
	}
	else
	{
		perror("getcwd() error");
	}
	strcpy(file_to_scan, pathToWrite);
	//***********************************************

	char line[256];
	int maxUsersCount = 0;
	while (fgets(line, sizeof(line), log))
	{
		maxUsersCount++; //Briskoyme to max twn user
	}

	char **users_count;
	users_count = malloc(maxUsersCount * sizeof(char *) * 2); //dhmioyrgoyme ena pinaka oso to maxuser *2,giati sthn prwth thesh tha mpainei o user,sth deyterh o arithmos prosbashs
	users_count[0] = malloc(5 * sizeof(char));
	strcpy(users_count[0], "null");
	users_count[1] = malloc(2 * sizeof(char));
	strcpy(users_count[1], "0");
	int distinct_users = 2;

	rewind(log);
	int lineCount = 0;
	char *hashToCompare = malloc(33 * sizeof(char));

	int found = 0;
	//Eyresh toy arxikoy hash pou tha sygkrinetai me ta ypoloipa
	while ((fgets(line, sizeof(line), log)) && found == 0)
	{
		char *userid = strtok(line, "|");
		char *path = strtok(NULL, "|");
		char *date = strtok(NULL, "|");
		char *time = strtok(NULL, "|");
		char *mode = strtok(NULL, "|");
		char *privileges = strtok(NULL, "|");
		char *filehash = strtok(NULL, "|");
		int k = distinct_users;
		int exists = 0;
		int hasChangeFile = 0;
		int findPointer = 0;

		if (strcmp(users_count[0], "null") == 0)
		{

			if (strcmp(file_to_scan, path) == 0 && strcmp(privileges, "0") == 0)
			{
				strcpy(hashToCompare, filehash);
				strcpy(users_count[0], userid);
				strcpy(users_count[1], "1");
				distinct_users = 2;
				found = 1;
			}
		}
	}

	//Scanarisma toy arxeioy kai sygkrish twn hashes gia na doume pou hparxoyna allages
	//An brethei  allagh to neo hash me to opoio tha sygkrithoyn ta ypoloipa einai ayto ths allaghs
	if (found != 0)
	{
		rewind(log);
		while (fgets(line, sizeof(line), log))
		{

			char *userid = strtok(line, "|");
			char *path = strtok(NULL, "|");
			char *date = strtok(NULL, "|");
			char *time = strtok(NULL, "|");
			char *mode = strtok(NULL, "|");
			char *privileges = strtok(NULL, "|");
			char *filehash = strtok(NULL, "|");
			int k = distinct_users;
			int exists = 0;
			int hasChangeFile = 0;
			int findPointer = 0;

			if (strcmp(file_to_scan, path) == 0)
			{
				for (int j = 0; j < distinct_users; j += 2)
				{
					if (strcmp(users_count[j], userid) == 0)
					{
						exists = 1;
						findPointer = j;
					}

					if (strcmp(hashToCompare, filehash) != 0 && strcmp(privileges, "0") == 0)
					{
						//printf("hastocomp %s filehaa %s \n",hashToCompare,filehash);
						hasChangeFile = 1;
					}
				}

				if (exists == 0 && hasChangeFile == 1) //An den yparxei ston pinaka kai exei allaksei to arxeio,bale ton xrhsth sto pinaka
				{

					users_count[distinct_users] = malloc(6 * sizeof(char));
					strcpy(users_count[distinct_users], userid);
					distinct_users++;
					users_count[distinct_users] = malloc(4 * sizeof(char));
					strcpy(users_count[distinct_users], "1");
					distinct_users++;

					strcpy(hashToCompare, filehash);
				}
				else if (exists == 1 && hasChangeFile == 1) //An yparxeis ston pinaka kai exei allaksei to arxeio prosthese stis allages toy +1.
				{
					char mod_count[100];

					sprintf(mod_count, "%d", (atoi(users_count[findPointer + 1])) + 1);
					strcpy(users_count[findPointer + 1], mod_count);
					strcpy(hashToCompare, filehash);
				}
			}
		}
		//Ektypws pinaka me ta apotelesmata
		printf("-----------------------------------\n");
		printf("|%s %25s\n", " USER |", "Number of modifications  |");
		printf("|------|--------------------------|\n");
		for (int i = 0; i < distinct_users; i += 2)
		{
			printf("| %s |%15s           |\n", users_count[i], users_count[i + 1]);
			printf("|------|--------------------------|\n");
		}
	}
	else
	{
		printf("No logs for this file, please check your file path and try again.\n");
		printf("If a folder contains your file please dont include '/' in front of your path(ex. directory/file.txt)\n");
		printf("The program finds files in the directory that you execute it.\n");
	}
}

//Returns the filename from a file path.
char *
getFileName(char *path)
{
	char delim[] = "/";
	char *fileName;
	char *ptr = strtok(path, delim);
	while (ptr != NULL)
	{
		fileName = ptr;
		ptr = strtok(NULL, delim);
	}
	return fileName;
}

//Returms true if the file is encrypted(last word is ".encrypt")
bool isEncrypted(char *fileName)
{
	char delim[] = ".";
	char *lastWord;
	bool res = false;
	char filename[128];
	strcpy(filename, fileName);

	char *ptr = strtok(filename, delim);
	while (ptr != NULL)
	{
		lastWord = ptr;
		ptr = strtok(NULL, delim);
	}
	if ((strcmp(lastWord, "encrypt")) == 0)
	{
		res = true;
	}

	return res;
}

//Returns true if a file has been opened (mode=1),in the past
//It searches only before the line that the encrypted file has been found (numOfLine),
//so the whole programm runs faster.
bool hasBeenOpened(char *fileNameToCheck, FILE *log, int numOfLine)
{
	bool res = false;
	char line[256];

	int prev = ftell(log); //Save the cuurent pointer of the log file,so we can read it again from the start
						   //to search for the reading operation.After we done we restore the log pointer.
	rewind(log);

	while (fgets(line, sizeof(line), log) && numOfLine > 0)
	{

		int userid = atoi(strtok(line, "|"));
		char *path = strtok(NULL, "|");
		char *date = strtok(NULL, "|");
		char *time = strtok(NULL, "|");
		char *mode = strtok(NULL, "|");
		char *fileName = getFileName(path);

		if ((strcmp(fileNameToCheck, fileName)) == 0 && (strcmp(mode, "1")) == 0)
		{
			res = true;
		}
		numOfLine--;
	}
	fseek(log, prev, SEEK_SET); //go back to where we were
	return res;
}

void list_encrypted_files(FILE *log)
{
	char line[256];
	int maxUsersCount = 0;
	while (fgets(line, sizeof(line), log))
	{
		maxUsersCount++;
	}
	printf("Lines of Text: %d\n", maxUsersCount);
	int numOfLine = 0;
	rewind(log);
	while (fgets(line, sizeof(line), log))
	{
		int userid = atoi(strtok(line, "|"));
		char *path = strtok(NULL, "|");
		char *date = strtok(NULL, "|");
		char *time = strtok(NULL, "|");
		char *mode = strtok(NULL, "|");
		char *fileName = getFileName(path);

		if (isEncrypted(fileName) && strcmp(mode, "0") == 0) //if file is encrypted and it was created(there wasnt such a file before the encrtption)
		{

			//*****RESORE THE NAME THE FILE HAD,BEFORE THE ENCRYPTION  HAPPENED********
			char FileNameBeforeEncryption[128];
			strcpy(FileNameBeforeEncryption, "");
			char delim[] = ".";
			char *ptr = strtok(fileName, delim);
			while ((ptr != NULL) && (strcmp(ptr, "encrypt")) != 0)
			{
				strcat(FileNameBeforeEncryption, ".");
				strcat(FileNameBeforeEncryption, ptr);
				ptr = strtok(NULL, delim);
			}

			//Remove the first dot.
			if (FileNameBeforeEncryption[0] == '.')
			{
				memmove(FileNameBeforeEncryption, FileNameBeforeEncryption + 1, strlen(FileNameBeforeEncryption));
			}
			//************************************************************

			if (hasBeenOpened(FileNameBeforeEncryption, log, numOfLine)) //if the file has been opened in the past(if it wasnt opened,it cant be encrypted)
			{
				printf("File %s has been encrypted.\n", FileNameBeforeEncryption);
			}
		}
		numOfLine++;
	}
}

int main(int argc, char *argv[])
{

	int ch;
	FILE *log;

	if (argc < 2)
		usage();

	log = fopen("./file_logging.log", "r");
	if (log == NULL)
	{
		printf("Error opening log file \"%s\"\n", "./log");
		return 1;
	}

	while ((ch = getopt(argc, argv, "hv:i:me")) != -1)
	{
		switch (ch)
		{
		case 'v':
			list_last_created_files(log, optarg);
			break;
		case 'i':
			list_file_modifications(log, optarg);
			break;
		case 'm':
			list_unauthorized_accesses(log);
			break;
		case 'e':
			list_encrypted_files(log);
			break;
		default:
			usage();
		}
	}

	fclose(log);
	argc -= optind;
	argv += optind;

	return 0;
}
