//  CITS2002 Project 1 2024
//  Student1:   23381807   Ari Carter
//  Student2:   23993019   Hongkang "Roy" Xu
//  Platform:   MacOS, Linux Mint

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 255
#define INIT_LINE_COUNT 20

typedef struct {
    char **content;
    int linecounts;
} StrBlock;


FILE *openfile(char str[]) {
    FILE *fp = fopen(str, "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(-1);
    }
    return fp;
}

StrBlock strblockinit() {
    // init limit on total lines are 20
    int linecounts = INIT_LINE_COUNT;
    char **content = (char **) calloc(linecounts, sizeof(char *));
    // set char limit for each line as 255 for now
    for (int j = 0; j < INIT_LINE_COUNT; j++) {
        // notice sizeof should be using char, not char *
        content[j] = (char *) calloc(MAX_LINE_LENGTH, sizeof(char));
    }
    StrBlock res = {content, linecounts};
    return res;
}

// failure on mem op will just kill program for now
void strblockexpand(StrBlock *block) {
    // double the size, similar to python's strateg
    char **newaddr = (char **) realloc(block->content, block->linecounts * 2 * sizeof(char *));
    if (newaddr != NULL) {
        block->content = newaddr;
    } else {
        printf("@StrBlock Array Expand failed, exiting...\n");
        exit(-1);
    }
    // init new space
    for (int j = block->linecounts; j < block->linecounts * 2; j++) {
        block->content[j] = (char *) calloc(MAX_LINE_LENGTH, sizeof(char));
    }
    block->linecounts *= 2;
}

StrBlock loadfile(FILE *fp) {
    StrBlock mlfile = strblockinit();

    // iterate & read through all lines
    int i = 0;
    while (fgets(mlfile.content[i], MAX_LINE_LENGTH, fp) != NULL) {
        i++;
        // if lines exceed current limitation
        // maybe it's better to isolate it to a new function
        if (i == mlfile.linecounts) {
            strblockexpand(&mlfile);
        }
    }
    // EOF Marking
    mlfile.content[i] = NULL;

    // shrink linecounts to actual line counts to avoid segfault
    mlfile.linecounts = i;
    return mlfile;
}

void freecontent(char **content) {
    // IDE won't be happy if I don't check content is null or not
    if (content == NULL) {
        return;
    }
    for (int i = 0; content[i] != NULL; i++) {
        free(content[i]);
    }
    free(content);
}


int main(int argc, char *argv[]) {
    // get file descriptor
    FILE *ifp = openfile(argv[1]);
    FILE *ofp = fopen("./.runml_temp.c", "w");

    StrBlock inputfile = loadfile(ifp);

    // init .runml_temp.c, with proper headers
    fputs("#include <stdio.h>\n", ofp);

    // store statements which will be written into .ml's main(), in a StrBlock struct
    StrBlock mlmain = strblockinit();
    // mlmain current line cursor
    int mlmaincur = 0;



    // DEBUG: print out the content
    for (int i = 0; inputfile.content[i] != NULL; i++) {
        printf("@%s", inputfile.content[i]);
    }

    // ********************  Main Logic ********************
    for (int i = 0; i < inputfile.linecounts; i++) {
        // check if the line is empty, or it's a comment
        if (inputfile.content[i][0] == '\n' || inputfile.content[i][0] == '#') {
            continue;
        }
        if (strstr(inputfile.content[i], "print") != NULL) {
            // print statement found
            // no validation on syntax yet
            printf("@print found\n");
            // if print is not at leftmost
            if (strstr(inputfile.content[i], "print") - inputfile.content[i] != 0) {
                printf("@ILLEGAL ML SYNTAX\n");
                exit(1);
            } else {
                strcpy(mlmain.content[mlmaincur], "printf(\"%f\",");
                mlmaincur += 1;
                // insert expression
                char *param = strstr(inputfile.content[i], " ");
                strcpy(mlmain.content[mlmaincur], param);
                mlmaincur += 1;

                // close printf()
                strcpy(mlmain.content[mlmaincur], ");");
            }
            continue;


            // print expression: not implemented
            // print numeric: not implemented
            // print var: not implemented
        }
        if (strstr(inputfile.content[i], "function") != NULL) {
            printf("@function start\n");
            // enter function body
        }
            // placeholder logic
        else {
//            strcpy(mlmain.content[mlmaincur], inputfile.content[i]);
//            mlmaincur += 1;

        }
        if (mlmaincur == mlmain.linecounts) {
            printf("@mlmain expanded.\n");
            strblockexpand(&mlmain);
        }
    }



    // *****************************************************



    // free memory
    freecontent(inputfile.content);
    fclose(ifp);


    fputs("int main(){\n", ofp);

    // write main func cache into .c file
    for (int i = 0; i < mlmain.linecounts; i++) {

        fputs(mlmain.content[i], ofp);
    }

    fputs("\n}\n", ofp);
    // flush writes to ofp first, or else it's likely to fail
    fclose(ofp);
    // compile and execute .runml.temp.c
    if (system("gcc ./.runml_temp.c -o .ml") == 0) {
        printf("@Compile ml successful\n");
        int exec_res = system("./.ml");
        if (exec_res == 0) {
            printf("@ml executed\n");
        } else {
            printf("@ml execution failed\n");
        }
    } else {
        printf("@ml compilation failed\n");
    }
    return 0;
}


