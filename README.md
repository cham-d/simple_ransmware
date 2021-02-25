# A Simple Ransomware

### General Info
------------
The main part of this Assignment is to make a ransmware program that can do two
things: 
* Create big volume of files and encrypt files. (You can decrypt them too.)
* The second one is to add some extra functionality to the file monitor program,so
it can detects the encrypted files and detects if a big volume of files was created 
in the last 20 minutes.<br />

 I. FILE LIST
------------
- acmonitor.c                    The file monitor tool with the new features.
- logger.c                       The logger with the 'fake' fopen and fwrite.
- MakeFile                       MakeFile,everyone knows it :).
- ransomware.h                   File with the 'malicious' code.
- creteFilesScript.c             A script that creates many files using fopen,written in C.
  <br />
  
II. INSTALATION
------------
1. Just run make command.<br />

 III. COMMAND LINE ARGUMENTS EXAMPLES
------------
##### Ransmware examples:

Example1:<br />
Make 1000 files in directory "testingDirectory" :<br />
```sh make ransom ARGS="-c testingDirectory -x 1000" ```

Example2:<br />
Encrypt all files in testingDirectory Folder,using password 1234:<br />
```sh make ransom ARGS="-e testingDirectory -p 1234" ```

Example3:<br />
Decrypt all files in testingDirectory Folder,using password 1234:<br />
```sh make ransom ARGS="-d testingDirectory -p 1234" ```

##### Monitor tool examples:

Show all encrypted files:<br />
 ```sh ./acmonitor -e ```

Show how many files created the last 20 minutes and prints a
warning message if they are more than 30.<br />
```sh ./acmonitor -i 30 ```<br />

  IV. THE WAY THAT THE WHOLE PROJECT WORKS-COMMENTS
------------

The Ransmware uses aes-256-ecb to encrypt/decrypt the files.
 It also uses createFilesScript.c to create the big volume of files.

   The option -e of the monitoring program works like:
 1.Scan the log file to find .encrypt extention in files with mode 0(creation).<br />
 2.When it finds one,it pauses the search and start scan the
 log file again until it reaches the pause point(The point that it 
 had been pause the search before) or if it find the same file without the 
 .encrypt extention and with mode 1(read the file).<br />
 3.If the last one is true then it prints tha the file has been encrypted.<br />
 4.it continues the search for the pause point until it reaches the 
 end of the log file.<br />

   The option -i finds and prints how many files have been created the last
 20 min by looking its file datetime modification-create field and comparing it
 with the today datetime.
