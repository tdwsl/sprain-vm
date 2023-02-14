#include "sprvm.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

enum {
    T_WORD,
    T_BYTE,
    T_REL,
};

struct label {
    char name[30];
    uint32_t org;
};

struct unres {
    char type;
    char base[30];
    uint32_t addr, org;
    uint32_t off;
};

struct ahead {
    char type;
    uint32_t addr, org;
    uint32_t off;
};

struct fileInfo {
    char name[40];
    int line;
};

struct ins {
    char str[10];
    char type;
    unsigned char opc;
};

uint32_t org = 0;
uint32_t prev = 0;
uint32_t size = 0;

struct fileInfo infoStack[20];
struct fileInfo *info = infoStack-1;

struct unres unres[200];
int nunres = 0;

struct ahead ahead[100];
int nahead = 0;

struct label labels[400];
int nlabels = 0;

const char *forbid = "*/'£^&[]=#?<>{}\\|";
const char *syntaxError = "syntax error";

void error(const char *err) {
    printf("error on line %d in %s: %s\n",
           info->line, info->name, err);
    exit(1);
}

void capitalise(char *s) {
    do {
        if(*s >= 'a' && *s <= 'z')
            *s += 'A'-'a';
    } while(*(++s));
}

struct label *findLabel(const char *s) {
    int i;
    for(i = 0; i < nlabels; i++)
        if(!strcmp(labels[i].name, s)) return &labels[i];
    return 0;
}

void resAhead() {
    int i;
    for(i = 0; i < nahead; i++) {
        ahead[i].off += org;
        switch(ahead[i].type) {
        case T_WORD:
            setm(ahead[i].addr, ahead[i].off);
            break;
        case T_BYTE:
            memory[ahead[i].addr] = ahead[i].off;
            break;
        case T_REL:
            memory[ahead[i].addr] = (char)(org-ahead[i].off);
            break;
        }
    }
    nahead = 0;
}

int number(const char *s, uint32_t *n) {
    uint32_t base;

    if(*s == '$' || *s == '&') { base = 16; s++; }
    else if(*s == '%') { base = 2; s++; }
    else base = 10;

    if(*s == 0) return 0;

    *n = 0;

    do {
        if(base <= 10) {
            if(*s >= '0' && *s < '0'+base-1)
                *n = *n * base + *s - '0';
            else return 0;
        } else {
            if(*s >= '0' && *s <= '9')
                *n = *n * base + *s - '0';
            else if(*s >= 'A' && *s <= 'A'+base-11)
                *n = *n * base + *s - 'A' + 10;
            else return 0;
        }
    } while(*(++s));

    return 1;
}

uint32_t eval(char *ex) {
    uint32_t n, t;
    char buf[200];
    char *tokens[20];
    int ntokens = 0;
    const char *delim = "+-";
    const char *failMsg = "failed to resolve expression";
    char *p = buf;
    char *b = buf;
    int i;
    char fl;
    struct label *lbl;

    do {
        if(*ex <= ' ') {
            if(p != b) {
                *p = 0;
                tokens[ntokens++] = b;
                b = ++p;
            }
        } else if(strchr(delim, *ex)) {
            if(p != b) {
                *p = 0;
                tokens[ntokens++] = b;
            }
            *(++p) = *ex;
            *(++p) = 0;
            tokens[ntokens++] = p-2;
            b = ++p;
        } else if(strchr(forbid, *ex)) {
            sprintf(buf, "forbidden character %c", *ex);
            error(buf);
        } else {
            *(p++) = *ex;
        }
    } while(*(++ex));

    if(ntokens == 0) error(failMsg);
    else if(ntokens == 1 && !strcmp(tokens[0], "-")) return prev;

    t = 0;
    fl = 0;
    for(i = 0; i < ntokens; i++) {
        if(!strcmp(tokens[i], "+")) fl ^= 1;
        else if(!strcmp(tokens[i], "-")) fl |= 1;
        else {
            if(!number(tokens[i], &n)) {
                if(!(lbl = findLabel(tokens[i])))
                    error(failMsg);
            }
            if(fl & 1) t -= n;
            else t += n;
            fl |= 2;
        }
    }

    if(!(fl & 2))
        error(failMsg);

    return t;
}

void resolve() {
    int i;
    struct label *lbl;

    for(i = 0; i < nunres; i++) {
        lbl = findLabel(unres[i].base);
        if(!lbl) continue;

        unres[i].off += lbl->org;
        switch(unres[i].type) {
        case T_WORD:
            setm(unres[i].addr, unres[i].off);
            break;
        case T_BYTE:
            memory[unres[i].addr] = unres[i].off;
            break;
        case T_REL:
            memory[unres[i].addr] = (char)(org-unres[i].off);
            break;
        }

        unres[i] = unres[--nunres];
        i--;
    }
}

void addValue(char *line, char type) {
    char sz;
    char *p;

    capitalise(line);

    if(type == T_WORD) {
        sz = 4;
        size += 4;
        org += 4;
    } else {
        sz = 1;
        size++;
        org++;
    }

    if(!strcmp(line, "-")) {
        switch(type) {
        case T_WORD:
            setm(size-4, prev);
            break;
        case T_BYTE:
            memory[size-1] = prev;
            break;
        case T_REL:
            memory[size-1] = (char)(prev-org);
            break;
        }
    } else if(!strcmp(line, "+")) {
        ahead[nahead].addr = size-sz;
        ahead[nahead].org = org - (type != T_REL) ? sz : 0;
        ahead[nahead].type = type;
        nahead++;
    } else {
        if(p = strstr(line, " \n\t")) {
            *(p++) = 0;
            unres[nunres].off = eval(p);
        } else if(p = strstr(line, "+-")) {
            unres[nunres].off = eval(p);
            *p = 0;
        } else
            unres[nunres].off = 0;
        strcpy(unres[nunres].base, line);
        unres[nunres].addr = size-sz;
        unres[nunres].org = org-sz;
        unres[nunres].type = type;
        nunres++;
        resolve();
    }
}

