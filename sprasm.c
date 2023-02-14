#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    T_WORD,
    T_BYTE,
    T_REL,
};

enum {
    A_REG,
    A_LOW,
    A_NREG,
    A_OTHER,
};

struct label {
    char name[20];
    uint32_t org;
};

struct unres {
    char ex[35];
    uint32_t addr, org;
    char type;
};

unsigned char memory[16777316];

const char *regs[] = {
    "ZERO", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
    "R8", "R9", "R10", "R11", "R12", "R13", "RSP", "RPC",
};

const char *nregs[] = {
    "(ZERO)", "(R1)", "(R2)", "(R3)", "(R4)", "(R5)", "(R6)", "(R7)",
    "(R8)", "(R9)", "(R10)", "(R11)", "(R12)", "(R13)", "(RSP)", "(RPC)",
};

const char *lregs[] = {
    "LERO", "L1", "L2", "L3", "L4", "L5", "L6", "L7",
    "L8", "L9", "L10", "L11", "L12", "L13", "LSP", "LPC",
};

unsigned int g_line;
const char *g_filename = 0;

uint32_t addr = 0;
uint32_t org = 0;
uint32_t prev = 0;

struct unres unres[100];
int nunres = 0;
struct label label[200];
int nlabel = 0;

void error(const char *err) {
    printf("error at line %d in %s: %s\n", g_line, g_filename, err);
    exit(1);
}

char argType(char *s, int *np) {
    int i, n;
    char t;
    n = -1;
    t = A_OTHER;
    for(i = 0; i < 16; i++) {
        if(!strcmp(regs[i], s)) { n = i; t = A_REG; break; }
        if(!strcmp(lregs[i], s)) { n = i; t = A_LREG; break; }
        if(!strcmp(nregs[i], s)) { n = i; t = A_NREG; break; }
    }
    if(np) *np = n;
    return t;
}

void addByte(char b) {
    memory[addr++] = b;
    org++;
}

void addWord(uint32_t w) {
    *(uint32_t*)&memory[addr] = w;
    addr += 4;
    org += 4;
}

void addVal(char *ex, char type) {
}

