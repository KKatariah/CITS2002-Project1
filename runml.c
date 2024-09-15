//  CITS2002 Project 1 2024
//  Student1:   23381807   Ari Carter
//  Student2:   23993019   Hongkang "Roy" Xu
//  Platform:   MacOS, Linux Mint

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_LINE_LENGTH 255
#define INIT_LINE_COUNT 20

typedef struct {
    char **content;
    int linecounts;
} Inputfile;


FILE *openfile(char str[]) {
    FILE *fp = fopen(str, "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(-1);
    }
    return fp;
}

Inputfile loadfile(FILE *fp) {
    // init limit on total lines are 20
    int linecounts = INIT_LINE_COUNT;
    char **content = (char **) calloc(linecounts, sizeof(char *));
    // set char limit for each line as 255 for now
    for (int j = 0; j < INIT_LINE_COUNT; j++) {
        content[j] = (char *) calloc(MAX_LINE_LENGTH, sizeof(char *));
    }

    // iterate through all lines
    int i = 0;
    while (fgets(content[i], MAX_LINE_LENGTH, fp) != NULL) {
        i++;
        // if lines exceed current limitation
        // maybe it's better to isolate it to a new function
        if (i > linecounts) {
            // double the size, similar to python's strategy
            linecounts *= 2;
            char **newaddr = (char **) realloc(content, linecounts * sizeof(char *));
            if(newaddr != NULL){
                content = newaddr;
            } else{
                printf("@Inputfile Array Expand failed, exiting...\n");
                exit(-1);
            }
            // init new space
            for (int j = 0; j < INIT_LINE_COUNT; j++) {
                content[j] = (char *) calloc(MAX_LINE_LENGTH, sizeof(char *));
            }
        }
    }
    // EOF Marking
    content[i] = NULL;
    Inputfile res = {content, i};
    return res;
}

void freecontent(char **content) {
    for (int i = 0; content[i] != NULL; i++) {
        free(content[i]);
    }
    free(content);
}


int main(int argc, char *argv[]) {
    // get file descriptor
    FILE *fp = openfile(argv[1]);
    Inputfile inputfile = loadfile(fp);
    // print out the content
    for (int i = 0; inputfile.content[i] != NULL; i++) {
        printf("%s", inputfile.content[i]);
    }
    freecontent(inputfile.content);
    fclose(fp);
    return 0;
}



// list keywords 
//const char keywords[] = {
//   "return",
//   "print",

//}

////Check syntax (idk what to call this)
//int is_thing(char ch) {
//    return (ch == '(' || ch == ')' || ch == ';');
//}
//
//int is_operator(char ch) {
//    return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '=');
//}
//
//int is_keyword(const char word) {
//    return (word == "return" || word == "print" || word == "function");
//}
//
//// Split each line into words
//int split_line(char line[], int word_count) {
//    int i = 0; //index of line
//    int j = 0; //index of current_word
//    char current_word[12]; //max length is 12 for words
//
//    while (line[i] != '\0') {
//        if (isspace(line[i])) {
//            i++;
//            continue;
//        }
//        if (isalpha(line[i]))
//        {
//            while (isalpha(line[i]) && j <= 12) { //need to make sure an error is thrown if the word is too long
//                current_word[j] = line[i];
//                i++;
//            }
//        }
//    }
//}
