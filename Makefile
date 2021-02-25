all: logger acmonitor createFiles

logger: logger.c
	gcc -Wall -fPIC -shared -o logger.so logger.c -lcrypto -ldl 

acmonitor: acmonitor.c 
	gcc acmonitor.c -o acmonitor
	
createFiles: createFilesScript.c 
	gcc createFilesScript.c -o createFiles 

ransom: 
	LD_PRELOAD=./logger.so bash ransomware.sh $(ARGS)

clean:
	rm -rf logger.so
	rm -rf acmonitor
	rm -rf createFiles