void addByte(unsigned char v) {
    memory[size] = v;
    size++;
    org++;
}

void addWord(uint32_t v) {
    setm(size, v);
    size += 4;
    org += 4;
}

void addInt(uint32_t v, char type) {
    if(type == T_WORD) addWord(v);
    else addByte(v);
}

void asmDB(char *line, char type) {
    char *p = line;
    char buf[20];

    do {
        if(*p == '"') {
            if(p-1 != line) error(syntaxError);
            p = strchr(p+1, '"');
            if(!p) error("unterminated quote");
            while(line < p)
                addInt(*(line++), type);
            line = p;
        } else if(*p == ',') {
            if(p-1 == line) error(syntaxError);
            *p = 0;
            addValue(line, type);
            line = p;
        } else {
            if(p-1 == line && *p <= ' ') line = p;
        }
    } while(*(++p));

    if(p-1 != line)
        addValue(line, type);
}

int isInstruction(char *s) {
    const char *instructions[] = {
        "INT", "CALL", "BNE", "BEQ", "MOV", "MOL", "AND", "OR",
        "XOR", "ADD", "SUB", "SHL", "SHR", "INV", "PUSH", "POP",
        0,
    };
    int i;
    for(i = 0; instructions[i]; i++)
        if(!strcmp(instructions[i], s)) return 1;
    return 0;
}

void strip(char **s) {
    char *p;
    while(*(*s) && *(*s) <= ' ') (*s)++;
    if(*(*s) == 0) return;
    p = (*s)+strlen(*s)-1;
    while(*p <= ' ') p--;
    *(p+1) = 0;
}

int regn(const char *s) {
    int i;
    const char *regs[] = {
        "ZERO", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
        "R8", "R9", "R10", "R11", "R12", "R13", "RSP", "RPC",
    };
    for(i = 0; i < 16; i++) 
        if(!strcmp(regs[i], s)) return i;
    return -1;
}

void asmI8(unsigned char opc, char *arg) {
    addByte(opc);
    addValue(arg, T_BYTE);
}

void asmI32(unsigned char opc, char *arg) {
    addByte(opc);
    addValue(arg, T_WORD);
}

void asmR(unsigned char opc, char *arg) {
    int r;
    strip(&arg);
    if((r = regn(arg)) == -1)
        error("invalid register");
    opc |= r;
    addByte(opc);
}

void asmR8(unsigned char opc, char *arg) {
    addByte(opc);
    addValue(arg, T_REL);
}

void asmLine(char *line) {
    char *p, *arg;

    printf("%s\n", line);

    if(p = strchr(line, ':')) {
        *p = 0;
        asmLine(p+1);
    }

    while(*line && *line <= ' ') line++;
    if(*line == 0) return;

    for(p = line; *p && *p > ' '; p++);
    if(*p) {
        *(p++) = 0;
        arg = p;
    } else arg = 0;

    capitalise(line);

    if(!strcmp(line, "-")) {
        prev = org;
        if(arg) asmLine(arg);
    } else if(!strcmp(line, "+")) {
        resAhead();
    } else if(!strcmp(line, "DB")) {
        asmDB(arg, T_BYTE);
    } else if(!strcmp(line, "DW")) {
        asmDB(arg, T_WORD);
    } else if(!strcmp(line, "ORG")) {
        org = eval(arg);
    } else if(isInstruction(line)) {
        if(!strcmp(line, "BRA"))
            asmR8(0x2f, arg);
        else if(!strcmp(line, "INT"))
            asmI8(0x00, arg);
    } else {
        labels[nlabels].org = org;
        strcpy(labels[nlabels].name, line);
        if(arg) asmLine(arg);
    }
}

void asmFile(const char *filename) {
    char buf[200];
    FILE *fp;
    char *p;

    info++;
    strcpy(info->name, filename);
    info->line = 1;

    fp = fopen(filename, "r");
    if(!fp) {
        if(info != infoStack) {
            printf("error on line %d in %s:\n  ",
                   (info-1)->line, (info-1)->name);
        }
        printf("failed to open %s\n", info->name);
        exit(1);
    }

    for(;;) {
        for(p = buf; (*p = fgetc(fp)) != '\n' && !feof(fp); p++);
        if(feof(fp)) p++;
        *p = 0;
        p = strchr(buf, ';');
        if(p) *p = ';';
        asmLine(buf);
        info->line++;
    }
}

void saveToFile(const char *filename) {
    FILE *fp;
    fp = fopen(filename, "wb");
    if(!fp) {
        printf("failed to save to %s\n", filename);
        exit(1);
    }
    fwrite(memory, 1, size, fp);
    fclose(fp);
}

int main(int argc, char **args) {
    if(argc != 3) {
        printf("usage: %s <file> <out>\n", args[0]);
        return 0;
    }

    asmFile(args[1]);
    printf("assembled successfully\n");
    saveToFile(args[2]);
    return 0;
}
