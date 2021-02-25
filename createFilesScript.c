#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>



/*Simple program,that creates files using fopen.
HOW TO USE IT: 
Call it using as args an int(the number of files you want to create)
If you want the files to be created in a directory,include the directory as 
a second arg.*/


int main(int argc, char *argv[])
{
    FILE *file;
    size_t bytes;
    char c;
    char text[100];
    strcpy(text, "");
    if (argc < 2)
    {
        printf("Not enough args.\n");
        exit(2);
    }

    //Create FileNames
    char **filenames;
    filenames = malloc(atoi(argv[1]) * sizeof(char *));

    for (int i = 0; i < atoi(argv[1]); i++)
    {
        char i_string[12];
        filenames[i] = malloc((60 + 1) * sizeof(char)); //60=mhkos path
        strcpy(filenames[i],"");
        if (argv[2]!=NULL)
        {
            struct stat st = {0};
            if (stat(argv[2], &st) == -1) 
            {
                mkdir(argv[2], 0700);
            }
            strcpy(filenames[i], argv[2]);
            strcat(filenames[i], "/");
        }
        strcat(filenames[i], "file");
        sprintf(i_string, "%d", i);
        strcat(filenames[i], i_string);
        strcat(filenames[i], ".txt");
    }

    //Create Files
      for (int i=0;i<atoi(argv[1]);i++)
      {
            file = fopen(filenames[i],"w");
    		if (file == NULL)
    			printf("fopen error\n");
    		else
    		{
    			bytes = fwrite(filenames[i], strlen(filenames[i]), 1, file);
    			fclose(file);
    		}
      }
}