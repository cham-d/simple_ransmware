#define _GNU_SOURCE

#include <time.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <errno.h>


int fsize(FILE *fp)
{
	int prev = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	fseek(fp, prev, SEEK_SET); //go back to where we were
	return sz;
}

char *
getTimeAndDateNow()
{
	char cur_time[128];
	char *actual_time = malloc(255);
	time_t t;
	struct tm *ptm;
	t = time(NULL);
	ptm = localtime(&t);
	strftime(cur_time, 128, "%d-%m-%Y|%H:%M:%S", ptm);
	strcpy(actual_time, cur_time);
	return actual_time;
}

char *
getAcessType(const char *mode)
{
	char *modeType = malloc(2 * sizeof(char));
	strcpy(modeType, "");
	if (strcmp(mode, "create") == 0)
	{
		strcpy(modeType, "0"); //Create Mode
	}
	else if ((strcmp(mode, "read") == 0))
	{
		strcpy(modeType, "1"); //Read Mode
	}
	else if ((strcmp(mode, "write") == 0))
	{
		strcpy(modeType, "2");
	}
	return modeType;
}

FILE *
myCleanfopen(const char *path, const char *mode)
{
	FILE *original_fopen_ret;
	FILE *(*original_fopen)(const char *, const char *);
	original_fopen = dlsym(RTLD_NEXT, "fopen");
	original_fopen_ret = (*original_fopen)(path, mode);
	return original_fopen_ret;
}

char *
getFilePathName(FILE *file)
{
	int fd;
	char fd_path[255];
	char *filename = malloc(255);
	ssize_t n;

	fd = fileno(file);
	sprintf(fd_path, "/proc/self/fd/%d", fd);
	n = readlink(fd_path, filename, 255);
	if (n < 0)
		printf("Error\n");
	filename[n] = '\0';
	return filename;
}

char *
getFileHash(FILE *file)
{
	char *fileHash = malloc(32);
	if (file != NULL)
	{
		char *arrayTingsToWrite;
		unsigned char digest[16];
		strcpy(fileHash, "");
		char fileHash_One[4];
		//Anoigma tou Arxeioy pou "peiraxthke" gia na paroyme to periexomeno kai na ypologisoyme to content
		FILE *file_read = myCleanfopen(getFilePathName(file), "r");
				int fileSize = fsize(file);


		if (file_read == NULL)
			printf("fopen error\n");
		else
		{
			arrayTingsToWrite = malloc(fileSize * sizeof(char));
				//rewind(file);
			fread(arrayTingsToWrite, fileSize, 1, file_read);
			fclose(file_read);
			
		}
		if ((arrayTingsToWrite)!=NULL)
		{
			MD5_CTX ctx;
			MD5_Init(&ctx);
			MD5_Update(&ctx, arrayTingsToWrite, fileSize);
			MD5_Final(digest, &ctx);

			for (int i = 0; i < 16; i++)
			{
				sprintf(fileHash_One, "%02x", digest[i]);
				strcat(fileHash, fileHash_One);
			}
		}else
		{
			strcpy(fileHash, "00000000000000000000000000000000");
		}
	}
	else 
	{
		strcpy(fileHash, "00000000000000000000000000000000");
	}
	return fileHash;
}

char *
buildLogLine(char *pathToWrite, char *privileges, FILE *file, const char *mode)
{
	char *ArrayToWite = malloc(256 * sizeof(char));
	sprintf(ArrayToWite, "%d", getuid());
	strcat(ArrayToWite, "|");
	strcat(ArrayToWite, pathToWrite);
	strcat(ArrayToWite, "|");
	strcat(ArrayToWite, getTimeAndDateNow());
	strcat(ArrayToWite, "|");
	strcat(ArrayToWite, getAcessType(mode));
	strcat(ArrayToWite, "|");
	strcat(ArrayToWite, privileges);
	strcat(ArrayToWite, "|");
	strcat(ArrayToWite, getFileHash(file));
	return ArrayToWite;
}

void writeLogFile(char *logLines)
{
	FILE *original_fopen_ret;
	FILE *(*original_fopen)(const char *, const char *);
	const char pathio[] = "file_logging.log";
	const char *modio = "a";

	original_fopen = dlsym(RTLD_NEXT, "fopen");
	original_fopen_ret = (*original_fopen)(pathio, modio);

	if (original_fopen_ret != NULL)
	{
		fprintf(original_fopen_ret, "%s\n", logLines); //Write the file using fprintf
		fclose(original_fopen_ret);
	}
	else
	{
		printf("Failed to write log file.\n");
	}
}

char *
getPrivileges(FILE *file)
{
	char *privileges = malloc(1 * sizeof(char));
	strcpy(privileges, "");
	if (errno==13) //Mono an den exei dikawmata
	{
		strcpy(privileges, "1");
	}
	else
	{
		strcpy(privileges, "0");
	}
	return privileges;
}

FILE *
fopen(const char *path, const char *mode)
{
	int existBefore = 0;
	int existAfter = 0;
	FILE *original_fopen_ret;
	FILE *(*original_fopen)(const char *, const char *);

	if (access(path, F_OK) != -1)
	{
		existBefore = 1;
	}
	errno=0;
	/* call the original fopen function */
	original_fopen = dlsym(RTLD_NEXT, "fopen");
	original_fopen_ret = (*original_fopen)(path, mode);
	

	if (access(path, F_OK) != -1)
	{
		existAfter = 1;
	}

	//Compute the path
	char cwd[256];
	char pathToWrite[260];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		sprintf(pathToWrite, "%s/%s", cwd, path);
	}
	else
	{
		perror("getcwd() error");
	}

	if (existBefore == 0 && existAfter == 1)
	{
		writeLogFile(buildLogLine(pathToWrite, getPrivileges(original_fopen_ret), original_fopen_ret, "create")); //Create file Log
	}
	else
	{
		writeLogFile(buildLogLine(pathToWrite, getPrivileges(original_fopen_ret), original_fopen_ret, "read")); //Read already existing file Log
	}

	return original_fopen_ret;
}

FILE *
fopen64(const char *path, const char *mode)
{
 FILE *file=fopen(path, mode); 
 return file;
}




size_t
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{

	size_t original_fwrite_ret;
	size_t (*original_fwrite)(const void *, size_t, size_t, FILE *);

	/* call the original fwrite function */
	original_fwrite = dlsym(RTLD_NEXT, "fwrite");
	original_fwrite_ret = (*original_fwrite)(ptr, size, nmemb, stream);
	
	writeLogFile(buildLogLine(getFilePathName(stream), getPrivileges(stream), stream, "write"));//Write a File

	return original_fwrite_ret;
}
