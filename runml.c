//  CITS2002 Project 1 2024
//  Student1:   23381807   Ari Carter
//  Student2:   23993019   Hongkang "Roy" Xu
//  Platform:   MacOS, Linux Mint

// Assumption 1: according to syntax defn, a blank line won't count as function body, if it occurs it should mark the end of a function
// Assumption (observation) 2: a line starting with indent outside a function body is not a legal program item
// Assumption 3: any open bracket will be closed by close bracket
// TODO: hard coded varlist count starter for "float": e.g. int j = 6
// TODO: varlist scanning should put isnew check to the outter for-loop
// TODO: transassign() called in transfunc() won't check glvarlist
// TODO: logic flaw in determine of new var: I assumed that varlist is sorted by varname's length; actually the whole logic is just wrong

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_LINE_LENGTH 255
#define INIT_LINE_COUNT 10
#define MAX_VARNAME_LENGTH 120
#define MAX_VARVALUE_LENGTH 120
#define MAX_VARNAMES 50

typedef struct {
    char **content;
    int linecounts;
    // [ROY] I decided to make StrBlock stateful to help reducing segv
    // curline should be a blank line ready for writing
    int curline;
} StrBlock;

// this is not a good practice, but I'm too tired now
StrBlock glvarlist;

// 1 for exist , 0 for new
int checkvar(StrBlock *varlist, char *needle) {
    for (int i = 0; i <= varlist->curline; i++) {
        char buf[MAX_VARNAME_LENGTH] = {'\0'};
        if (varlist->content[i][0] == '\0') {
            continue;
        }
        char *name = strstr(varlist->content[i], " =");
        if (name == NULL) { continue; }
        strncpy(buf, varlist->content[i], name - varlist->content[i]);
        //strip "double " out

        strcpy(buf, strstr(buf, " ") + 1);
        if (strcmp(buf, needle) == 0) {
            return 1;
        }
    }
    return 0;
}

FILE *openfile(char str[]) {
    FILE *fp = fopen(str, "r");
    if (fp == NULL) {
        fprintf(stderr, "! Error opening file. Expected arguments: <runml_program> <input_file>\n");
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
        fprintf(stderr, " ! @StrBlock Array Expand failed, exiting...\n");
        exit(-1);
    }
    // init new space
    for (int j = block->linecounts; j < block->linecounts * 2; j++) {
        block->content[j] = (char *) calloc(MAX_LINE_LENGTH, sizeof(char));
    }
    block->linecounts *= 2;
}


void inccurline(StrBlock *block) {
    block->curline++;
    if (block->curline >= block->linecounts) {
        strblockexpand(block);
    }
}

