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