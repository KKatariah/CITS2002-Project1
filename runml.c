//  CITS2002 Project 1 2024
//  Student1:   23381807   Ari Carter
//  Student2:   23993019   Hongkang "Roy" Xu
//  Platform:   MacOS, Linux Mint

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 255
#define INIT_LINE_COUNT 10
#define MAX_VARNAME_LENGTH 120
#define MAX_VARVALUE_LENGTH 120

typedef struct {
    char **content;
    int linecounts;
    // [ROY] I decided to make StrBlock stateful to help reducing segv
    int curline;
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
    StrBlock res = {content, linecounts, 0};
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

void rmnewline(char *str) {
    size_t len = strlen(str);
    if (str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void freecontent(char **content) {
    // IDE won't be happy if I don't check content is null or not
    if (content == NULL) {
        return;
    }
    // still wondering why i < sizeof(content) is needed, or else there will be a repeat freeing=SIGABRT issue
    for (int i = 0; content[i] != NULL && i < sizeof(content); i++) {
        printf("Freeing content[%d] at address %p with value '%s'\n", i, (void *)content[i], content[i]);
        free(content[i]);
    }
    free(content);
}

void transprint(StrBlock *dest, StrBlock *src, int targetline) {
    // no validation on syntax yet
    printf("@print found\n");
    // if print is not at leftmost
    if (strstr(src->content[targetline], "print") - src->content[targetline] != 0) {
        printf("@ILLEGAL ML SYNTAX\n");
        exit(1);
    } else {
        char *substr = strstr(src->content[targetline], " ");
        rmnewline(substr);
        sprintf(dest->content[dest->curline], "printf(\"%%f\\n\",%s);", substr);
        dest->curline += 1;
    }
}

void transassign(StrBlock *dest, StrBlock *src, StrBlock *varlist, int targetline) {
    printf("@assign found\n");

    // for getting a cleaned var name from src
    char varname[MAX_VARNAME_LENGTH];
    int var_cur = 0;
    // value don't need to be clean, or "shouldn't"
    char value[MAX_VARVALUE_LENGTH];

    // just a simplification
    char *line = src->content[targetline];
    rmnewline(line);
    int pos = strstr(line, "<-") - line;
    for (int i = 0, startspace = 1; i < pos; i++) {
        // deal with space/tabs (is space allowed?) in assignment statement inside function body
        if (line[i] == ' ' || line[i] == '\t' && startspace == 1) {
            continue;
        } else { startspace = 0; }
        if (line[i] == ' ') {
            printf("@SYNTAX ERROR: space not allowed in variable name!\n");
            exit(-1);
        }
        varname[var_cur] = line[i];
        var_cur += 1;
    }

    varname[var_cur] = '\0';
    strcpy(value, line + pos + 2);

    // scan through current varlist
    for (int i = 0; i < varlist->linecounts; i++) {
        // if exists, overwrite value in main()
        if (strstr(varlist->content[i], varname)) {
            // ensure full name matched
            // all lines in varlist is like "float foo = bar"
            // just a reminder here, varlist->content[i] is a line=string
            // j is line cursor, k is varname cursor
            for (int j = 6, k = 0; j < strlen(varlist->content[i]) - 5 && varlist->content[i][j] != '\0';
                 j++, k++) {
                if (varlist->content[i][j] != varname[k]) { break; }
            }
            sprintf(dest->content[dest->curline], "%s = %s;", varname, value);
            dest->curline += 1;
            return;
        }
    }
    // if not, define new var
    sprintf(varlist->content[varlist->curline], "float %s = %s;", varname, value);
    varlist->curline += 1;
}

// Using varlist is quite similar to the one in transassign(), isolate it?
void transfunc(StrBlock *dest, StrBlock *src) {
    // src, varlist is a temporary storage, free mem when return
    StrBlock varlist = strblockinit();

    // strip the funcname and arguments down, src->content[0] is "function foobar a , b ... "
    // first space pos = 9; j is varlist's inline cursor
    // let first line in varlist to be function name
    for (int i = 9, j = 0; i < (size_t)strlen(src->content[0]); i++) {
        // check varlist size as usual
        if (varlist.curline >= varlist.linecounts) {
            strblockexpand(&varlist);
        }
        // next arg
        if (src->content[0][i] == ' ') {
            // enclose last line
            varlist.content[varlist.curline][j] = '\0';
            j = 0;
            varlist.curline += 1;
        } else {
            varlist.content[varlist.curline][j] = src->content[0][i];
            j++;
        }
    }
    // put function declaration down
    char buf[MAX_LINE_LENGTH] = {'\0'};
    char comma[2] = {','};
    // <= curline not < curline, curline is not size
    for (int i = 1; i <= varlist.curline; i++) {
        strcat(buf,"float ");
        strcat(buf, varlist.content[i]);
        strcat(buf, comma);
    }
    // strip the last comma
    buf[strlen(buf) - 1] = '\0';
    sprintf(dest->content[dest->curline], "float %s (%s){", varlist.content[0], buf);
    dest->curline +=1;
    if (dest->curline>=dest->linecounts){
        strblockexpand(dest);
    }
    sprintf(dest->content[dest->curline], "}");

    // add a return statement for catching all

    freecontent(varlist.content);
    freecontent(src->content);
}

int main(int argc, char *argv[]) {
    // get file descriptor
    FILE *ifp = openfile(argv[1]);
    FILE *ofp = fopen("./.runml_temp.c", "w");

    StrBlock inputfile = loadfile(ifp);

    // init .runml_temp.c, with proper headers
    fputs("#include <stdio.h>\n"
          "#include <stdlib.h>\n"
          "#include <string.h>\n", ofp);

    // store statements which will be written into .ml's main(), in a StrBlock struct
    StrBlock mlmain = strblockinit();

    // store functions
    StrBlock mlfunc = strblockinit();

    // global variable list
    StrBlock glvarlist = strblockinit();

//    // DEBUG: print out the content
//    for (int i = 0; inputfile.content[i] != NULL; i++) {
//        printf("@%s", inputfile.content[i]);
//    }

    // ********************  Main Logic ********************
    for (int i = 0; i < inputfile.linecounts; i++) {
        // check if the line is empty, or it's a comment
        if (inputfile.content[i][0] == '\n' || inputfile.content[i][0] == '#') {
            continue;
        }
        // *****************************************************************
        // check mlmain's size in each loop
        if (mlmain.curline >= mlmain.linecounts) {
            printf("@mlmain expanded.\n");
            strblockexpand(&mlmain);
        }
        // *****************************************************************
        // print statement found
        if (strstr(inputfile.content[i], "print") != NULL) {
            transprint(&mlmain, &inputfile, i);
            continue;
        }
        // *****************************************************************
        // value assign
        if (strstr(inputfile.content[i], "<-") != NULL) {
            transassign(&mlmain, &inputfile, &glvarlist, i);
            continue;
        }
        // *****************************************************************
        // enter function body
        if (strstr(inputfile.content[i], "function") != NULL) {
            printf("@function start\n");
            StrBlock funcbody = strblockinit();
            // do-while to include the funtion defn line
            do {
                // check funcbody's size in every iteration
                if (funcbody.curline >= funcbody.linecounts) {
                    strblockexpand(&funcbody);
                }
                rmnewline(inputfile.content[i]);
                strcpy(funcbody.content[funcbody.curline], inputfile.content[i]);
                // don't forget to increment outter loop's counter
                i += 1;
                funcbody.curline += 1;
            } while (i < inputfile.linecounts && inputfile.content[i][0] == '\t');
            transfunc(&mlfunc, &funcbody);
            continue;
        }
            // *****************************************************************
            // catch all
        else {
            exit(-1);
        }
        // *****************************************************************

    }

    // *****************************************************


    // free memory
    freecontent(inputfile.content);
    fclose(ifp);


    // write global varlist to .c file
    for (int i = 0; i < glvarlist.linecounts; i++) {
        fputs(glvarlist.content[i], ofp);
        fputs("\n", ofp);
    }

    // write function declarations to .c file
    for (int i = 0; i < mlfunc.linecounts; i++) {
        fputs(mlfunc.content[i], ofp);
        fputs("\n", ofp);
    }
    // write main func cache into .c file
    fputs("int main(){\n", ofp);
    for (int i = 0; i < mlmain.linecounts; i++) {
        fputs(mlmain.content[i], ofp);
        fputs("\n", ofp);
    }
    fputs("\n}\n", ofp);

    // flush writes to ofp first, or else it's likely to fail
    fclose(ofp);

    // compile and execute .runml.temp.c
    if (system("gcc ./.runml_temp.c -o .ml") == 0) {
        printf("@Compile ml successful\n");
        printf("@ml executing...\n");
        int exec_res = system("./.ml");
        if (exec_res == 0) {
            printf("\n@ml executed\n");
        } else {
            printf("@ml execution failed\n");
        }
    } else {
        printf("@ml compilation failed\n");
    }

    // NOT IMPLEMENTED: delete .runml.temp.c

    return 0;
}