StrBlock loadfile(FILE *fp) {

    StrBlock mlfile = strblockinit();

    // iterate & read through all lines
    int i = 0;
    while (fgets(mlfile.content[i], MAX_LINE_LENGTH, fp) != NULL) {
        i++;
        // if lines exceed current limitation
        // maybe it's better to isolate it to a new function

        // there's a function now, not yet migrate to it
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

char *rmcontspaces(char *line) {
    // since stored mem space won't be changed, just let the source be char *line as well
    char *src = line;
    char *dst = line;

    while (*src != '\0') {
        if (*src != ' ' || (src != line && (*(src - 1) != ' '))) {
            *dst = *src;
            dst += 1;
        }
        src += 1;
    }
    *dst = '\0';
    return line;
}

void freecontent(char **content) {
    // IDE won't be happy if I don't check content is null or not
    if (content == NULL) {
        return;
    }

    // still wondering why i < sizeof(content) is needed, or else there will be a repeat freeing=SIGABRT issue
    for (int i = 0; content[i] != NULL && i < sizeof(content); i++) {

        free(content[i]);
    }
    free(content);
}

void getvarnames(char *line, char dest[][MAX_VARNAME_LENGTH], int *count) {
    char *ptr = line;
    *count = 0;
    while (*ptr) {
        while (*ptr == ' ') {
            ptr += 1;
        }
        if (*ptr >= 'a' && *ptr <= 'z') {
            int varcur = 0;
            char temp_var[MAX_VARNAME_LENGTH] = {'\0'};
            while ((*ptr >= 'a' && *ptr <= 'z') && varcur < (MAX_VARNAME_LENGTH - 1)) {
                temp_var[varcur++] = *ptr;
                ptr += 1;
            }
            temp_var[varcur] = '\0';

            // ignore function calls, with a new assumption
            if (*ptr == '(') {
                while (*ptr && *ptr != ')') {
                    ptr += 1;
                }
                if (*ptr == ')') {
                    ptr += 1;
                }
                continue;
            }

            if (strcmp(temp_var, "print") == 0 || strcmp(temp_var, "return") == 0) {
                continue;
            }

            if (*count < MAX_VARNAMES) {
                strncpy(dest[*count], temp_var, MAX_VARNAME_LENGTH - 1);
                dest[*count][MAX_VARNAME_LENGTH - 1] = '\0';
                (*count)++;
            } else {

                fprintf(stderr, "Varnames exceed 50.\n");
                break;
            }
        } else {
            ptr += 1;
        }
    }
}


// first arg should always be glvarlist, if there's a second arg it's function's private varlist
void defaultinit(char *line, int argc, ...) {
    // ignore function declaration line
    if (strstr(line, "function ") != 0) {
        return;
    }
    char varname[MAX_VARNAMES][MAX_VARNAME_LENGTH] = {'\0'};
    int count = 0;
    getvarnames(line, varname, &count);

    va_list args;
    va_start(args, argc);
    StrBlock *glvarlist;
    StrBlock *funcvarlist = NULL;
    if (argc >= 1) {
        glvarlist = va_arg(args, StrBlock *);
    }
    if (argc == 2) {
        funcvarlist = va_arg(args, StrBlock *);
    }


    // now we have extracted varname(s)

    // check whether it's a new varname or not
    for (int j = 0; j < count; j++) {
        int isnew_gl = 0;
        int isnew_func = 0;

        if (checkvar(glvarlist, varname[j]) == 0) {
            isnew_gl = 1;
        }
        if (funcvarlist) {
            if (checkvar(funcvarlist, varname[j]) == 0) {
                isnew_func = 1;
            }
        } else {
            isnew_func = 1;
        }

        // default init
        if (isnew_gl == 1 && isnew_func == 1) {
            if (funcvarlist) {
//                    inccurline(funcvarlist);
                snprintf(funcvarlist->content[funcvarlist->curline], MAX_LINE_LENGTH, "double %s = 0.0;", varname[j]);
                inccurline(funcvarlist);
            } else {
//                    inccurline(glvarlist);
                snprintf(glvarlist->content[glvarlist->curline], MAX_LINE_LENGTH, "double %s = 0.0;", varname[j]);
                inccurline(glvarlist);

            }
        }
    }
    va_end(args);
}

void transprint(StrBlock *dest, StrBlock *src, int targetline) {
    // no validation on syntax yet
    // if print is not at leftmost
    if (strstr(src->content[targetline], "print") - src->content[targetline] != 0) {
        if (strncmp(strstr(src->content[targetline], "print"), "print ", strlen("print ")) == 0) {
            fprintf(stderr, "@ILLEGAL ML SYNTAX\n");
            exit(1);
        }

    } else {
        char *substr = strstr(src->content[targetline], " ");

        rmnewline(substr);
        // need to print to stdout?
        sprintf(dest->content[dest->curline],
                "if ((int)(%s)==%s){\n\tprintf(\"%%.0f\\n\",%s);\n}\nelse {\n\tprintf(\"%%.6f\\n\",%s);\n}",
                substr, substr, substr, substr);
//        sprintf(dest->content[dest->curline], "printf(\"%%f\\n\",%s);", substr);
        inccurline(dest);
    }
}

// transassign() writes translated statement to varlist, not dest
// not a good idea, better change it
// if changed, change transfunc() behaviour as well
void transassign(StrBlock *dest, StrBlock *src, int targetline, int argc, ...) {
    va_list args;
    va_start(args, argc);
    StrBlock *glvarlist;
    StrBlock *funcvarlist = NULL;

    if (argc >= 1) {
        glvarlist = va_arg(args, StrBlock *);
    }
    if (argc == 2) {
        funcvarlist = va_arg(args, StrBlock *);
    }

    // for getting a cleaned var name from src
    char varname[MAX_VARNAME_LENGTH] = {'\0'};
    int var_cur = 0;
    // value don't need to be clean, or "shouldn't"
    char value[MAX_VARVALUE_LENGTH];

    // getting varname from input/src
    char *line = src->content[targetline];
    rmnewline(line);
    int pos = strstr(line, "<-") - line;

    // pos - 1 excludes the space before <-
    for (int i = 0; i < pos - 1; i++) {
        if (line[i] == ' ') {
            fprintf(stderr, " ! @SYNTAX ERROR: Spaces found in variable namen");
            exit(-1);
        }
        varname[var_cur] = line[i];
        var_cur += 1;
    }

    // enclose varname with proper str ending
    varname[var_cur] = '\0';
    // retrive var value
    strcpy(value, line + pos + 2);

    // scan through current varlist
    if (funcvarlist) {
        if (checkvar(glvarlist, varname) == 1) {
            sprintf(glvarlist->content[glvarlist->curline], "%s = %s;", varname, value);
            inccurline(glvarlist);
        } else if (checkvar(funcvarlist, varname) == 1) {
            sprintf(funcvarlist->content[funcvarlist->curline], "%s = %s;", varname, value);
            inccurline(funcvarlist);
        } else {
            // if not, define new var in funvarlist
            sprintf(funcvarlist->content[funcvarlist->curline], "double %s = %s;", varname, value);
            inccurline(funcvarlist);
        }
    }
        // no funcvarlist passed
    else if (checkvar(glvarlist, varname) == 1) {
        sprintf(glvarlist->content[glvarlist->curline], "%s = %s;", varname, value);
        inccurline(glvarlist);
    } else {
        sprintf(glvarlist->content[glvarlist->curline], "double %s = %s;", varname, value);
        inccurline(glvarlist);


    }
}

// Using varlist is quite similar to the one in transassign(), isolate it?
void transfunc(StrBlock *dest, StrBlock *src) {
    // src, varlist is a temporary storage, free mem when return
    StrBlock varlist = strblockinit();
    char funcname[MAX_VARNAME_LENGTH] = {'\0'};
    char arglist[MAX_VARNAMES][MAX_VARNAME_LENGTH] = {'\0'};
    int argcount = 0;
    // strip the funcname and arguments down, src->content[0] is "function foobar a , b ... "
    // first space pos = 9; j is varlist's inline cursor
    // let first line in varlist to be function name
    char firstline[MAX_LINE_LENGTH] = {'\0'};
    strcpy(firstline, strstr(src->content[0], " ") + 1);
    int k = 0;
    while (firstline[k] != ' ' && firstline[k] != '\0') {
        funcname[k] = firstline[k];
        k++;
    }
    funcname[k + 1] = '\0';
    getvarnames(firstline + k, arglist, &argcount);

    // translate varlist from (a, b, ...) to (double a, double b, ...)
    for (int i = 1; i < varlist.curline; i++) {
        char buf[MAX_VARNAME_LENGTH] = {'\0'};
        strcat(buf, "double ");
        strcat(buf, varlist.content[i]);
        // in order to let transassign() recognize formal args, put a second space after each varname
        strcat(buf, " ");
        strcpy(varlist.content[i], buf);
    }

    char buf[MAX_LINE_LENGTH] = {'\0'};
    char comma[2] = {','};
    for (int i = 0; i < argcount; i++) {
        strcat(buf, "double ");
        strcat(buf, arglist[i]);
        strcat(buf, comma);
    }
    // strip the last comma, put down function first line/head
    buf[strlen(buf) - 1] = '\0';
    sprintf(dest->content[dest->curline], "double %s (%s){", funcname, buf);
    inccurline(dest);

    // put args back into varlist
    for (int l = 0; l < argcount; l++) {
        sprintf(varlist.content[l], "double %s =", arglist[l]);
        inccurline(&varlist);
    }


    // scan for any other undeclared vars that needs to be init
    for (int i = 1; i < src->curline; i++) {
        defaultinit(src->content[i], 2, &glvarlist, &varlist);
    }

    // put down newly inited vars if applicable
    // skip args lines
    for (int j = argcount; j < varlist.curline; j++) {
        if (varlist.content[j] == NULL) {
            break;
        }
        strcpy(dest->content[dest->curline], varlist.content[j]);
        inccurline(dest);
    }


    // put function body down
    for (int i = 1; i < src->curline; i++) {
        if (strstr(src->content[i], "<-") != NULL) {
            transassign(dest, src, i, 2, &glvarlist, &varlist);
            // because of the reason provided in transassign, need to copy the statements from varlist to dest(mlfunc)
            strcpy(dest->content[dest->curline], varlist.content[varlist.curline - 1]);
            inccurline(dest);
        } else if (strstr(src->content[i], "print") != NULL) {
            if (strncmp(src->content[i], "print ", strlen("print ")) == 0) {
                transprint(dest, src, i);
            }
        } else {
            if (dest->content[dest->curline]!= NULL){
                inccurline(dest);
            }
            sprintf(dest->content[dest->curline], "%s;", src->content[i]);
            inccurline(dest);
        }
    }

    // add a return statement for catching all
    sprintf(dest->content[dest->curline], "return 0.0;\n}");
    inccurline(dest);


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
          "#include <string.h>\n",
          ofp);

    // store statements which will be written into .ml's main(), in a StrBlock struct
    StrBlock mlmain = strblockinit();

    // store functions
    StrBlock mlfunc = strblockinit();

    // global variable list
    glvarlist = strblockinit();

    // clean up continuous white space before we start translation
    for (int i = 0; i < inputfile.linecounts; i++) {
        inputfile.content[i] = rmcontspaces(inputfile.content[i]);
    }


    // ********************  Main Logic ********************
    for (int i = 0; i < inputfile.linecounts; i++) {
        // check if the line is empty, or it's a comment
        if (inputfile.content[i][0] == '\n' || inputfile.content[i][0] == '#') {
            continue;
        }

        // *****************************************************************
        // value assign
        if (strstr(inputfile.content[i], "<-") != NULL) {
            transassign(&mlmain, &inputfile, i, 1, &glvarlist);
            continue;
        }
        // *****************************************************************
        // enter function body
        if (strstr(inputfile.content[i], "function") != NULL) {
            if (strncmp(inputfile.content[i], "function ", strlen("function ")) == 0) {
                StrBlock funcbody = strblockinit();
                // do-while to include the funtion defn line
                do {
                    rmnewline(inputfile.content[i]);
                    strcpy(funcbody.content[funcbody.curline], inputfile.content[i]);
                    // don't forget to increment outter loop's counter
                    i += 1;
                    inccurline(&funcbody);
                } while (i < inputfile.linecounts && inputfile.content[i][0] == '\t');
                // offset the i++ from loop itself
                i--;
                // strip all /t from funcbody
                for (int j = 1; j < funcbody.curline; j++) {
                    char buf[MAX_LINE_LENGTH] = {'\0'};
                    strcpy(buf, funcbody.content[j] + 1);
                    strcpy(funcbody.content[j], buf);
                }
                transfunc(&mlfunc, &funcbody);
                continue;
            }

        }
        // *****************************************************************
        // print statement found
        if (strstr(inputfile.content[i], "print") != NULL) {
            if (strncmp(inputfile.content[i], "print ", strlen("print ")) == 0) {
                rmnewline(inputfile.content[i]);
                defaultinit(inputfile.content[i], 1, &glvarlist);
                transprint(&mlmain, &inputfile, i);
                continue;
            }
        }
        // *****************************************************************
        // if it's a function call to an exist function
        int isfunc = 0;
        for (int j = 0; j < mlfunc.curline; j++) {
            char *firstspace = strstr(mlfunc.content[j], " ") + 1;
            int pos = strstr(firstspace, " ") - firstspace;
            if (strncmp(inputfile.content[i], strcat(firstspace, " "), pos) == 0) {
                rmnewline(inputfile.content[i]);
                sprintf(mlmain.content[mlmain.curline], "%s;", inputfile.content[i]);
                inccurline(&mlmain);
                isfunc = 1;
                break;
            }
        }
        if (isfunc == 1) {
            continue;
        }
        // *****************************************************************
        // the rest must be an invalid statement
        fprintf(stderr, "!SYNTAX ERROR, unknown expression.\n");
        exit(-1);
    }

    // *********************Main Logic Ends********************************

    // free memory
    freecontent(inputfile.content);
    fclose(ifp);

    // write global varlist to .c file
    for (int i = 0; i < glvarlist.linecounts; i++) {
        if (glvarlist.content[i][0] == '\0') {
            continue;
        }

        fputs(glvarlist.content[i], ofp);
        fputs("\n", ofp);
    }

    // write function declarations to .c file
    for (int i = 0; i < mlfunc.linecounts; i++) {
        if (mlfunc.content[i][0] == '\0') {
            continue;
        }
        fputs(mlfunc.content[i], ofp);
        fputs("\n", ofp);
    }
    // write main() cache into .c file
    fputs("int main(){\n", ofp);
    for (int i = 0; i < mlmain.linecounts; i++) {
        if (mlmain.content[i][0] == '\0') {
            continue;
        }

        fputs(mlmain.content[i], ofp);
        fputs("\n", ofp);
    }
    fputs("\n}\n", ofp);

    // flush writes to ofp first, or else it's likely to fail
    fclose(ofp);

    // compile and execute .runml.temp.c
    if (system("gcc ./.runml_temp.c -o .ml") == 0) {
        int exec_res = system("./.ml");
        if (exec_res != 0) {
            // not sure if these are 'errors' or not (which would need to use stdout vs stderr)
            fprintf(stderr, " ! @ml execution failed\n");
            exit(-1);
        }
    } else {
        fprintf(stderr, " ! @ml compilation failed\n");
        exit(-1);

    }

    // delete temp files after compliation and run
    if (remove("./.runml_temp.c") == 0 && remove("./.ml") == 0) {
        //fprintf(stdout, "\n@ temp files deleted\n");
    } else {
        fprintf(stderr, " ! @temp file deletion failed\n");

        return 0;
    }
}
