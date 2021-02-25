#!/bin/bash

    
function enc_files 
{
            echo "All files in this directory will be encrypted: $1";
            for entry in "$1"/*
            do
                output="${entry}.encrypt"
                printf "Encrypting %-40s  [OK]\n" ${entry}
                openssl enc -aes-256-ecb -pbkdf2 -in "$entry" -out "$output"  -k "$2" 
                rm "$entry"
            done;
 } 

 function dec_files 
{
            echo "All files in this directory will be decrypted: $1";
            for entry in "$1"/*
            do
                part1=`dirname "$entry"`;
                part2=`basename "$entry"`;
                var1="$1/${part2%.*}";              
                printf "Decrypting %-40s  [OK]\n" ${var1}
                openssl aes-256-ecb -pbkdf2 -in "$entry" -out "${var1}" -d -k "$2" 
                rm "$entry"
                touch "ok"
            done;
 } 

function create_files
{
./createFiles $2 $1
}

function usage
{
    echo "-e <directory> -p <password>:             Encrypts all files in <directory> using <password>";
    echo "-d <directory> -p <password>:             Decrypts all files in <directory> using <password>";
    echo "-x <number_of_files>:                     Create <number_of_files> files in current directory";
    echo "-c <directory> -x <number_of_files>:      Create <number_of_files> files in <directory>";
}


e=false
d=false
c=false
x=false
p=false
check=true

while getopts e:d:c:p:x: flag
	do
	    case "${flag}" in
	        e) enc_path=${OPTARG};
               e=true ;;
            d) dec_path=${OPTARG};
               d=true ;;
            p) pass=${OPTARG};
               p=true ;;
	        c) create_files_path=${OPTARG};
               c=true ;;
            x) number_of_files=${OPTARG};
               x=true ;;
            *)
            check=false;
            usage
            ;;
	    esac
	done
if $check; then
    if $e && $p; then enc_files $enc_path $pass;   
    elif $d && $p; then dec_files $dec_path $pass;     
    elif $c && $x; then create_files $create_files_path $number_of_files;   
    elif $x; then create_files $create_files_path $number_of_files; 
    else usage; fi
fi
