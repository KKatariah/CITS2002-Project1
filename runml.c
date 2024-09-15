//  CITS2002 Project 1 2024
//  Student1:   23381807   Ari Carter
//  Student2:   STUDENT-NUMBER2   Roy Xu
//  Platform:   MacOS(??), Linux Mint

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

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

// list keywords 
//const char keywords[] = {
 //   "return",
 //   "print",

//}

//Check syntax (idk what to call this)
int is_thing(char ch) {
    return (ch == '(' || ch == ')' || ch == ';')
}
int is_operator(char ch) {
    return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '=')
}
int is_keyword (const char word) {
    return (word == "return" || word == "print" || word == "function")
}

// Split each line into words
int split_line(char line[], int word_count) {
    int i = 0; //index of line
    int j = 0; //index of current_word
    char current_word[12]; //max length is 12 for words

    while (line[i] != '\0') {
        if (isspace(line[i])){
            i++;
            continue;
        }
        if isalpha(line[i]) {
            while (isalpha(line[i]) && j <= 12) { //need to make sure an error is thrown if the word is too long
                current_word[j] = line[i];
                i++;
            }
        }
    }
}