void asmLine(char *line) {
    const char *syntaxError = "syntax error";
    const char *nargsError = "wrong number of args";
    const char *argsError = "wrong combination of args";
    const char *white = " \t,:";
    char *tokens[40];
    int ntokens = 0;
    char buf[200];
    char *p, *b, *s;
    char a1, a2;
    int i, j;

    /* tokenize */

    b = buf;
    p = buf;

    do {
        if(*line == '"') {
            if(b != p) error(syntaxError);
            s = strchr(line+1, '"');
            if(!s) error("unterminated quote");
            while(line < s) *(p++) = *(line++);
            *p = 0;
            tokens[ntokens++] = b;
            b = ++p;
        } else {
            if(*line >= 'a' && *line <= 'z') *line += 'A'-'a';

            if(strchr(white, *line)) {
                if(b != p) {
                    *p = 0;
                    tokens[ntokens++] = b;
                    b = ++p;
                }
            } else {
                *(p++) = *line;
            }
        }
    } while(*(++line));

    /* assemble */

    if(!strcmp(tokens[0], "ORG")) {
        if(ntokens != 2) error(nargsError);
        org = eval(tokens[1]);
    } else if(!strcmp(tokens[0], "DB")) {
        if(ntokens == 1) error(nargsError);
        for(i = 1; i < ntokens; i++) {
            if(tokens[i][0] == '"') {
                for(j = 1; tokens[i][j]; j++)
                    addByte(tokens[i][j]);
            } else
                addVal(tokens[i], T_BYTE);
        }
    } else if(!strcmp(tokens[0], "DW")) {
        if(ntokens == 1) error(nargsError);
        for(i = 1; i < ntokens; i++)
            addVal(tokens[i], T_WORD);
    } else if(!strcmp(tokens[0], "INT")) {
        if(ntokens != 2) error(nargsError);
        addByte(0x00);
        addVal(tokens[1], T_BYTE);
    } else if(!strcmp(tokens[0], "CALL")) {
        if(ntokens != 2) error(nargsError);
        addByte(0x01);
        addVal(tokens[1], T_WORD);
    } else if(!strcmp(tokens[0], "BEQ")) {
        if(ntokens != 4) error(nargsError);
        addByte(0x02);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte((a1<<4)|a2);
        addVal(tokens[3], T_REL);
    } else if(!strcmp(tokens[0], "BNE")) {
        if(ntokens != 4) error(nargsError);
        addByte(0x03);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte((a1<<4)|a2);
        addVal(tokens[3], T_REL);
    } else if(!strcmp(tokens[0], "MOV")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 == A_REG && a2 == A_OTHER) {
            addByte(0x10|r1);
            addVal(tokens[2], T_WORD);
        } else if(a1 == A_REG && a2 == A_NREG) {
            addByte(0x04);
            addByte((r1<<4)|r2);
        } else if(a1 == A_NREG && a2 == A_REG) {
            addByte(0x05);
            addByte((r1<<4)|r2);
        } else if(a1 == A_LREG && a2 == A_NREG) {
            addByte(0x06);
            addByte((r1<<4)|r2);
        } else if(a1 == A_NREG && a2 == A_LREG) {
            addByte(0x07);
            addByte((r1<<4)|r2);
        } else error(argsError);
    } else if(!strcmp(tokens[0], "AND")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte(0x09);
        addByte((r1<<4)|r2);
    } else if(!strcmp(tokens[0], "OR")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte(0x0A);
        addByte((r1<<4)|r2);
    } else if(!strcmp(tokens[0], "XOR")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte(0x0B);
        addByte((r1<<4)|r2);
    } else if(!strcmp(tokens[0], "ADD")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 == A_REG && a2 == A_OTHER) {
            addByte(0x20|r1);
            addVal(tokens[2], T_BYTE);
        } else if(a1 == A_REG && a2 == A_REG) {
            addByte(0x0C);
            addByte((r1<<4)|r2);
        } else error(argsError);
    } else if(!strcmp(tokens[0], "SUB")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte(0x0D);
        addByte((r1<<4)|r2);
    } else if(!strcmp(tokens[0], "PUSH")) {
        if(ntokens != 2) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x30|r1);
    } else if(!strcmp(tokens[0], "POP")) {
        if(ntokens != 2) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x40|r1);
    } else if(!strcmp(tokens[0], "INV")) {
        if(ntokens != 2) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x50|r1);
    } else if(!strcmp(tokens[0], "SHR")) {
        if(ntokens != 2) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x60|r1);
    } else if(!strcmp(tokens[0], "SHL")) {
        if(ntokens != 2) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x70|r1);
    } else if(!strcmp(tokens[0], "-")) {
        prev = org;
    } else if(!strcmp(tokens[0], "+")) {
        resAhead();
    } else {
        strcpy(labels[nlabels].name, tokens[0]);
        labels[nlabels].org = org;
        nlabels++;
        resolve();
    }
}

void asmFile(const char *filename) {
    FILE *fp;
    unsigned int line;
    char buf[200];
    char *p;
    char c;

    fp = fopen(filename, "r");
    if(!fp) {
        if(g_filename)
            printf("error on line %d in %s:\n  failed to open %s\n",
                   g_line, g_filename, filename);
        else
            printf("error: failed to open %s\n", filename);
        exit(1);
    }

    g_filename = filename;
    g_line = 1;

    while(!feof(fp)) {
        p = buf;
        while(!feof(fp) && (c = fgetc(fp)) != '\n')
            *(p++) = c;
        *p = 0;
        if(p = strchr(buf, ";")) *p = 0;
        line = g_line;
        asmLine(buf);
        g_line = line;
        g_filename = filename;
        g_line++;
    }

    fclose(fp);
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
