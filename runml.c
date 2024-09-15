//  CITS2002 Project 1 2024
//  Student1:   23381807   Ari Carter
//  Student2:   STUDENT-NUMBER2   Roy Xu
//  Platform:   MacOS(??), Linux Mint

#include <stdio.h> 
#include <stdlib.h> 

int main(int argc, char *argv[]) { 
	char file[] = read_file();
}

int readfile(char filename[])
{
    char file[] = fopen(filename, "r");

    if(file == NULL) {
        return 1;
    }
    
    fclose(filename);
    return 0;
}